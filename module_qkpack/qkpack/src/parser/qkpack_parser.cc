#include "parser/qkpack_parser.h"
#include "parser/qkpack_yajl_json_impl.h"
#include "qkpack_common.h"
#include "qkpack_log.h"

using namespace com::youku::data::qkpack::parser;

QkPackParser::QkPackParser() 
{

	qkpack_yajl_json_ = new QkPackYajlJSONImpl;
}

QkPackParser::~QkPackParser() 
{
	if ( qkpack_yajl_json_ ) {
		delete qkpack_yajl_json_;
		qkpack_yajl_json_ = NULL;
	}
}

/**
 * 
 * 解析配置文件
 * 
*/
int QkPackParser::ParseConf(qkpack_conf_t *conf)
{
	return qkpack_yajl_json_->ReadJsonConf(conf);
}


/**
 * 
 * 解析Request Uri
 * 
*/
int QkPackParser::ParseRequestUri(qkpack_request_t *request) 
{
	const std::string &uri = request->request_uri;

	if ( strcasecmp(uri.c_str(), QKPACK_URI_GET) == 0 ) {
		
		request->kvpair.metrics_command = QKPACK_METRICS_GET;
		request->kvpair.command_type = REDIS_GET;
		request->kvpair.operation_type = QKPACK_OPERATION_READ;
	}

	else if ( strcasecmp(uri.c_str(), QKPACK_URI_SET) == 0 ) {
	
		request->kvpair.metrics_command = QKPACK_METRICS_SET;
		request->kvpair.command_type = REDIS_SET;
		request->kvpair.operation_type = QKPACK_OPERATION_WRITE;
	}

	else if ( strcasecmp(uri.c_str(), QKPACK_URI_DEL) == 0 ) {
	
		request->kvpair.metrics_command = QKPACK_METRICS_DEL;
		request->kvpair.command_type = REDIS_DEL;
		request->kvpair.operation_type = QKPACK_OPERATION_WRITE;
	}

	else if ( strcasecmp(uri.c_str(), QKPACK_URI_TTL) == 0  ) {
		
		request->kvpair.metrics_command = QKPACK_METRICS_TTL;
		request->kvpair.command_type = REDIS_TTL;
		request->kvpair.operation_type = QKPACK_OPERATION_READ;
	}
	
	else if ( strcasecmp(uri.c_str(), QKPACK_URI_INCR) == 0  ) {
		
		request->kvpair.metrics_command = QKPACK_METRICS_INCR;
		request->kvpair.command_type = REDIS_INCR;
		request->kvpair.operation_type = QKPACK_OPERATION_WRITE;
	}

	else if ( strcasecmp(uri.c_str(), QKPACK_URI_INCRBY) == 0  ) {

		request->kvpair.metrics_command = QKPACK_METRICS_INCRBY;
		request->kvpair.command_type = REDIS_INCRBY;
		request->kvpair.operation_type = QKPACK_OPERATION_WRITE;
	}
	
	else if ( strcasecmp(uri.c_str(), QKPACK_URI_SET_SADD) == 0 ) {

		request->kvpair.metrics_command =  QKPACK_METRICS_SADD;
		request->kvpair.command_type = REDIS_SADD;
		request->kvpair.operation_type = QKPACK_OPERATION_WRITE;
	}

	else if ( strcasecmp(uri.c_str(), QKPACK_URI_SET_SREM) == 0 ) {

		request->kvpair.metrics_command = QKPACK_METRICS_SREM;
		request->kvpair.command_type = REDIS_SREM;
		request->kvpair.operation_type = QKPACK_OPERATION_WRITE;
	}
	
	else if ( strcasecmp(uri.c_str(), QKPACK_URI_SET_SCARD) == 0 ) {

		request->kvpair.metrics_command = QKPACK_METRICS_SCARD; 
		request->kvpair.command_type = REDIS_SCARD;
		request->kvpair.operation_type = QKPACK_OPERATION_READ;
	}
	
	else if ( strcasecmp(uri.c_str(), QKPACK_URI_SET_SMEMBERS) == 0 ) {

		request->kvpair.metrics_command = QKPACK_METRICS_SMEMBERS;
		request->kvpair.command_type = REDIS_SMEMBERS;
		request->kvpair.operation_type = QKPACK_OPERATION_READ;
	}
	
	else if ( strcasecmp(uri.c_str(), QKPACK_URI_SET_SISMEMBER) == 0 ) {

		request->kvpair.metrics_command = QKPACK_METRICS_SISMEMBER;
		request->kvpair.command_type = REDIS_SISMEMBER;
		request->kvpair.operation_type = QKPACK_OPERATION_READ;
	}
	
	else if ( strcasecmp(uri.c_str(), QKPACK_URI_MGET) == 0 ) {
		
		request->kvpair.metrics_command = QKPACK_METRICS_MGET;
		request->kvpair.command_type = REDIS_MGET;
		request->kvpair.operation_type = QKPACK_OPERATION_READ;
	}
	
	else if ( strcasecmp(uri.c_str(), QKPACK_URI_MSET) == 0 ) {
	
		request->kvpair.metrics_command = QKPACK_METRICS_MSET;
		request->kvpair.command_type = REDIS_MSET;
		request->kvpair.operation_type = QKPACK_OPERATION_WRITE;
	}

	else if ( strcasecmp(uri.c_str(), QKPACK_URI_ZFIXEDSET_ADD) == 0 ) {

		request->kvpair.metrics_command = QKPACK_METRICS_ZFIXEDSETADD;
		request->kvpair.command_type = REDIS_ZADD;
		request->kvpair.operation_type = QKPACK_OPERATION_WRITE;
	}

	else if ( strcasecmp(uri.c_str(), QKPACK_URI_ZFIXEDSET_BATCHADD) == 0 ) {

		request->kvpair.metrics_command = QKPACK_METRICS_ZFIXEDSETBATCHADD;
		request->kvpair.command_type = REDIS_ZBATCHADD;
		request->kvpair.operation_type = QKPACK_OPERATION_WRITE;
	}

	else if ( strcasecmp(uri.c_str(), QKPACK_URI_ZFIXEDSET_GETBYSCORE) == 0 ) {

		request->kvpair.metrics_command = QKPACK_METRICS_ZFIXEDSETGETBYSCORE;
		request->kvpair.command_type = REDIS_ZRANGEBYSCORE;
		request->kvpair.operation_type = QKPACK_OPERATION_READ;
	}

	else if ( strcasecmp(uri.c_str(), QKPACK_URI_ZFIXEDSET_GETBYRANK) == 0 ) {

		request->kvpair.metrics_command = QKPACK_METRICS_ZFIXEDSETGETBYRANK;
		request->kvpair.command_type = REDIS_ZRANGE;
		request->kvpair.operation_type = QKPACK_OPERATION_READ;
	}

	else if ( strcasecmp(uri.c_str(), QKPACK_URI_ZFIXEDSET_BATCHGETBYSCORE) == 0 ) {

		request->kvpair.metrics_command = QKPACK_METRICS_ZFIXEDSETBATCHGETBYSCORE;
		request->kvpair.command_type = REDIS_BATCHZRANGEBYSCORE;
		request->kvpair.operation_type = QKPACK_OPERATION_READ;
	}

	else {
		request->code = QKPACK_RESPONSE_CODE_ILLEGAL_RIGHT;	
		return QKPACK_ERROR;
	}

	return QKPACK_OK;
}


