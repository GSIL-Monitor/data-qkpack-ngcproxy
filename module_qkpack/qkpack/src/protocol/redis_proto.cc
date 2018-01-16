#include "protocol/redis_proto.h"
#include "core/redis_slot.h"
#include "core/qkpack_metrics.h"
#include "qkpack_log.h"
#include "util/string_util.h"
#include "util/snprintf.h"
#include "snappys/snappy_compress.h"
#include <hiredis/hiredis.h>

using namespace com::youku::data::qkpack::protocol;
using namespace com::youku::data::qkpack::core;

static const std::string redis_proto_add_multikv_name_ ="addMultiKVScript";
static const std::string redis_proto_add_zset_name_ ="addZSetScript";
static const std::string redis_proto_incrby_name_ ="incrByScript";
static const std::string redis_proto_add_rset_name_="addRSetScript";


RedisProto::RedisProto(std::string add_multikv_script,std::string add_zset_script,
		std::string incrby_script,std::string add_rset_script):
	redis_proto_add_multikv_script_(add_multikv_script),
	redis_proto_add_zset_script_(add_zset_script),
	redis_proto_incrby_script_(incrby_script),
	redis_proto_add_rset_script_(add_rset_script)
{
	compress_ = new SnappyCompress();
}

RedisProto::~RedisProto()
{
	if ( compress_ ) {
		delete compress_;
		compress_ = NULL;
	}
}


/**
 * 
 * 同步初始化Script Load Lua脚本
 * 
*/
int RedisProto::InitClusterScript(qkpack_request_t *request,const std::string &script, std::string &script_sha)
{
	int 			port;
	std::vector<std::string> vector_line = StringUtil::Split(request->node_ip, ":");
	StringUtil::StrToInt32(vector_line[1].c_str(),port);
	
	redisContext* c = redisConnect(vector_line[0].c_str(), port);
	if (c->err) {
		//QKPACK_LOG_ERROR("Connection error: %s\n", c->errstr);
		return REDIS_ERROR;
	}
	
	redisReply *reply = (redisReply *)redisCommand(c,"SCRIPT LOAD %s",script.c_str());
	script_sha = reply->str;
	
    freeReplyObject(reply);
	redisFree(c);  
	
	return REDIS_OK;
}


/**
 * 
 * 执行Script Load Lua脚本(add_multikv_script,add_zset_script,incrby_script,add_rset_script)
 * 
*/
int RedisProto::RedisProtoEncodeLoadScript(qkpack_request_t *request)
{
	int									rc;
	std::string							script_sha;
	
	if ( map_script_[request->node_ip][redis_proto_add_multikv_name_].empty() ) {
		
		rc = InitClusterScript(request, redis_proto_add_multikv_script_, script_sha);
		if ( rc != REDIS_OK ) {
			return REDIS_ERROR;
		}
		map_script_[request->node_ip][redis_proto_add_multikv_name_] = script_sha;
	}
	
	if ( map_script_[request->node_ip][redis_proto_add_zset_name_].empty() ) {
		
		rc = InitClusterScript(request, redis_proto_add_zset_script_, script_sha);
		if ( rc != REDIS_OK ) {
			return REDIS_ERROR;
		}
		map_script_[request->node_ip][redis_proto_add_zset_name_] = script_sha;
	}

	if ( map_script_[request->node_ip][redis_proto_incrby_name_].empty() ) {
		
		rc = InitClusterScript(request, redis_proto_incrby_script_, script_sha);
		if ( rc != REDIS_OK ) {
			return REDIS_ERROR;
		}
		map_script_[request->node_ip][redis_proto_incrby_name_] = script_sha;
	}

	if ( map_script_[request->node_ip][redis_proto_add_rset_name_].empty() ) {
		
		rc = InitClusterScript(request, redis_proto_add_rset_script_, script_sha);
		if ( rc != REDIS_OK ) {
			return REDIS_ERROR;
		}
		map_script_[request->node_ip][redis_proto_add_rset_name_] = script_sha;
	}

	return REDIS_OK;
}



/**
 * 
 * 压缩Value
 * 
*/
bool RedisProto::CompressValue(const std::string &value, std::string &compress_value, int compress_threshold)
{
	int										compress_len;
	int										len = value.length();
	
	if ( len < compress_threshold ) {
		return false;
	}


	compress_len = compress_->Compress(value,compress_value);	

	if ( compress_len > len ) {
		return false;
	}

	return true;
}


/**
 * 
 * 解压Value
 * 
*/
bool RedisProto::UncompressValue(const std::string &value,std::string &src_value)
{
	bool									b;

	//判断是否是snappy格式
	b = compress_->IsValidCompressed(value);
	if ( !b ) {
		return false;
	}

	//解压snappy格式
	b = compress_->Uncompress(value,src_value);
	if ( !b ) {
		return false;
	}
	
	return true;
}



