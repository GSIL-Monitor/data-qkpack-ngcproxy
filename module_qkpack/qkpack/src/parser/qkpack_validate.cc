#include "parser/qkpack_validate.h"

using namespace com::youku::data::qkpack::parser;


/**
 * 
 * 验证配置文件
 * 
*/
int QkPackValidate::ValidateConfig(const Document &d)
{
	if ( d.HasParseError() || !d.IsObject() ) { 
		return QKPACK_ERROR;
    }   
	
	if ( !d.HasMember(QKPACK_CONF_ACL_URL) ||
			!d[QKPACK_CONF_ACL_URL].IsString()) {
		return QKPACK_ERROR;
	}
	
	if ( !d.HasMember(QKPACK_CONF_TIMEOUT) ||
			!d[QKPACK_CONF_TIMEOUT].IsInt()) {
		return QKPACK_ERROR;
	}
	
	if ( !d.HasMember(QKPACK_CONF_ADD_ZSET_SCRIPT) || 
			!d[QKPACK_CONF_ADD_ZSET_SCRIPT].IsString()) {
		return QKPACK_ERROR;
	}
	
	if ( !d.HasMember(QKPACK_CONF_ADD_MULTIKV_SCRIPT) ||
			!d[QKPACK_CONF_ADD_MULTIKV_SCRIPT].IsString()) {
		return QKPACK_ERROR;
	}

	if ( !d.HasMember(QKPACK_CONF_INCRBY_SCRIPT) ||
			!d[QKPACK_CONF_INCRBY_SCRIPT].IsString()) {
		return QKPACK_ERROR;
	}
	
	return QKPACK_OK;
}


/**
 * 
 * 验证ACL响应信息
 * 
*/
int QkPackValidate::ValidateACLResponse(qkpack_request_t *request,
		Document &d)
{
	if ( d.HasParseError() || !d.IsObject() ) { 
		
		return QKPACK_ERROR;
    }
	
	if ( !d.HasMember(QKPACK_ACL_URI_ID) ||
			!d[QKPACK_ACL_URI_ID].IsInt()) {
		
		request->desc = QKPACK_ERROR_ACL_URI_NOT_EMPTY;
		return QKPACK_ERROR;
	}
	
	if ( !d.HasMember(QKPACK_ACL_OPERATION) ||
			!d[QKPACK_ACL_OPERATION].IsInt()) {
		
		return QKPACK_ERROR;
	}
	
	if ( !d.HasMember(QKPACK_ACL_QUOTATION) ||
			!d[QKPACK_ACL_QUOTATION].IsObject()) {
		
		return QKPACK_ERROR;
	}

	rapidjson::Value &quotationObject = d[QKPACK_ACL_QUOTATION];
	if ( !quotationObject.HasMember(QKPACK_ACL_QUOTATION_EXPIRE) ||
			!quotationObject[QKPACK_ACL_QUOTATION_EXPIRE].IsInt()) {
		
		return QKPACK_ERROR;
	}
	
	if ( !quotationObject.HasMember(QKPACK_ACL_QUOTATION_COMMPRESSTHRESHOLD) ||
			!quotationObject[QKPACK_ACL_QUOTATION_COMMPRESSTHRESHOLD].IsInt()) {
		
		request->desc = QKPACK_ERROR_ACL_COMPRESS_THRESHOLD;
		return QKPACK_ERROR;
	}

	
	if ( !d.HasMember(QKPACK_ACL_ROUTE) || !d[QKPACK_ACL_ROUTE].IsObject()) {
	
		request->desc = QKPACK_ERROR_ACL_ROUTE_NOT_BLANK;
		return QKPACK_ERROR;
	}
	
	rapidjson::Value &routeObject = d[QKPACK_ACL_ROUTE];
	if ( !routeObject.HasMember(QKPACK_ACL_ROUTE_DEFAULT) || 
			!routeObject[QKPACK_ACL_ROUTE_DEFAULT].IsString()) {
		
		request->desc = QKPACK_ERROR_ACL_ROUTE_NOT_MATCH;
		return QKPACK_ERROR;
	}
	
	if ( !routeObject.HasMember(QKPACK_ACL_ROUTE_CLIENT) || 
			!routeObject[QKPACK_ACL_ROUTE_CLIENT].IsObject()) {
		
		request->desc = QKPACK_ERROR_ACL_ROUTE_NOT_MATCH;
		return QKPACK_ERROR;
	}
	
	if ( !routeObject.HasMember(QKPACK_ACL_ROUTE_SERVER) || 
			!routeObject[QKPACK_ACL_ROUTE_SERVER].IsObject()) {
		
		request->desc = QKPACK_ERROR_ACL_ROUTE_NOT_MATCH;
		return QKPACK_ERROR;
	}
	
	return QKPACK_OK;
}


