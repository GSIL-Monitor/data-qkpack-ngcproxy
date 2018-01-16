#ifndef __QKPACK_VALIDATE_H__
#define __QKPACK_VALIDATE_H__

#include "common.h"
#include "qkpack_common.h"


#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/reader.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "rapidjson/internal/pow10.h"
#include "rapidjson/internal/stack.h"
#include "rapidjson/internal/strfunc.h"

using namespace rapidjson;

namespace com { 
namespace youku { 
namespace data {
namespace qkpack {
namespace parser {


class QkPackValidate
{

public:
    QkPackValidate(){};
	virtual ~QkPackValidate(){};
	
public:
	
	int ValidateConfig(const Document &d);
	
	int ValidateACLResponse(qkpack_request_t *request, Document &d);

	int ValidateAclInfo(qkpack_request_t *request,const Document &d);
	
	int ValidateKVPair(qkpack_request_t *request,const rapidjson::Value &d);
	
	int ValidateKVPair(qkpack_request_t *request,const Document &d);
	
	int ValidateMultiKVPair(qkpack_request_t *request,const Document &d);
	
	int ValidateZset(qkpack_request_t *request,const Document &d);
	
	int ValidateZset(qkpack_request_t *request,const rapidjson::Value &d);
	
	int ValidateZsetQuery(qkpack_request_t *request,const Document &d);
	
	int ValidateZsetQuery(qkpack_request_t *request,const rapidjson::Value &d);
	
	int ValidateZsetMember(qkpack_request_t *request,const rapidjson::Value &d);
	
	int ValidateResponse(qkpack_request_t *request,const Document &d);
};


} /* namespace parser */
} /* namespace qkpack */
} /* namespace data */
} /* namespace youku */
} /* namespace com */

#endif //__QKPACK_VALIDATE_H__