/**
 * 
 * 调用hiredis构建redis协议
 * 
*/
int RedisProto::RedisProtoCmd(std::string &proto_str,size_t argc, const std::vector<std::string> &vector_argv, const std::vector<int> &vector_lens)
{
	int										len;
	size_t									size = LONG_LEN;
	char									buf[size];

	len = safe_snprintf(buf, size,"*%lu\r\n", (unsigned long) argc);	
	proto_str = std::string(buf,len);
	
	for (size_t i = 0; i < argc; i++)
	{
		len = safe_snprintf(buf, size,"$%lu\r\n", (unsigned long) vector_lens[i]);	
		proto_str.append(buf,len);
		
		proto_str.append(vector_argv[i].c_str(), vector_lens[i]);
		proto_str.append("\r\n");
	}

	return REDIS_OK;
}


/**
 * 
 * 调用hiredis构建redis协议
 * 
*/
int RedisProto::RedisProtoCmdArray(std::string &proto_str,size_t argc, const char* argv[], const int lens[])
{
	int										len;
	size_t									size = LONG_LEN;
	char									buf[size];

	len = safe_snprintf(buf, size,"*%lu\r\n", (unsigned long) argc);	
	proto_str = std::string(buf,len);
	
	for (size_t i = 0; i < argc; i++)
	{
		len = safe_snprintf(buf, size,"$%lu\r\n", (unsigned long) lens[i]);	
		proto_str.append(buf,len);
		
		proto_str.append(argv[i], lens[i]);
		proto_str.append("\r\n");
	}

	return REDIS_OK;
}

/**
 * 
 * 调用hiredis构建redis协议
 * 
*/
int RedisProto::RedisProtoCmdOther(std::string &proto_str,size_t argc,const std::vector<std::string> &vector_argv, const std::vector<int> &vector_lens)
{
	int										len;
	size_t									lens[argc];			
	char									*buf;
	const char								*argv[argc];								

	for ( size_t i = 0; i < argc; ++i ) {

		argv[i] = vector_argv[i].c_str();
		lens[i] = vector_lens[i];
	}

	len = redisFormatCommandArgv(&buf,argc,argv,lens);
	proto_str = std::string(buf,len);
	free(buf);

	return REDIS_OK;
}


/**
 * 
 * 构建Redis协议 ping
 * 
*/
int RedisProto::RedisProtoEncodePing(qkpack_request_t *request) 
{
	size_t                                  argc = 1;
	const char*                             argv[argc];
	int                                     lens[argc];

	argv[0] = REDIS_PING_COMMAND;
	lens[0] = strlen(argv[0]);

	return RedisProtoCmdArray(request->kvpair.proto_str, argc, argv, lens);
} 


/**
 * 
 * 构建Redis协议 GET,DEL,TTL,SCARD,SMEMBERS
 * 
*/
int RedisProto::RedisProtoEncodeParamOne(qkpack_request_t *request) 
{
	size_t                                  argc = 2;
	const char*								argv[argc];
	int										lens[argc];
	std::string								ns_key;
	std::string								&command = request->kvpair.command;

	ns_key.append(request->kvpair.namespaces);
	ns_key.append(request->kvpair.key);
	
	//argv add command
	argv[0] = command.c_str();
	lens[0] = command.length();

	//argv add namespaces key
	argv[1] = ns_key.c_str();
	lens[1] = ns_key.length();

	return RedisProtoCmdArray(request->kvpair.proto_str, argc, argv, lens);
} 


/**
 * 
 * 构建Redis协议 SISMEMBER
 * 
*/
int RedisProto::RedisProtoEncodeParamTwo(qkpack_request_t *request) 
{
	size_t                                  argc = 3;
	const char*								argv[argc];
	int										lens[argc];
	bool									b;
	std::string								compress_value;
	std::string								&command = request->kvpair.command;
	std::string								&value = request->kvpair.value; 
	std::string								ns_key;

	ns_key.append(request->kvpair.namespaces);
	ns_key.append(request->kvpair.key);
	
	//argv add cmd
	argv[0] = command.c_str();
	lens[0] = command.length();

	//argv add namespaces key
	argv[1] = ns_key.c_str();
	lens[1] = ns_key.length();

	//compress value
	b = CompressValue(value,compress_value, request->kvpair.compress_threshold);
	if ( b ) {	
		//argv add value
		argv[2] = compress_value.c_str();
		lens[2] = compress_value.length();
	}
	else {
		//argv add value
		argv[2] = value.c_str();
		lens[2] = value.length();
	}

	return RedisProtoCmdArray(request->kvpair.proto_str, argc, argv, lens);
}



