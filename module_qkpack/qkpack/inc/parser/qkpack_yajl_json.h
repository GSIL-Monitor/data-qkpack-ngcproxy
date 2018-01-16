#ifndef __QKPACK_YAJL_JSON_H__
#define __QKPACK_YAJL_JSON_H__

#include "common.h"
#include "qkpack_common.h"
#include "parser/qkpack_yajl_validate.h"

extern "C" {
#include <yajl/yajl_parse.h>
#include <yajl/yajl_gen.h>
#include <yajl/yajl_tree.h>
}


namespace com { 
namespace youku { 
namespace data {
namespace qkpack {
namespace parser {


class QkPackYajlJSON
{

public:
    QkPackYajlJSON();
	virtual ~QkPackYajlJSON();
	
public:
	/**
	* 
	* Parse global config file
	* 
	*/
	int ReadJsonConf(qkpack_conf_t  *conf);
	
	

	int ReadACLQuotation(qkpack_acl_t *acl, yajl_val &quotation);
	
	/**
	 * 
	 * Parse ACL Response JSON
	 * 
	*/
	int ReadACLResponse(qkpack_acl_t *acl,const std::string &response_buffer);
	
	/**
	* 
	* Create Interface Response Body Json
	* 
	*/
	void WriterJsonCommon(qkpack_request_t *request, yajl_gen  &g);
	
	/**
	 * 
	 * Parse Interface Request Body Json(GET,TTL,INCR,DEL,SCARD,SMEMBERS)
	 * 
	*/
	int ReadJsonGet(qkpack_request_t *request);
	
	/**
	* 
	* Create Interface Response Body Json(GET)
	* 
	*/
	int WriterJsonGet(qkpack_request_t *request);
	
	/**
	* 
	* Create Interface Response Body Json(SET,TTL,DEL,INCR,INCRBY,SADD,SREM,SCARD,SISMEMBER)
	* 
	*/
	int WriterJsonTtl(qkpack_request_t *request);

	/**
	* 
	* Parse Interface Request Body Json(SET,INCRBY,SISMEMBER)
	* 
	*/
	int ReadJsonSet(qkpack_request_t *request);
	
	/**
	* 
	* Parse Interface Request Body Json(SADD)
	* 
	*/
	int ReadJsonSadd(qkpack_request_t *request);
	
	/**
	* 
	* Create Interface Response Body Json(SMEMBERS)
	* 
	*/
	int WriterJsonSmembers(qkpack_request_t *request);

	/**
	* 
	* Parse Interface Request Body Json(MGET)
	* 
	*/
	int ReadJsonMget(qkpack_request_t *request);
	
	/**
	* 
	* Create Interface Response Body Json(MGET) 已经根据slotid划分好的key集合
	* 
	*/
	int WriterJsonMget(qkpack_request_t *request); 

	/**
	* 
	* Create Interface SubRequest Body Json(MGET), 已经根据slotid划分好的key集合
	* 
	*/
	int WriterSubRequestJsonMget(qkpack_request_t *request,std::vector<kvpair_t> vector_kvpair); 
	
	/**
	* 
	* Parse Interface SubResponse Body Json(MGET)
	* 
	*/
	int ReadSubResponseJsonMget(qkpack_request_t *request);
	
	/**
	* 
	* Create Interface SubResponse Body Json(MGET)
	* 
	*/
	int WriterSubResponseJsonMget(qkpack_request_t *request); 
	
	/**
	* 
	* Parse Interface Request Body Json(MSET)
	* 
	*/
	int ReadJsonMset(qkpack_request_t *request);
	
	/**
	* 
	* Create Interface Response Body Json(MSET)
	* 
	*/
	int WriterJsonMset(qkpack_request_t *request);

	/**
	* 
	* Create Interface SubRequest Body Json(MSET) 经根据slotid划分好的key集合
	* 
	*/
	int WriterSubRequestJsonMset(qkpack_request_t *request, std::vector<kvpair_t> vector_kvpair);
	
	/**
	* 
	* Parse Interface SubResponse Body Json(MSET)
	* 
	*/
	int ReadSubResponseJsonMset(qkpack_request_t *request);

protected:
	QkPackYajlValidate *qkpack_yajl_validate_;
		
};

} /* namespace parser */
} /* namespace qkpack */
} /* namespace data */
} /* namespace youku */
} /* namespace com */


#endif //__QKPACK_YAJL_JSON_H__

