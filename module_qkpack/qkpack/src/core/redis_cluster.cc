#include "core/redis_cluster.h"
#include "util/string_util.h"
#include "qkpack_log.h"

#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <hiredis/adapters/libevent.h>

using namespace com::youku::data::qkpack::core;
using namespace com::youku::data::qkpack::util;


RedisCluster::RedisCluster(std::string backup_server):
	backup_server_(backup_server)
{
}


RedisCluster::~RedisCluster()
{
}


void GetCallback(redisAsyncContext *c, void *r, void *privdata) 
{
    redisReply 	*reply = (redisReply *)r;
	RedisCluster *redis_cluster_ = (RedisCluster*)privdata;
    if (reply == NULL) return;
	
	std::string cluster_name = redis_cluster_->GetClusterName();	
	redis_cluster_->UpdateClusterNodes(reply->str, cluster_name);
	
    /* Disconnect after receiving the reply to GET */
    redisAsyncDisconnect(c);
}

int RedisCluster::InitAsyncClusterNode( const std::string &node_ip, 
		const std::string &cluster_name )
{
	int 						port;
	
    signal(SIGPIPE, SIG_IGN);
    struct event_base *base = event_base_new();
	
	std::vector<std::string> vector_line = StringUtil::Split(node_ip, ":");
	StringUtil::StrToInt32(vector_line[1].c_str(),port);
	SetClusterName(cluster_name);
	
    redisAsyncContext *c = redisAsyncConnect(vector_line[0].c_str(), port);
    if (c->err) {
        /* Let *c leak for now... */
		//QKPACK_LOG_ERROR("RedisCluster::InitAsyncClusterNode Error: %s", c->errstr);
        return QKPACK_ERROR;
    }

    redisLibeventAttach(c,base);
    redisAsyncSetConnectCallback(c,NULL);
    redisAsyncSetDisconnectCallback(c,NULL);
    redisAsyncCommand(c, GetCallback, this, "CLUSTER NODES");
    event_base_dispatch(base);
	
    return QKPACK_OK;
}

//初始化redis cluster节点，获取节点信息
int RedisCluster::InitClusterNode(const std::string &node_ip, 
		const std::string &cluster_name)
{
	int 						port;
	
	std::vector<std::string> vector_line = StringUtil::Split(node_ip, ":");
	StringUtil::StrToInt32(vector_line[1].c_str(),port);
	
	redisContext* c = redisConnect(vector_line[0].c_str(), port);
	if (c->err) {
		
		QKPACK_LOG_ERROR("init cluster node error=[%s]",c->errstr);
		return QKPACK_ERROR;
	}
	
	redisReply *reply = (redisReply *)redisCommand(c,"CLUSTER NODES");
	UpdateClusterNodes(reply->str, cluster_name);
	
    freeReplyObject(reply);
	redisFree(c);  
	
	return QKPACK_OK;
}


int RedisCluster::UpdateRouteNodeList(const std::string &iplist,const std::string &cluster_name) 
{
	int							rc;
	std::vector<std::string>	vector_ip = StringUtil::Split(iplist,",");

	for ( size_t i = 0; i < vector_ip.size(); ++i ) {
		
		rc = InitClusterNode(vector_ip[i], cluster_name);
		if ( rc == QKPACK_OK ) {
			return QKPACK_OK;
		}
	}
	return QKPACK_ERROR;
}


//根据请求的key得到节点ip  key->slot->node_ip
int RedisCluster::GetNode(qkpack_request_t	*request)
{
	std::string					&cluster_name = request->parent->cluster_name;

	std::map<std::string, std::map<int, std::string> >::const_iterator it;
	it = map_cluster_.find(cluster_name);
	if( it == map_cluster_.end() ) {
		return QKPACK_ERROR;
	}
	
	std::map<int, std::string>::const_iterator it_slot;
	it_slot = it->second.find(request->slot_id);
	if( it_slot != it->second.end() ) {
		request->node_ip = it_slot->second;
		return QKPACK_OK;
	}
		
	QKPACK_LOG_ERROR("redis cluster get node not found");
	return QKPACK_ERROR;	
}