/**
 * 
 * 构建Redis协议 SETEX
 * 
*/
int RedisProto::RedisProtoEncodeParamThree(qkpack_request_t *request) 
{
	size_t                                  argc = 4;
	const char*								argv[argc];
	int										lens[argc];
	bool									b;
	std::string								ns_key;
	std::string								compress_value;
	std::string								&command = request->kvpair.command;
	std::string								&value = request->kvpair.value; 
	std::string								exptime = StringUtil::ToString(request->kvpair.exptime);

	ns_key.append(request->kvpair.namespaces);
	ns_key.append(request->kvpair.key);

	//argv add cmd
	argv[0] = command.c_str();
	lens[0] = command.length();

	//argv add namespaces key
	argv[1] = ns_key.c_str();
	lens[1] = ns_key.length();

	//argv add exptime
	argv[2] = exptime.c_str();
	lens[2] = exptime.length();
	
	//compress value
	b = CompressValue(value,compress_value, request->kvpair.compress_threshold);
	if ( b ) {	
		//argv add value
		argv[3] = compress_value.c_str();
		lens[3] = compress_value.length();
	}
	else {
		//argv add value
		argv[3] = value.c_str();
		lens[3] = value.length();
	}

	return RedisProtoCmdArray(request->kvpair.proto_str, argc, argv, lens);
}

/**
 * 
 * 构建Redis协议 MGET
 * 
*/
int RedisProto::RedisProtoEncodeParamMulti(qkpack_request_t *request)
{
	std::string								ns_key;
	std::map<int, std::vector<kvpair_t*> >	&map_slots = request->parent->map_slots_kv;
	std::string								&namespaces = request->parent->kvpair.namespaces;
	std::string								&command = request->kvpair.command;

	request->kvpair.script_name = QKPACK_METRICS_G_MGET;

	//MGET子请求的key，已经划分好了. key都属于同一个slotid，所以这里只需要取一次slotid下面的所有key
	std::map<int, std::vector<kvpair_t*> >::iterator  it;
	it = map_slots.find(request->slot_id);
	if ( it == map_slots.end() ) {
		return REDIS_ERROR;
	}

	std::vector<kvpair_t*>					&vector_kvpair = it->second;		
	size_t									argc = vector_kvpair.size() + 1;
	std::vector<std::string>				vector_argv;
	std::vector<int>						vector_lens;

	//argv add cmd
	vector_argv.push_back(command);
	vector_lens.push_back(command.length());
	
	for ( size_t i = 0; i < vector_kvpair.size(); ++i ) {
		ns_key.clear();
		ns_key.append(namespaces);
		ns_key.append(vector_kvpair[i]->key);

		vector_argv.push_back(ns_key);
		vector_lens.push_back(ns_key.length());
	}

	return RedisProtoCmd(request->kvpair.proto_str, argc, vector_argv, vector_lens);
}


/**
 * 
 * 构建Redis协议 SREM
 * 
*/
int RedisProto::RedisProtoEncodeSrem(qkpack_request_t *request)
{
	bool									b;
	std::string								ns_key;
	std::string								compress_value;
	std::vector<zset_member_t>				&vector_mbs = request->vector_mbs;
	std::string								&namespaces = request->kvpair.namespaces;
	std::string								&key = request->kvpair.key;
	std::string								&command = request->kvpair.command;
	size_t									argc = vector_mbs.size() + 2;
	std::vector<std::string>				vector_argv;
	std::vector<int>						vector_lens;
	
	ns_key.append(namespaces);
	ns_key.append(key);
	
	//argv add cmd
	vector_argv.push_back(command);
	vector_lens.push_back(command.length());
	
	//argv add namespaces key
	vector_argv.push_back(ns_key);
	vector_lens.push_back(ns_key.length());

	for ( size_t i = 0; i < vector_mbs.size(); ++i ) {
	
		//compress member
		std::string &value = vector_mbs[i].member;
		b = CompressValue(value, compress_value, request->kvpair.compress_threshold);
		
		if ( b ) {
			//argv add value
			vector_argv.push_back(compress_value);
			vector_lens.push_back(compress_value.length());
		}
		else {
			//argv add value
			vector_argv.push_back(value);
			vector_lens.push_back(value.length());
		}	
	}

	return RedisProtoCmd(request->kvpair.proto_str, argc, vector_argv, vector_lens);
}