/**
 * 
 * 验证ACL信息
 * 
*/
int QkPackValidate::ValidateAclInfo(qkpack_request_t *request,const Document &d) 
{
	if ( d.HasParseError() || !d.IsObject() ) { 
		
		request->desc = d.GetParseError();	
		return QKPACK_ERROR;
    }   
	
	if ( !d.HasMember(QKPACK_JSON_URI) || !d.HasMember(QKPACK_JSON_AK) ) {
		
		request->desc = QKPACK_ERROR_ACL_NOT_NULL;	
		return QKPACK_ERROR;
	}
	
	if ( !d[QKPACK_JSON_URI].IsString() || !d[QKPACK_JSON_URI].GetStringLength() )  {
		
		request->desc = QKPACK_ERROR_ACL_URI_NOT_EMPTY;	
		return QKPACK_ERROR;
	}

	if ( !d[QKPACK_JSON_AK].IsString() || !d[QKPACK_JSON_AK].GetStringLength()  ) {
		
		request->desc = QKPACK_ERROR_ACL_APPKEY_NOT_EMPTY;
		return QKPACK_ERROR;
	}
	
	return QKPACK_OK;
}


/**
 * 
 * 验证KV信息
 * 
*/
int QkPackValidate::ValidateKVPair(qkpack_request_t *request,const rapidjson::Value &d) 
{
	if ( !d.HasMember(QKPACK_JSON_KEY) ) {
	
		request->desc = QKPACK_ERROR_TTL_KEY_NOT_EXIST;
		return QKPACK_ERROR;
	}

	if ( !d[QKPACK_JSON_KEY].IsString() || !d[QKPACK_JSON_KEY].GetStringLength() ) {
	
		request->desc = QKPACK_ERROR_KVPAIR_KEY_NOT_EMPTY;
		return QKPACK_ERROR;
	}

	if ( request->kvpair.command_type == REDIS_MSET ) {
		
		if ( !d.HasMember(QKPACK_JSON_VALUE) || !d[QKPACK_JSON_VALUE].IsString() ||
				!d[QKPACK_JSON_VALUE].GetStringLength() ) {
			
			request->desc = QKPACK_ERROR_KVPAIR_VALUE_NOT_EMPTY;
			return QKPACK_ERROR;
		}
	}


	return QKPACK_OK;
}


/**
 * 
 * 验证KV信息
 * 
*/
int QkPackValidate::ValidateKVPair(qkpack_request_t *request,const Document &d) 
{
	if ( !d.HasMember(QKPACK_JSON_KEY) ) {
	
		request->desc = QKPACK_ERROR_TTL_KEY_NOT_EXIST;
		return QKPACK_ERROR;
	}

	if ( !d[QKPACK_JSON_KEY].IsString() || !d[QKPACK_JSON_KEY].GetStringLength() ) {
	
		request->desc = QKPACK_ERROR_KVPAIR_KEY_NOT_EMPTY;
		return QKPACK_ERROR;
	}

	switch( request->kvpair.command_type )
	{
		case REDIS_SET:
		case REDIS_INCRBY:
		case REDIS_SISMEMBER:
			if ( !d.HasMember(QKPACK_JSON_VALUE) || !d[QKPACK_JSON_VALUE].IsString() ||
					!d[QKPACK_JSON_VALUE].GetStringLength() ) {
				
				request->desc = QKPACK_ERROR_KVPAIR_VALUE_NOT_EMPTY;
				return QKPACK_ERROR;
			}
	}

	return QKPACK_OK;
}


