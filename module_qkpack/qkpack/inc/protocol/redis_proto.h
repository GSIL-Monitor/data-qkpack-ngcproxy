#ifndef __REDIS_PROTO_H__
#define __REDIS_PROTO_H__

#include "common.h" 
#include "qkpack_common.h"
#include "snappys/compress.h"

using namespace com::youku::data::qkpack::snappys;

namespace com { 
namespace youku { 
namespace data {
namespace qkpack {
namespace protocol {


class RedisProto 
{
public:
	RedisProto(std::string add_multikv_script,std::string add_zset_script,
			std::string incrby_script,std::string add_rset_script);
	virtual ~RedisProto();

public:
	
	int RedisProtoEncode(qkpack_request_t *request);
	
	int RedisProtoDecode(qkpack_request_t *request);

protected:
	int InitClusterScript(qkpack_request_t *request,const std::string &script, std::string &script_sha);
	int RedisProtoEncodeLoadScript(qkpack_request_t *request);
	int RedisProtoCmd(std::string &proto_str,size_t argc, const std::vector<std::string> &vector_argv
			,const std::vector<int> &vector_lens);
	int RedisProtoCmdArray(std::string &proto_str,size_t argc, const char *argv[], const int lens[]);

	int RedisProtoCmdOther(std::string &proto_str,size_t argc, const std::vector<std::string> &vector_argv,
			const std::vector<int> &vector_len);

	std::string GetRedisKey(const std::string &namespaces, const std::string &key);
	bool CompressValue(const std::string &value, std::string &compress_value, int compress_threshold);
	bool UncompressValue(const std::string &value,std::string &src_value);

	int RedisProtoEncodePing(qkpack_request_t *request);
	int RedisProtoEncodeParamOne(qkpack_request_t *request);
	int RedisProtoEncodeParamTwo(qkpack_request_t *request);

	/**
	 * 
	 * 构建Redis协议 SETEX
	 * 
	*/
	int RedisProtoEncodeParamThree(qkpack_request_t *request); 
	
	int RedisProtoEncodeParamMulti(qkpack_request_t *request); 
	int RedisProtoEncodeSadd(qkpack_request_t *request);
	int RedisProtoEncodeSrem(qkpack_request_t *request);

	int RedisProtoEncodeIncrBy(qkpack_request_t *request);
	int RedisProtoEncodeMset(qkpack_request_t *request);
	int RedisProtoEncodeZfixedsetAdd(qkpack_request_t *request);
	int RedisProtoEncodeZfixedsetGetByScore(qkpack_request_t *request);
	int RedisProtoEncodeZfixedsetGetByRank(qkpack_request_t *request);

	
	int RedisProtoDecodeGetString(char *rdata,qkpack_request_t *request);
	int RedisProtoDecodeGetStatus(char *rdata,qkpack_request_t *request);
	int RedisProtoDecodeGetError(char *rdata,qkpack_request_t *request);
	int RedisProtoDecodeGetInteger(char *rdata,qkpack_request_t *request);
	int RedisProtoDecodeGetArray(char *rdata,qkpack_request_t *request);
	
private:
	std::map<std::string, std::map<std::string,std::string> >  map_script_;
	std::string redis_proto_add_multikv_script_;
	std::string redis_proto_add_zset_script_;
	std::string redis_proto_incrby_script_;
	std::string redis_proto_add_rset_script_;
	
	ICompress *compress_;	
};

} /* namespace protocol */
} /* namespace qkpack */
} /* namespace data */
} /* namespace youku */
} /* namespace com */


#endif //__REDIS_PROTO_H__