/**
 * 
 * 构建Redis协议 SADD Lua脚本
 * 
*/
int RedisProto::RedisProtoEncodeSadd(qkpack_request_t *request)
{
	int										rc;
	bool									b;
	std::string								ns_key;
	std::string								compress_value;
	std::string								&key = request->kvpair.key;
	std::string								exptime = StringUtil::ToString(request->kvpair.exptime);
	std::vector<zset_member_t>				&vector_mbs = request->vector_mbs;
	std::string								&namespaces = request->kvpair.namespaces;
	std::string								&command = request->kvpair.command;
	std::string								script_sha;
	size_t									argc = vector_mbs.size() + 6;
	std::vector<std::string>				vector_argv;
	std::vector<int>						vector_lens;

	//script load
	rc = RedisProtoEncodeLoadScript(request);
	if ( rc != REDIS_OK ) {
		return REDIS_ERROR;
	}

	ns_key.append(namespaces);
	ns_key.append(key);
	
	//argv add cmd
	command = REDIS_SCRIPT_EVALSHA_COMMAND;
	vector_argv.push_back(command);
	vector_lens.push_back(command.length());

	//argv add script_sha
	request->kvpair.script_name = redis_proto_add_rset_name_;
	script_sha = map_script_[request->node_ip][redis_proto_add_rset_name_];
	if ( script_sha.empty() ) {
		return REDIS_ERROR;
	}
	
	//argv add script_sha
	vector_argv.push_back(script_sha);
	vector_lens.push_back(script_sha.length());

	//argv add key num
	vector_argv.push_back("1");
	vector_lens.push_back(1);

	//argv add namespaces key
	vector_argv.push_back(ns_key);
	vector_lens.push_back(ns_key.length());

	//argv add exptime
	vector_argv.push_back(exptime);
	vector_lens.push_back(exptime.length());

	//argv add member count
	std::string mbs_str = StringUtil::ToString(vector_mbs.size());
	vector_argv.push_back(mbs_str);
	vector_lens.push_back(mbs_str.length());
	
	for ( size_t i = 0; i < vector_mbs.size(); ++i ) {
		
		//compress member
		std::string &value = vector_mbs[i].member;
		b =	CompressValue(value, compress_value, request->kvpair.compress_threshold);
		
		if ( b ) {
		
			//argv add value
			vector_argv.push_back(compress_value);
			vector_lens.push_back(compress_value.length());
		}
		else {
		
			//argv add value
			vector_argv.push_back(value);
			vector_lens.push_back(value.length());
		}
	}
	
	return RedisProtoCmd(request->kvpair.proto_str, argc, vector_argv, vector_lens);
}


/**
 * 
 * 构建Redis协议 INCRBY Lua脚本 (INCR, INCRBY)
 * 
*/
int RedisProto::RedisProtoEncodeIncrBy(qkpack_request_t *request)
{
	int										rc;
	std::string								ns_key;
	std::string								compress_value;
	std::string								script_sha;
	std::string								&key = request->kvpair.key;
	std::string								&namespaces = request->kvpair.namespaces;
	std::string								&command = request->kvpair.command;
	std::string								&value = request->kvpair.value;
	std::string								exptime = StringUtil::ToString(request->kvpair.exptime);
	size_t									argc =  6;
	const char*								argv[argc];
	int										lens[argc];

	ns_key.append(namespaces);
	ns_key.append(key);	

	//script load
	rc = RedisProtoEncodeLoadScript(request);
	if ( rc != REDIS_OK ) {
		return REDIS_ERROR;
	}

	if ( request->kvpair.command_type == REDIS_INCR ) {
		command = REDIS_INCR_COMMAND;
		if ( value.empty() ) {
			value = "1";
		}
	}
	else if ( request->kvpair.command_type == REDIS_INCRBY ) {
		request->kvpair.script_name = redis_proto_incrby_name_;
		command = REDIS_SCRIPT_EVALSHA_COMMAND;
		
		if ( value.empty() ) {
			return REDIS_ERROR;
		}
	}
	
	//argv add cmd
	argv[0] = REDIS_SCRIPT_EVALSHA_COMMAND;
	lens[0] = strlen(argv[0]);

	//argv add script_sha 
	script_sha = map_script_[request->node_ip][redis_proto_incrby_name_];
	if ( script_sha.empty() ) {
		return REDIS_ERROR;
	}

	//argv add script_sha
	argv[1] = script_sha.c_str();
	lens[1] = script_sha.length();

	//argv add key num
	argv[2] = "1";
	lens[2] = 1;

	//argv add namespaces key
	argv[3] = ns_key.c_str();
	lens[3] = ns_key.length();

	//argv add value
	argv[4] = value.c_str();
	lens[4] = value.length();
	
	//argv add exptime
	argv[5] = exptime.c_str();
	lens[5] = exptime.length();
	
	return RedisProtoCmdArray(request->kvpair.proto_str, argc, argv, lens);
}