/**
 * 
 * 验证Multi KV信息
 * 
*/
int QkPackValidate::ValidateMultiKVPair(qkpack_request_t *request,const Document &d)
{
	if ( !d.HasMember(QKPACK_JSON_KEYS) || !d[QKPACK_JSON_KEYS].IsArray() ) {
		
		request->desc = QKPACK_ERROR_KVPAIR_KEYS_NOT_EMPTY;		
		return QKPACK_ERROR;
	}

	return QKPACK_OK;
}


/**
 * 
 * 验证Zset
 * 
*/
int QkPackValidate::ValidateZset(qkpack_request_t *request,const Document &d)
{
	if ( !d.HasMember(QKPACK_JSON_KEY) ) {
	
		request->desc = QKPACK_ERROR_TTL_KEY_NOT_EXIST;
		return QKPACK_ERROR;
	}

	if ( !d[QKPACK_JSON_KEY].IsString() || !d[QKPACK_JSON_KEY].GetStringLength() ) {
	
		request->desc = QKPACK_ERROR_ZSET_KEY_NOT_BLANK;
		return QKPACK_ERROR;
	}
	
	if ( !d.HasMember(QKPACK_JSON_MBS) || !d[QKPACK_JSON_MBS].IsArray()) {
		
		request->desc = QKPACK_ERROR_SET_MEMBERS_NOT_EMPTY;
		return QKPACK_ERROR;
	}
	
	return QKPACK_OK;
}


/**
 * 
 * 验证Zset
 * 
*/
int QkPackValidate::ValidateZset(qkpack_request_t *request,const rapidjson::Value &d)
{
	if ( !d.HasMember(QKPACK_JSON_KEY) ) {
	
		request->desc = QKPACK_ERROR_TTL_KEY_NOT_EXIST;
		return QKPACK_ERROR;
	}

	if ( !d[QKPACK_JSON_KEY].IsString() || !d[QKPACK_JSON_KEY].GetStringLength() ) {
	
		request->desc = QKPACK_ERROR_ZSET_KEY_NOT_BLANK;
		return QKPACK_ERROR;
	}
	
	if ( !d.HasMember(QKPACK_JSON_MBS) || !d[QKPACK_JSON_MBS].IsArray()) {
		
		request->desc = QKPACK_ERROR_SET_MEMBERS_NOT_EMPTY;
		return QKPACK_ERROR;
	}
	
	return QKPACK_OK;
}


/**
 * 
 * 验证ZsetQuery
 * 
*/
int QkPackValidate::ValidateZsetQuery(qkpack_request_t *request,const Document &d)
{
	if ( !d.HasMember(QKPACK_JSON_MIN) ) {

		request->desc = QKPACK_ERROR_ZSET_QUERY_MIN_NOT_NULL;	
		return QKPACK_ERROR;
	}
	
	if ( !d.HasMember(QKPACK_JSON_MAX) ) {
		
		request->desc = QKPACK_ERROR_ZSET_QUERY_MAX_NOT_NULL;
		return QKPACK_ERROR;
	}

	if ( !d[QKPACK_JSON_MIN].IsInt() || !d[QKPACK_JSON_MAX].IsInt() ) {
		
		request->desc = QKPACK_ERROR_ZSET_QUERY_NUMERIC_VALUE;
		return QKPACK_ERROR;
	}

	if ( !d.HasMember(QKPACK_JSON_KEY) || !d[QKPACK_JSON_KEY].IsString() ||
			!d[QKPACK_JSON_KEY].GetStringLength() ) {
		
		request->desc = QKPACK_ERROR_ZSET_QUERY_KEY_NOT_BLANK;
		return QKPACK_ERROR;
	}

	return QKPACK_OK;
}