//根据请求的key得到节点ip  key->slot->node_ip
int RedisCluster::GetNode(qkpack_request_t	*request, const route_t  &route)
{
	int							rc;
	std::string					&cluster_name = request->cluster_name;

	std::map<std::string, std::map<int, std::string> >::const_iterator it;
	it = map_cluster_.find(cluster_name);
	
	if( it == map_cluster_.end() ) {
		//update cluster nodes 
		if ( route.is_match_ip ) {
			
			std::map<std::string,std::string>::const_iterator it;
			for ( it = route.map_server.begin(); it != route.map_server.end(); ++it ) {
				rc = UpdateRouteNodeList(it->second,cluster_name); 
				if ( rc != QKPACK_OK ) {
					 QKPACK_LOG_ERROR("redis cluster get node update route node list match true");
					return QKPACK_ERROR;
				}
			}
		}
		else {
			rc = UpdateRouteNodeList(route.defaults,cluster_name);
			if ( rc != QKPACK_OK ) {
				QKPACK_LOG_ERROR("redis cluster get node update route node list match false");
				return QKPACK_ERROR;
			}
		}

		it = map_cluster_.find(cluster_name);
		if ( it == map_cluster_.end() ) {
			QKPACK_LOG_ERROR("redis cluster map_cluster_ init fail");
			return QKPACK_ERROR;
		}
	}

	if ( request->kvpair.key.empty() ) {
		return QKPACK_OK;
	}

	std::string					&namespaces = request->kvpair.namespaces;
	std::string  				key = namespaces + request->kvpair.key;
	
	if ( request->kvpair.slot_id < 0 ) {
		request->kvpair.slot_id = key_hash_slot((char*)key.c_str(),key.length());
	}


	std::map<int, std::string>::const_iterator it_slot;
	it_slot = it->second.find(request->kvpair.slot_id);
	if( it_slot != it->second.end() ) {
		request->node_ip = it_slot->second;
		return QKPACK_OK;
	}
		
	QKPACK_LOG_ERROR("redis cluster get node not found");
	return QKPACK_ERROR;	
}


//process -MOVED 1 127.0.0.1:7000，update memory slotid
int RedisCluster::ProcessMoved(const std::string &in,const std::string &cluster_name, std::string &node_ip, int status) 
{
	QKPACK_LOG_ERROR("redis cluster moved_ask=[%s]",in.c_str());
	
	if ( status != REDIS_MOVED && status != REDIS_ASK ) {
		return QKPACK_OK;
	}

	std::vector<std::string> vector_line = StringUtil::Split(in, " ");
	if ( vector_line.size() < 3 ) {
		return QKPACK_ERROR;
	}
	
	std::string &str_slotid = vector_line[1];
	std::vector<std::string> vector_line_node = StringUtil::Split(vector_line[2], "\r\n");
	node_ip =  vector_line_node[0];

	std::vector<std::string> vector_node =  StringUtil::Split(node_ip, ":");
	if ( vector_node.size() != 2 ) {
		return QKPACK_ERROR;
	}

	int ip = 0;
	StringUtil::StrToInt32(vector_node[0].c_str(),ip);
	if ( ip <= 0 ) {
		return QKPACK_ERROR;
	}

	int port = 0;
	StringUtil::StrToInt32(vector_node[1].c_str(),port);
	if ( port <= 0 ) {
		return QKPACK_ERROR;
	}

	if ( status != REDIS_MOVED ) {
		return QKPACK_OK;
	}
	
	int slotid = -1;
	StringUtil::StrToInt32(str_slotid.c_str(),slotid);
	map_cluster_[cluster_name][slotid] = node_ip;
	
	return QKPACK_OK;
}