/**
 * 
 * 构建Redis协议 MSET Lua脚本 
 * 
*/
int RedisProto::RedisProtoEncodeMset(qkpack_request_t *request)
{
	bool									b;
	int										rc;
	std::string								ns_key;
	std::string								compress_value;
	std::string								script_sha;
	std::string								&namespaces = request->parent->kvpair.namespaces;
	std::string								&command = request->kvpair.command;
	std::string								exptime = StringUtil::ToString(request->parent->kvpair.exptime);
	std::string								expt;
	std::map<int, std::vector<kvpair_t*> >  &map_slots = request->parent->map_slots_kv;
	
	//子请求的k/v，已经划分好了. k/v都属于同一个slotid，所以这里只需要取一次slotid下面的所有k/v
	std::map<int, std::vector<kvpair_t*> >::iterator  it;
	
	it = map_slots.find(request->slot_id);
	if ( it == map_slots.end() ) {
		return REDIS_ERROR;
	}
	std::vector<kvpair_t*>					&vector_kv	= it->second;
	size_t									kv_size = vector_kv.size();
	size_t									argc =  3 + kv_size * 3;
	std::vector<std::string>				vector_argv;
	std::vector<int>						vector_lens;
	
	//script load
	rc = RedisProtoEncodeLoadScript(request);
	if ( rc != REDIS_OK ) {
		return REDIS_ERROR;
	}
	
	//argv add cmd
	command = REDIS_SCRIPT_EVALSHA_COMMAND;
	vector_argv.push_back(command);
	vector_lens.push_back(command.length());

	request->kvpair.script_name = redis_proto_add_multikv_name_;
	script_sha = map_script_[request->node_ip][redis_proto_add_multikv_name_];
	if ( script_sha.empty() ) {
		return REDIS_ERROR;
	}

	//argv add script_sha
	vector_argv.push_back(script_sha);
	vector_lens.push_back(script_sha.length());

	//argv add key num
	std::string key_num = StringUtil::ToString(kv_size);
	vector_argv.push_back(key_num);
	vector_lens.push_back(key_num.length());
	
	for ( size_t i = 0; i < kv_size; ++i ) {
		
		//argv add namespaces key
		ns_key.clear();
		ns_key.append(namespaces);
		ns_key.append(vector_kv[i]->key);

		vector_argv.push_back(ns_key);
		vector_lens.push_back(ns_key.length());
	}

	for ( size_t i = 0; i < kv_size; ++i ) {
	
		//compress value
		b = CompressValue(vector_kv[i]->value, compress_value, request->kvpair.compress_threshold);
		if ( b ) {
			//argv add value
			vector_argv.push_back(compress_value);
			vector_lens.push_back(compress_value.length());
		}
		else {
			//argv add value
			vector_argv.push_back(vector_kv[i]->value);
			vector_lens.push_back(vector_kv[i]->value.length());
		}
	}

	for ( size_t i = 0; i < it->second.size(); ++i ) {
		vector_argv.push_back(exptime);
		vector_lens.push_back(exptime.length());
	}
	
	return RedisProtoCmd(request->kvpair.proto_str, argc ,vector_argv, vector_lens);
}


/**
 * 
 * 构建Redis协议 ZfixedsetAdd Lua脚本 
 * 
*/
int RedisProto::RedisProtoEncodeZfixedsetAdd(qkpack_request_t *request)
{
	int													rc;
	std::string											ns_key;
	std::string											member_key;
	std::string											compress_value;
	std::string											script_sha;
	std::string											scor;
	std::string											&key = request->kvpair.key;
	std::string											&command = request->kvpair.command;
	std::string											&namespaces = request->parent->kvpair.namespaces;
	std::string											exptime = StringUtil::ToString(request->parent->kvpair.exptime);
	
	std::map<std::string, std::vector<kvpair_t> >			&map_kv = request->parent->map_kv;
	std::map<std::string, std::vector<kvpair_t> >::iterator	it;

	it = map_kv.find(key);
	if ( it == map_kv.end() ) {
		return REDIS_ERROR; 
	}

	std::vector<kvpair_t>							    vector_kv =  it->second;
	size_t												count = vector_kv.size();
	size_t												argc =  7 + count * 2;
	std::vector<std::string>							vector_argv;
	std::vector<int>									vector_lens;
	std::string											limit = StringUtil::ToString(vector_kv[0].limit);
	
	ns_key.append(namespaces);
	ns_key.append(key);	
	
	//script load
	rc = RedisProtoEncodeLoadScript(request);
	if ( rc != REDIS_OK ) {
		return REDIS_ERROR;
	}
	
	//argv add cmd
	command = REDIS_SCRIPT_EVALSHA_COMMAND;
	vector_argv.push_back(command);
	vector_lens.push_back(command.length());

	//argv add script_sha 
	request->kvpair.script_name = redis_proto_add_zset_name_;
	script_sha = map_script_[request->node_ip][redis_proto_add_zset_name_];
	if ( script_sha.empty() ) {
		return REDIS_ERROR;
	}
	
	//argv add script_sha
	vector_argv.push_back(script_sha);
	vector_lens.push_back(script_sha.length());

	//argv add key num
	vector_argv.push_back("1");
	vector_lens.push_back(1);

	//argv add namespaces key
	vector_argv.push_back(ns_key);
	vector_lens.push_back(ns_key.length());

	//argv add limit
	vector_argv.push_back(limit);
	vector_lens.push_back(limit.length());

	//argv add exptime
	vector_argv.push_back(exptime);
	vector_lens.push_back(exptime.length());

	//argv add member count
	std::string str_mbs = StringUtil::ToString(count);
	vector_argv.push_back(str_mbs);
	vector_lens.push_back(str_mbs.length());

	for ( size_t i = 0; i < count; ++i ) {
		kvpair_t &kvpair = vector_kv[i];

		//argv add score
		scor.clear();
		scor.append(StringUtil::DoubleToString(kvpair.score));
		vector_argv.push_back(scor);
		vector_lens.push_back(scor.length());

		//argv add member
		member_key.clear();
		member_key.append(namespaces);
		member_key.append(kvpair.key);
		
		vector_argv.push_back(member_key);
		vector_lens.push_back(member_key.length());
	}
	
	return RedisProtoCmd(request->kvpair.proto_str, argc, vector_argv, vector_lens);
}