/**
 * 
 * 验证ZsetQuery
 * 
*/
int QkPackValidate::ValidateZsetQuery(qkpack_request_t *request,const rapidjson::Value &d)
{
	if ( !d.HasMember(QKPACK_JSON_MIN) ) {

		request->desc = QKPACK_ERROR_ZSET_QUERY_MIN_NOT_NULL;	
		return QKPACK_ERROR;
	}
	
	if ( !d.HasMember(QKPACK_JSON_MAX) ) {
		
		request->desc = QKPACK_ERROR_ZSET_QUERY_MAX_NOT_NULL;
		return QKPACK_ERROR;
	}

	if ( !d[QKPACK_JSON_MIN].IsInt() || !d[QKPACK_JSON_MAX].IsInt() ) {
		
		request->desc = QKPACK_ERROR_ZSET_QUERY_NUMERIC_VALUE;
		return QKPACK_ERROR;
	}

	if ( !d.HasMember(QKPACK_JSON_KEY) || !d[QKPACK_JSON_KEY].IsString() ||
			!d[QKPACK_JSON_KEY].GetStringLength() ) {
		
		request->desc = QKPACK_ERROR_ZSET_QUERY_KEY_NOT_BLANK;
		return QKPACK_ERROR;
	}

	return QKPACK_OK;
}


/**
 * 
 * 验证ZsetMember
 * 
*/
int QkPackValidate::ValidateZsetMember(qkpack_request_t *request,const rapidjson::Value &val)
{
	if ( !val.HasMember(QKPACK_JSON_MB) ||  !val[QKPACK_JSON_MB].IsString() ) {
		
		request->desc = QKPACK_ERROR_ZSET_MEMBER_NOT_BLANK;
		return QKPACK_ERROR;
	}
			
	if ( !val.HasMember(QKPACK_JSON_SC) ||  !val[QKPACK_JSON_SC].IsInt() ) {
	
		request->desc = QKPACK_ERROR_ZSET_MEMBER_SCORE_NOT_NULL;
		return QKPACK_ERROR;
	}
			
	if ( !val.HasMember(QKPACK_JSON_V) ||  !val[QKPACK_JSON_V].IsString() ) {
		
		request->desc = QKPACK_ERROR_ZSET_MEMBER_VALUE_NOT_BLANK;
		return QKPACK_ERROR;
	}
			
	return QKPACK_OK;
}


/**
 * 
 * 
 * 
*/
int QkPackValidate::ValidateResponse(qkpack_request_t *request,const Document &d)
{
	if ( d.HasParseError() || !d.IsObject() ) { 
		//request->desc = ;
		return QKPACK_ERROR;
    }   
	
	if ( !d.HasMember(QKPACK_JSON_E) || !d[QKPACK_JSON_E].IsObject()) {
		//request->desc ;	
		return QKPACK_ERROR;
	}

	if ( !d.HasMember(QKPACK_JSON_COST) || !d[QKPACK_JSON_COST].IsInt()) {
		//request->desc = ;
		return QKPACK_ERROR;
	}  
	
	if ( !d[QKPACK_JSON_E].HasMember(QKPACK_JSON_CODE) || !d[QKPACK_JSON_E][QKPACK_JSON_CODE].IsInt()) {
		return QKPACK_ERROR;
	}

	if ( !d[QKPACK_JSON_E].HasMember(QKPACK_JSON_DESC) || !d[QKPACK_JSON_E][QKPACK_JSON_DESC].IsString()) {
		return QKPACK_ERROR;
	}

	return QKPACK_OK;
}


