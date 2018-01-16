#ifndef __QKPACK_PARSER_H__
#define __QKPACK_PARSER_H__

#include "common.h"
#include "parser/qkpack_yajl_json_impl.h"

namespace com { 
namespace youku { 
namespace data {
namespace qkpack {
namespace parser {


class QkPackParser
{
public:
    QkPackParser();
	virtual ~QkPackParser();

public:

	/**
	 * 
	 * 解析配置文件
	 * 
	*/
	int ParseConf(qkpack_conf_t *conf);
	
	
	/**
	 * 
	 * 解析Request Uri
	 * 
	*/
	int ParseRequestUri(qkpack_request_t *request);
	
	
	/**
	 * 
	 * 解析Request Body
	 * 
	*/
	int ParseRequestBody(qkpack_request_t *request);
	
	
	/**
	 * 
	 * 解析Response Body
	 * 
	*/
	int ParseResponseBody(qkpack_request_t *request);
	
	
	/**
	 * 
	 * 解析Response Body Error
	 * 
	*/
	int ParseResponseError(qkpack_request_t *request);

protected:

	QkPackYajlJSONImpl *qkpack_yajl_json_;
};

} /* namespace parser */
} /* namespace qkpack */
} /* namespace data */
} /* namespace youku */
} /* namespace com */

#endif /* __QKPACK_PARSER_H__ */