/**
 * 
 * 构建Redis协议 ZfixedsetGetByScore
 * 
*/
int RedisProto::RedisProtoEncodeZfixedsetGetByScore(qkpack_request_t *request)
{
	std::string											ns_key;
	std::string											&command = request->kvpair.command;
	std::string											&key = request->kvpair.key;
	
	std::string											&namespaces = request->parent->kvpair.namespaces;
	std::map<std::string, zset_query_t>					&map_query = request->parent->map_query;
	std::map<std::string, zset_query_t>::iterator		it;

	it = map_query.find(key);
	if ( it == map_query.end() ) {
		return REDIS_ERROR;
	}

	zset_query_t										&zset_query = it->second;
	size_t												argc = zset_query.ws?5:4;
	const char*											argv[argc];
	int													lens[argc];

	ns_key.append(namespaces);
	ns_key.append(request->kvpair.key);
	
	if ( zset_query.asc ) {
		command = REDIS_ZRANGEBYSCORE_COMMAND;
	}else {
		command = REDIS_ZREVRANGEBYSCORE_COMMAND;	
	}

	//argv add cmd
	request->kvpair.script_name = QKPACK_METRICS_G_ZRANGE;
	argv[0] = command.c_str();
	lens[0] = command.length();

	//argv add namespaces key
	argv[1] = ns_key.c_str();
	lens[1] = ns_key.length();

	std::string str_min = StringUtil::DoubleToString(zset_query.min);
	std::string str_max = StringUtil::DoubleToString(zset_query.max);

	if ( zset_query.asc ) {
		
		//argv add min	
		argv[2] = str_min.c_str();
		lens[2] = str_min.length();
		
		//argv add max
		argv[3] = str_max.c_str();
		lens[3] = str_max.length();

	}else {

		//argv add max
		argv[2] = str_max.c_str();
		lens[2] = str_max.length();
		
		//argv add min
		argv[3] = str_min.c_str();
		lens[3] = str_min.length();
	}

	if ( zset_query.ws ) {
	
		//argv add withscores
		argv[4] = REDIS_ZRANGE_WITHSCORES_COMMAND;
		lens[4] = strlen(argv[4]);
	}

	return RedisProtoCmdArray(request->kvpair.proto_str, argc, argv, lens);
}


/**
 * 
 * 构建Redis协议 ZfixedsetGetByRank
 * 
*/
int RedisProto::RedisProtoEncodeZfixedsetGetByRank(qkpack_request_t *request)
{
	std::string											ns_key;
	std::string											&command = request->kvpair.command;
	std::string											&key = request->kvpair.key;
	
	std::string											&namespaces = request->parent->kvpair.namespaces;
	std::map<std::string, zset_query_t>					&map_query = request->parent->map_query;
	std::map<std::string, zset_query_t>::iterator		it;

	it = map_query.find(key);
	if ( it == map_query.end() ) {
		return REDIS_ERROR;
	}

	zset_query_t										&zset_query = it->second;
	size_t												argc = zset_query.ws?5:4;
	int													lens[argc];
	const char*											argv[argc];
	
	ns_key.append(namespaces);
	ns_key.append(request->kvpair.key);
	
	if ( zset_query.asc ) {
		command = REDIS_ZRANGE_COMMAND;
	}
	else {
		command = REDIS_ZREVRANGE_COMMAND;
	}
	
	//argv add cmd
	request->kvpair.script_name = QKPACK_METRICS_G_ZRANGE;
	argv[0] = command.c_str();
	lens[0] = command.length();
	
	//argv add namespaces key
	argv[1] = ns_key.c_str();
	lens[1] = ns_key.length();
	
	std::string str_min = StringUtil::ToString((int64_t)zset_query.min);
	std::string str_max = StringUtil::ToString((int64_t)zset_query.max);

	//argv add min
	argv[2] = str_min.c_str();
	lens[2] = str_min.length();
		
	//argv add max
	argv[3] = str_max.c_str();
	lens[3] = str_max.length();

	if ( zset_query.ws ) {
	
		//argv add withscores
		argv[4] = REDIS_ZRANGE_WITHSCORES_COMMAND;
		lens[4] = strlen(argv[4]);
	}

	return RedisProtoCmdArray(request->kvpair.proto_str, argc, argv, lens);
}