/**
 * 
 * 解析Request Body
 * 
*/
int QkPackParser::ParseRequestBody(qkpack_request_t *request)
{
	int				rc;

	//是否是子请求
	if ( request->is_subrequest ) {
		
		request->cluster_name = request->parent->cluster_name;

		switch( request->parent->kvpair.command_type )
		{
			case REDIS_MGET:
			case REDIS_MSET:
				request->kvpair.metrics_command = request->parent->kvpair.metrics_command;
				request->kvpair.command_type = request->parent->kvpair.command_type;
				request->kvpair.operation_type = request->parent->kvpair.operation_type;
				return QKPACK_OK;
		}

		rc = ParseRequestUri(request); 
		if ( rc != QKPACK_OK ) {
			return QKPACK_ERROR;
		}
		return QKPACK_OK;
	}


	rc = ParseRequestUri(request); 
	if ( rc != QKPACK_OK ) {
		return QKPACK_ERROR;
	}

	switch (request->kvpair.command_type) 
	{
		case REDIS_GET:
		case REDIS_TTL:
		case REDIS_INCR:
		case REDIS_DEL:
		case REDIS_SCARD:
		case REDIS_SMEMBERS:
			return qkpack_yajl_json_->ReadJsonGet(request);
		case REDIS_SET:
		case REDIS_INCRBY:
		case REDIS_SISMEMBER:
			return qkpack_yajl_json_->ReadJsonSet(request);
		case REDIS_SADD:
		case REDIS_SREM:
			return qkpack_yajl_json_->ReadJsonSadd(request);
		case REDIS_MGET:
			return qkpack_yajl_json_->ReadJsonMget(request);
		case REDIS_MSET:
			return qkpack_yajl_json_->ReadJsonMset(request);
		case REDIS_ZADD:
			return qkpack_yajl_json_->ReadJsonZfixedsetAdd(request);
		case REDIS_ZBATCHADD:
			return qkpack_yajl_json_->ReadJsonZfixedsetBatchAdd(request);
		case REDIS_ZRANGEBYSCORE:
		case REDIS_ZRANGE:
			return qkpack_yajl_json_->ReadJsonZfixedsetGetByScore(request);
		case REDIS_BATCHZRANGEBYSCORE:
			return qkpack_yajl_json_->ReadJsonZfixedsetBatchGetByScore(request);
		default:
			return QKPACK_OK;
	}
	
	return QKPACK_OK;
}


/**
 * 
 * 解析Response Body
 * 
*/
int QkPackParser::ParseResponseBody(qkpack_request_t *request)
{
	QKPACK_TIME_MS_END(request);	
	
	if ( request->is_subrequest ) {
		return QKPACK_OK;
	}

	// request解析错误码，直接构建json返回错误信息
	if ( request->code != QKPACK_RESPONSE_CODE_OK ) {
		return qkpack_yajl_json_->WriterJsonMset(request);
	}

	switch (request->kvpair.command_type) 
	{
		case REDIS_GET:
			return qkpack_yajl_json_->WriterJsonGet(request);
		case REDIS_SET:
		case REDIS_TTL:
		case REDIS_DEL:
		case REDIS_INCR:
		case REDIS_INCRBY:
		case REDIS_SADD:
		case REDIS_SREM:
		case REDIS_SCARD:
		case REDIS_SISMEMBER:
			return qkpack_yajl_json_->WriterJsonTtl(request);
		case REDIS_SMEMBERS:
			return qkpack_yajl_json_->WriterJsonSmembers(request);
		default:
			return QKPACK_OK;
	}

	return QKPACK_OK;
}


/**
 * 
 * 解析Response Body Error
 * 
*/
int QkPackParser::ParseResponseError(qkpack_request_t *request)
{
	if ( request->code == QKPACK_RESPONSE_CODE_OK ) {
		request->code = QKPACK_RESPONSE_CODE_UNKNOWN;
	}
	
	QKPACK_TIME_MS_END(request);	
	
	return qkpack_yajl_json_->WriterJsonMset(request);
}