// d52ea3cb4cdf7294ac1fb61c696ae6483377bcfc 127.0.0.1:7000 master - 0 1428410625374 73 connected 5461-10922
// 94e5d32cbcc9539cc1539078ca372094c14f9f49 127.0.0.1:7001 myself,master - 0 0 1 connected 0-9 11-5460
// e7b21f65e8d0d6e82dee026de29e499bb518db36 127.0.0.1:7002 slave d52ea3cb4cdf7294ac1fb61c696ae6483377bcfc 0 1428410625373 73 connected
// 6a78b47b2e150693fc2bed8578a7ca88b8f1e04c 127.0.0.1:7003 myself,slave 94e5d32cbcc9539cc1539078ca372094c14f9f49 0 0 4 connected
// 70a2cd8936a3d28d94b4915afd94ea69a596376a :7004 myself,master - 0 0 0 connected
int RedisCluster::UpdateClusterNodes(const std::string &in, std::string cluster_name )
{	
	int 						rc;
	std::map<int,std::string> 	slot_map;
	std::vector<RedisNode*> 	list;
	
	std::vector<std::string> vector_line = StringUtil::Split(in, "\n");
	size_t vector_line_len = vector_line.size();

	for (size_t i =0; i < vector_line_len; ++i ) {

		RedisNode *node = GetNode(vector_line[i]);
		if ( node == NULL ) {
			continue;
		}
			
		rc = SetMemNode(node, slot_map);
		if ( rc != QKPACK_OK ) {
			return QKPACK_ERROR;
		}
		
		delete node;
		node = NULL;
	
	}
	
	map_cluster_.insert(std::make_pair(cluster_name, slot_map));

	return QKPACK_OK;	
}


int RedisCluster::SetMemNode(RedisNode *node, std::map<int,std::string> &slot_map)
{
	std::string id(node->GetId());
	std::string addr(node->GetAddr());
	std::string sid(node->GetSid());
	
	if ( !node->IsConnected() ) {
		return QKPACK_OK;
	}
		
	std::vector<std::pair<size_t, size_t> > slots = node->GetSlots();
	size_t slots_len = slots.size();

	for( size_t j =0; j < slots_len; ++j ) {
		
		for( size_t slot_num = slots[j].first; slot_num <= slots[j].second; ++ slot_num ) {
			//key:slot_num 			value:addr
			slot_map[slot_num] = addr;
			
		}
	}

	return QKPACK_OK;
}


RedisNode* RedisCluster::GetNode(const std::string& line)
{
	std::vector<std::string> tokens = StringUtil::Split(line," ");
	if ( tokens.size() < CLUSTER_NODES_LENGTH ) {
		return NULL;
	}

	bool myself = false;
	char* node_role = (char*)tokens[CLUSTER_NODES_ROLE].c_str();
	char* ptr = strchr(node_role, ',');
	if (ptr != NULL && *(ptr + 1) != 0) {
		*ptr++ = 0;
		if (strcasecmp(node_role, "myself") == 0)
			myself = true;
		node_role = ptr;
	}

	RedisNode* node = new RedisNode;
	node->SetId(tokens[CLUSTER_NODES_ID].c_str());
	node->SetAddr(tokens[CLUSTER_NODES_ADDR].c_str());
	node->SetSid(tokens[CLUSTER_NODES_SID].c_str());
	node->SetMyself(myself);
	node->SetConnected(strcasecmp(tokens[CLUSTER_NODES_CONNECTED].c_str(), "connected") == 0);
	
	if (strcasecmp(node_role, "master") == 0) {
		
		node->SetRole("master");
		size_t n = tokens.size();
		for (size_t i = CLUSTER_NODES_LENGTH; i < n; i++)
			AddSlotRange(node, (char*)tokens[i].c_str());
	}
	else if (strcasecmp(node_role, "slave") == 0) {
		node->SetRole("slave");
	}
	else if (strcasecmp(node_role, "handshake") == 0) {
		
		node->SetRole("master"); //handshake == master
		size_t n = tokens.size();
		for (size_t i = CLUSTER_NODES_LENGTH; i < n; i++)
			AddSlotRange(node, (char*)tokens[i].c_str());
	}
	
	return node;
}


void RedisCluster::AddSlotRange(RedisNode* node, char* slots)
{
	size_t slot_min, slot_max;

	char* ptr = strchr(slots, '-');
	if (ptr != NULL && *(ptr + 1) != 0) {
		
		*ptr++ = 0;
		slot_min = (size_t) atol(slots);
		slot_max = (size_t) atol(ptr);
		// xxx
		if (slot_max < slot_min)
			slot_max = slot_min;
	}
	else {
		slot_min = (size_t) atol(slots);
		slot_max = slot_min;
	}

	node->AddSlotRange(slot_min, slot_max);
}


void RedisCluster::SetClusterName(std::string cluster_name)
{
	cluster_name_ = cluster_name;
}


std::string RedisCluster::GetClusterName() 
{
	return cluster_name_;
}