/**
 * 
 * 构建Redis协议
 * 
*/
int RedisProto::RedisProtoEncode(qkpack_request_t *request) 
{
	//判断是否解析错误，如果解析错误，发送ping命令
	if ( request->code != QKPACK_RESPONSE_CODE_OK ) {
		return RedisProtoEncodePing(request);
	}

	switch(request->kvpair.command_type)
	{
		case REDIS_GET:
			request->kvpair.command = REDIS_GET_COMMAND;
			return RedisProtoEncodeParamOne(request);
		case REDIS_DEL:
			request->kvpair.command = REDIS_DEL_COMMAND;
			return RedisProtoEncodeParamOne(request);
		case REDIS_TTL:
			request->kvpair.command = REDIS_TTL_COMMAND;
			return RedisProtoEncodeParamOne(request);
		case REDIS_SET:
			request->kvpair.command = REDIS_SET_COMMAND;
			return RedisProtoEncodeParamThree(request);
		case REDIS_INCR:
		case REDIS_INCRBY:
			return RedisProtoEncodeIncrBy(request);
		case REDIS_SADD:
			request->kvpair.command = REDIS_SADD_COMMAND;
			return RedisProtoEncodeSadd(request);
		case REDIS_SREM:
			request->kvpair.command = REDIS_SREM_COMMAND;
			return RedisProtoEncodeSrem(request);
		case REDIS_SCARD:
			request->kvpair.command = REDIS_SCARD_COMMAND;
			return RedisProtoEncodeParamOne(request);
		case REDIS_SMEMBERS:
			request->kvpair.command = REDIS_SMEMBERS_COMMAND;
			return RedisProtoEncodeParamOne(request);
		case REDIS_SISMEMBER:
			request->kvpair.command = REDIS_SISMEMBER_COMMAND;
			return RedisProtoEncodeParamTwo(request);
		case REDIS_MGET:
			request->kvpair.command = REDIS_MGET_COMMAND;
			return RedisProtoEncodeParamMulti(request);
		case REDIS_MSET:
			return RedisProtoEncodeMset(request);
		case REDIS_ZADD:
			return RedisProtoEncodeZfixedsetAdd(request);
		case REDIS_ZBATCHADD:
			return REDIS_OK;
		case REDIS_ZRANGEBYSCORE:
			return RedisProtoEncodeZfixedsetGetByScore(request);
		case REDIS_ZRANGE:
			return RedisProtoEncodeZfixedsetGetByRank(request);
	}

	return REDIS_ERROR;
}


/**
 * 
 * 解码Redis协议GetString
 * 
*/
int RedisProto::RedisProtoDecodeGetString(char *rdata,qkpack_request_t *request)
{
	//$3\r\nabc\r\n
	bool									b;
	int										bulklen;
	char									*rest;
	std::string								value;
	std::string								src_value;
	
	bulklen = strtol(rdata, &rest, 0);
	if (bulklen == -1) {
		request->kvpair.value = "null";
		return REDIS_OK;
	}
	rest += 2;
	value = std::string(rest, bulklen);
	
	//uncompress value
	b = UncompressValue(value, src_value);
	if ( b ) {
		request->kvpair.value = src_value;
	}else  {
		request->kvpair.value = value;
	}

	return REDIS_OK;	
}


/**
 * 
 * 解码Redis协议GetStatus
 * 
*/
int RedisProto::RedisProtoDecodeGetStatus(char *rdata,qkpack_request_t *request)
{
	//+OK\r\n
	char									*p = rdata;

	while(strncmp(p, "\r\n", 2) != 0) p++;
	int size = p - rdata;
	size = ( size == -1) ? strlen(rdata) : size;
	
	if ( size < 0 ) {
		return REDIS_ERROR;
	}

	request->kvpair.value = std::string(rdata, size);
	
	return REDIS_OK;
}


/**
 * 
 * 解码Redis协议GetError
 * 
*/
int RedisProto::RedisProtoDecodeGetError(char *rdata,qkpack_request_t *request)
{
	//-MOVED || -ASK || -ERR
	int										rc;
	char									*p = rdata;
	
	//MOVED 1 127.0.0.1
	if ( strncmp(p,"MOVED",sizeof("MOVED")-1) == 0 ) {
		return REDIS_MOVED;
	}
	
	//ASK 1 127.0.0.1
	if ( strncmp(p,"ASK",sizeof("ASK")-1) == 0 ) {
		return REDIS_ASK;
	}
	
	//TRYAGAIN
	if ( strncmp(p,"TRYAGAIN",sizeof("TRYAGAIN")-1) == 0 ) {
		return REDIS_TRYAGAIN;
	}

	//NOSCRIPT
	if ( strncmp(p,"NOSCRIPT",sizeof("NOSCRIPT")-1) == 0 ) {
		//script load
		rc = RedisProtoEncodeLoadScript(request);
		if ( rc != REDIS_OK ) {
			return REDIS_ERROR;
		}
		
		return REDIS_TRYAGAIN;
	}

	//ERR 
	while(strncmp(p, "\r\n", 2) != 0) p++;
	int size = p - rdata;
	size = ( size == -1) ? strlen(rdata) : size;
	
	if ( size < 0 ) {
		return REDIS_ERROR;
	}

	request->desc = std::string(rdata, size);
	request->code = QKPACK_RESPONSE_CODE_REDIS_STORAGE_ERROR;
	
	return REDIS_OK;
}


