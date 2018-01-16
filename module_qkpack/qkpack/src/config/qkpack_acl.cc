#include "config/qkpack_acl.h"
#include "qkpack_log.h"
#include "util/string_util.h"
#include "config/easycurl.h"

using namespace com::youku::data::qkpack::config;
using namespace com::youku::data::qkpack::util;

QkPackACL::QkPackACL(std::string acl_url,int timeout):
	acl_url_(acl_url),
	timeout_(timeout)
{
	qkpack_yajl_json_ = new QkPackYajlJSON();
}


QkPackACL::~QkPackACL()
{
	if ( qkpack_yajl_json_ ) {
		delete qkpack_yajl_json_;
		qkpack_yajl_json_ = NULL;
	}

	
	std::map<std::string, qkpack_acl_t*>::const_iterator		it;

	for ( it = map_acl_.begin(); it != map_acl_.end(); ++it) {
		
		qkpack_acl_t * acl =  it->second;	
		if ( acl )  {
			delete acl;
			acl = NULL;
		}
	}
}


int QkPackACL::SetNameSpace(qkpack_request_t *request) 
{
	int                                             j,num = 0;
	const char*                                           data = (char*)request->uri.c_str();
	int                                             len = request->uri.length();
	
	if ( strncmp(data, (const char*)QKPACK_ACL_URI_FORMAT, strlen(QKPACK_ACL_URI_FORMAT)) ) {
		request->desc = QKPACK_ERROR_ACL_FORMAT;
		return QKPACK_ERROR;
	}

	for (j = 0;j < len;j++)
	{
		if ( data[j] == 47 ) num++;
		if ( num == 3 ) break;
	}

	if ( j <= 0 || num != 3 ) {
		request->desc = QKPACK_ERROR_ACL_NAMESPACE_NOT_EMPTY;
		return QKPACK_ERROR;
	}

	request->kvpair.namespaces = std::string(data+j+1);
	if ( request->kvpair.namespaces.length() != QKPACK_ACL_NAMESPACE_LENGTH ) {
		request->desc = QKPACK_ERROR_ACL_NAMESPACE_LENGTH;
		return QKPACK_ERROR;
	}
	
	return QKPACK_OK;
}

int QkPackACL::SetClusterName(qkpack_request_t *request) 
{
	int                                             j = 0,first = 0,last = 0,num = 0;
	const char*                                           data = (char*)request->uri.c_str();
	int                                             len = request->uri.length();
	
	for (j = 0;j < len;j++)
	{
		if ( data[j] == 47 ) num++;
		if ( num == 2 && first == 0 ) first = j;
		if ( num == 3 && last == 0 ) {last = j;break;}
	}
	
	if ( j <=0 || first == 0 || last == 0 ) {
		return QKPACK_ERROR;
	}

	request->cluster_name = std::string(data+first+1, last-first-1);
	return QKPACK_OK;
}	

int QkPackACL::GetZsetLimit(qkpack_request_t *request, const std::vector<limit_policies_t> &vector_policies)
{
	int								key_len;
	bool							status = false;

	if ( request->kvpair.command_type != REDIS_ZADD &&
			request->kvpair.command_type != REDIS_ZBATCHADD ) {
		return QKPACK_OK;
	}

	std::map<std::string, std::vector<kvpair_t> >				&map_kv = request->map_kv;
	std::map<std::string, std::vector<kvpair_t> >::iterator		it;

	for ( it = map_kv.begin(); it != map_kv.end(); ++it ) {

		key_len = it->first.length();

		for ( size_t i = 0; i < vector_policies.size(); ++i ) {
		
			if ( key_len >= vector_policies[i].min && key_len <= vector_policies[i].max ) {
				
				status = true;
				it->second[0].limit = vector_policies[i].limit;
				break;
			}

		}
		
		if ( !status ) {
			request->desc.append(QKPACK_ERROR_ACL_LIMITPOLICIES_LIMIT);
			request->desc.append(StringUtil::ToString(key_len));

			return QKPACK_ERROR;
		}
	}

	return QKPACK_OK;
}

//判断接口的超时时间
int QkPackACL::CheckInterfaceTimeout(qkpack_request_t *request,const std::vector<timeouts_t> &vector_timeouts)
{
	std::size_t						found;

	for ( size_t i = 0; i < vector_timeouts.size(); ++i ) {
	
		found = request->request_uri.find(vector_timeouts[i].interface_name);
		if ( found != std::string::npos ) {
		
			request->timeout = vector_timeouts[i].timeout;
			request->tries = vector_timeouts[i].tries;
			break;
		}
	}

	return QKPACK_OK;
}


//判断接口的长度限制(keylen,vallen,datalen)
int QkPackACL::CheckInterfaceLimitsLength(qkpack_request_t *request, const std::vector<limits_length_t> &vector_limits_len)
{
	std::size_t						found;
	int								keylen = request->kvpair.key.length();
	int								vallen = request->kvpair.value.length();
	int								datalen = request->request_buffer.length();

	for ( size_t i = 0; i < vector_limits_len.size(); ++i ) {
	
		found = request->request_uri.find(vector_limits_len[i].interface_name);
		if ( found != std::string::npos ) {
		
			if ( keylen &&  keylen > vector_limits_len[i].keylen ) {
				
				request->desc = QKPACK_ERROR_ACL_LIMITSLENGTH_KEYLEN;
				return QKPACK_ERROR;
			}

			if ( vallen && vallen > vector_limits_len[i].vallen ) {
				
				request->desc = QKPACK_ERROR_ACL_LIMITSLENGTH_VALLEN;
				return QKPACK_ERROR;
			}

			if ( datalen && datalen > vector_limits_len[i].datalen ) {
				
				request->desc = QKPACK_ERROR_ACL_LIMITSLENGTH_DATALEN;;
				return QKPACK_ERROR;
			}
		}
	}

	return QKPACK_OK;
}


