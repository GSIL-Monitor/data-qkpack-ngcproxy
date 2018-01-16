#ifndef __QKPACK_JSON_IMPL_H__
#define __QKPACK_JSON_IMPL_H__

#include "parser/qkpack_yajl_json.h"

namespace com { 
namespace youku { 
namespace data {
namespace qkpack {
namespace parser {


class QkPackYajlJSONImpl : public QkPackYajlJSON
{

public:
    QkPackYajlJSONImpl(){};
	virtual ~QkPackYajlJSONImpl(){};

public:
	
	/**
	 * 
	 * 解析Request Body Json(ZADD)
	 * 
	*/
	int ReadJsonZfixedsetAdd(qkpack_request_t *request);
	
	
	/**
	 * 
	 * 构建SubRequest Body Json(ZADD)
	 * 
	*/
	int WriterSubRequestJsonZfixedsetAdd(qkpack_request_t *request);
	
	
	/**
	 * 
	 * 解析Request Body Json(ZADD Batch)
	 * 
	*/
	int ReadJsonZfixedsetBatchAdd(qkpack_request_t *request);
	
	
	/**
	 * 
	 * 解析Request Body Json(ZRANGEBYSCORE,ZREVRANGEBYSCORE)
	 * 
	*/
	int ReadJsonZfixedsetGetByScore(qkpack_request_t *request);

	
	/**
	 * 
	 * 构建Response Body Json(ZRANGEBYSCORE,ZREVRANGEBYSCORE)
	 * 
	*/
	int WriterJsonZfixedsetGetByScore(qkpack_request_t *request);
	
	
	/**
	 * 
	 * 解析Response Body Json(ZRANGEBYSCORE,ZREVRANGEBYSCORE)
	 * 
	*/
	int ReadResponseJsonZfixedsetGetByScore(qkpack_request_t *request);
	
	
	/**
	 * 
	 * 解析Request Body Json Batch(ZRANGEBYSCORE,ZREVRANGEBYSCORE)
	 * 
	*/
	int ReadJsonZfixedsetBatchGetByScore(qkpack_request_t *request);
	
	
	/**
	 * 
	 * 构建SubRequest Body Json(ZRANGEBYSCORE,ZREVRANGEBYSCORE)
	 * 
	*/
	int WriterSubRequestJsonZfixedsetGetByScore(qkpack_request_t *request);
	
	
	/**
	 * 
	 * 构建Response Body Json Batch(ZRANGEBYSCORE,ZREVRANGEBYSCORE)
	 * 
	*/
	int WriterJsonZfixedsetBatchGetByScore(qkpack_request_t *request);
};

} /* namespace parser */
} /* namespace qkpack */
} /* namespace data */
} /* namespace youku */
} /* namespace com */


#endif //__QKPACK_JSON_IMPL_H__