/**
 * 
 * 解码Redis协议GetInteger
 * 
*/
int RedisProto::RedisProtoDecodeGetInteger(char *rdata,qkpack_request_t *request)
{
	//:1000\r\n
	char									*rest;
	
	/* bstr_t can be used as char* */
	request->kvpair.num = strtol(rdata, &rest, 0);
	
	return REDIS_OK;	
}


/**
 * 
 * 解码Redis协议GetArray
 * 
*/
int RedisProto::RedisProtoDecodeGetArray(char *rdata,qkpack_request_t *request)
{
	//*3\r\n$4\r\n1000\r\n$4\r\n1000\r\n$4\r\n1000\r\n
	bool									b;
	int										bulklen;
	char									*rest;
	int										i;
	std::string								member;
	std::string								score;
	std::string								value;
	std::string 							src_value;

	int multibulkSize = strtol(rdata, &rest, 0);
	if (multibulkSize == -1) {
		return REDIS_ERROR;
	}

	rest += 2;                          
	i = 0;
	
	while (i < multibulkSize)       
	{
		rest++;                           
		bulklen = strtol(rest, &rest, 0); 
		rest += 2;                        
		if (bulklen == -1)                
		{
			if ( request->kvpair.command_type == REDIS_MGET ||
					request->kvpair.command_type == REDIS_MSET ) {
				request->parent->map_slots_kv[request->slot_id][i]->value = "null";
			}
		    i++;
		    continue;
		}
	
		if ( request->kvpair.command_type == REDIS_MGET ||
				request->kvpair.command_type == REDIS_MSET ) {
			
			value = std::string(rest, bulklen);
			
			//uncompress value
			b = UncompressValue(value, src_value);
			if ( b ) {
				request->parent->map_slots_kv[request->slot_id][i]->value = src_value;
			}else {
				request->parent->map_slots_kv[request->slot_id][i]->value = value;
			}
		}
		
		else if ( request->kvpair.command_type == REDIS_SMEMBERS ) {
			zset_member_t mb;
			mb.member = std::string(rest,bulklen);
			
			//uncompress value
			b = UncompressValue(mb.member, src_value);
			if ( b ) {
				mb.member = src_value;
			}

			request->vector_mbs.push_back(mb);
		}

		else if ( request->kvpair.command_type == REDIS_ZRANGEBYSCORE ||
				request->kvpair.command_type == REDIS_ZRANGE ) {
			if ( i % 2 ) {
				score = std::string (rest, bulklen);
				kvpair_t kvpair;
				kvpair.key = member;
				StringUtil::StrToDouble(score.c_str(),kvpair.score);

				request->parent->map_kv[request->kvpair.key].push_back(kvpair);
				
				member.clear();
				score.clear();

			}else {
				member = std::string(rest, bulklen).replace(0,request->parent->kvpair.namespaces.length(), "");
			}
		}
		
		rest += (bulklen + 2);
		i++;
	}
	
	return REDIS_OK;
}


/**
 * 
 * 解码Redis协议
 * 
*/
int RedisProto::RedisProtoDecode(qkpack_request_t *request) 
{
	int										rc;

	if ( request->response_buffer.empty() ) {
		return REDIS_ERROR;
	}

	//request error
	if ( request->code != QKPACK_RESPONSE_CODE_OK ) {
		return REDIS_ERROR;
	}

	char* rdata = (char*)request->response_buffer.c_str();
	
	switch (*rdata)	
	{
		case '-':	// ERROR
			return RedisProtoDecodeGetError(++rdata,request);
		case '+':	// STATUS
			return RedisProtoDecodeGetStatus(++rdata,request);
		case ':':	// INTEGER
			return RedisProtoDecodeGetInteger(++rdata,request);
		case '$':	// STRING
			rc = RedisProtoDecodeGetString(++rdata,request);
			break;
		case '*':	// ARRAY
			rc = RedisProtoDecodeGetArray(++rdata,request);
			break;
		default:	// INVALID
			return REDIS_ERROR;
	}
	
	if ( request->kvpair.command_type == REDIS_TTL ) {
		if ( request->kvpair.num == -1 ) {
		
			request->desc = QKPACK_ERROR_TTL_KEY_NO_EXPIRE; 
			request->code = QKPACK_RESPONSE_CODE_REDIS_STORAGE_ERROR;
			
			return REDIS_ERROR;
		}
		else if ( request->kvpair.num == -2 ) {
			
			request->desc = QKPACK_ERROR_TTL_KEY_NOT_EXIST;
			request->code = QKPACK_RESPONSE_CODE_REDIS_STORAGE_ERROR;
			
			return REDIS_ERROR;
		}
	}

	return rc;
}