int QkPackACL::GetNodeList(qkpack_request_t *request, qkpack_acl_t *acl)
{
	std::string												key;
	std::string												&x_real_ip = request->x_real_ip;
	std::map<std::string,std::string>						&map_server = acl->route.map_server;
	std::map<std::string,std::string>						&map_client = acl->route.map_client;
	std::map<std::string,std::string>::const_iterator		it_server;
	std::map<std::string,std::string>::const_iterator		it_client;
	char*													data = (char*)x_real_ip.c_str();
	int														len = x_real_ip.length();
	int														j,num = 0;
	char													rest[10];
	
	request->kvpair.exptime = acl->quotation.expire;
	request->kvpair.uri_id = acl->uri_id;
	request->kvpair.compress_threshold = acl->quotation.compress_threshold;

	//根据客户端ip网段匹配集群名以及节点IP
	for (j = 0;j < len;j++)
	{
		char p = data[j];
		rest[j] = p;
		if ( p == 46 ) num++;
		if ( num == 2 ) break;
	}
	key = std::string(rest,j);

	it_client = map_client.find(key);
	if ( it_client == map_client.end() ) {
		acl->route.is_match_ip = false;
		return QKPACK_OK;
	}

	it_server = map_server.find(it_client->second);
	if ( it_server != map_server.end() ) {
		acl->route.is_match_ip = true;
		return QKPACK_OK;
	}
	
	acl->route.is_match_ip = false;
	return QKPACK_OK;
}


qkpack_acl_t* QkPackACL::Process(qkpack_request_t *request) 
{
	int													rc;
	qkpack_acl_t										*acl = NULL;

	rc = SetNameSpace(request);
	if ( rc != QKPACK_OK ) {
		return NULL;
	}
	
	//set acl info
	acl = SetACL( request->ak, request->uri);
	if ( acl == NULL ) {
		request->desc = QKPACK_ERROR_ACL_NO_AUTH;
		return NULL;
	}
			
	rc = SetClusterName(request);
	if ( rc != QKPACK_OK ) {
		return NULL;
	}	
	
	//get node list
	rc = GetNodeList(request, acl);
	if ( rc != QKPACK_OK ) {
		return NULL;
	}
		
	//权限验证
	if ( !(acl->operation & request->kvpair.operation_type) ) {
	
		request->desc = (request->kvpair.operation_type == 
				QKPACK_OPERATION_READ?QKPACK_ERROR_INTERFACE_OPERATION_READ:QKPACK_ERROR_INTERFACE_OPERATION_WRITE);
		return NULL;
	}

	//check keylen,vallen,datalen
	rc = CheckInterfaceLimitsLength(request, acl->quotation.vector_limits_length);
	if ( rc != QKPACK_OK ) {
		return NULL;
	}

	//check zfixedsetadd limit
	rc = GetZsetLimit(request, acl->quotation.vector_policies);
	if ( rc != QKPACK_OK ) {
		return NULL;
	}
	
	//check interface timeout
	rc = CheckInterfaceTimeout(request, acl->quotation.vector_timeouts);
	if ( rc != QKPACK_OK ) {
		return NULL;
	}

	return acl;
}


qkpack_acl_t* QkPackACL::SetACL(const std::string &ak, const std::string &uri)
{
	int													rc;
	std::string											key = uri + "&&" + ak;
	std::map<std::string, qkpack_acl_t*>::const_iterator		it;

	it = map_acl_.find(key);		
	if( it != map_acl_.end() ) {
		return it->second;
	}
	
	qkpack_acl_t *acl = new qkpack_acl_t();
	rc = GetCurlResponse(acl,ak, uri); 
	if ( rc != QKPACK_OK ) {
		delete acl;
		return NULL;
	}
	map_acl_.insert(std::make_pair(key, acl));

	return acl;
}


int QkPackACL::GetCurlResponse(qkpack_acl_t *acl,const std::string &ak, const std::string &uri)
{
	int													rc;
	EasyCurl											easy_curl;
	std::vector<std::string>							headers;
	std::string											data;

	//POST /data-auth-http-server/httpServer/authority
	//Content-Type: application/json;charset=UTF-8
	//{"app_key":"GBeArhYgWl","uri":"qkd://BJ001/lj"}

	QKPACK_LOG_DEBUG("acl post /data-auth-http-server/httpServer/authority");
	
	headers.push_back("Content-Type: application/json;charset=UTF-8");
	data = "{\"app_key\":\""+ak+"\",\"uri\":\""+uri+"\"}";
	
	rc = easy_curl.Post(acl_url_, headers, data, timeout_);
	if ( rc != QKPACK_OK || !easy_curl.GetResponse().length() ) {
		return QKPACK_ERROR;
	}
	
	rc = qkpack_yajl_json_ ->ReadACLResponse(acl, easy_curl.GetResponse());
	if ( rc != QKPACK_OK ) {
		return QKPACK_ERROR;
	}

	return QKPACK_OK;
}
