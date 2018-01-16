#include "parser/qkpack_yajl_json.h"
#include "util/string_util.h"
#include "core/redis_slot.h"

extern "C" {
#include <yajl/yajl_parse.h>
#include <yajl/yajl_gen.h>
#include <yajl/yajl_tree.h>
}

using namespace com::youku::data::qkpack::parser;
using namespace com::youku::data::qkpack::util;


QkPackYajlJSON::QkPackYajlJSON() 
{
	qkpack_yajl_validate_ = new QkPackYajlValidate();
}


QkPackYajlJSON::~QkPackYajlJSON() 
{
	if ( qkpack_yajl_validate_ ) {
		delete qkpack_yajl_validate_;
		qkpack_yajl_validate_ = NULL;
	}
}

/**
 * 
 * Parse global config file
 * 
*/
int QkPackYajlJSON::ReadJsonConf(qkpack_conf_t  *conf)
{
	/*
	{
		"aclUrl" :  "http://10.100.14.152:8080/data-auth-http-server/httpServer/authority",
		"timeout" : 1000,
		"addZSetScript" : "",
		"addMultiKVScript" : "",
		"incrByScript" : "",
		"addRSetScript" : ""
	}
	*/
	yajl_val					node;

	node = yajl_tree_parse(conf->conf_str.c_str(), NULL, 0);
	if ( node == NULL ) {
		return QKPACK_ERROR;
	}
	
	yajl_val acl_url = yajl_tree_get(node, (const char *[]){QKPACK_CONF_ACL_URL, 0}, yajl_t_string);
	yajl_val timeout = yajl_tree_get(node, (const char *[]){QKPACK_CONF_TIMEOUT, 0}, yajl_t_number);
	yajl_val add_zset_script = yajl_tree_get(node, (const char *[]){QKPACK_CONF_ADD_ZSET_SCRIPT, 0}, yajl_t_string);
	yajl_val add_multikv_script = yajl_tree_get(node, (const char *[]){QKPACK_CONF_ADD_MULTIKV_SCRIPT, 0}, yajl_t_string);
	yajl_val incrby_script = yajl_tree_get(node, (const char *[]){QKPACK_CONF_INCRBY_SCRIPT, 0}, yajl_t_string);
	yajl_val add_rset_script = yajl_tree_get(node, (const char *[]){QKPACK_CONF_ADD_RSET_SCRIPT, 0}, yajl_t_string);
	
	if ( !acl_url || !timeout || !add_zset_script || !add_multikv_script || !incrby_script || !add_rset_script ) {
		yajl_tree_free(node);
		return QKPACK_ERROR;
	}

	conf->acl_url = std::string(YAJL_GET_STRING(acl_url));
	conf->timeout = YAJL_GET_INTEGER(timeout);
	conf->add_zset_script = std::string(YAJL_GET_STRING(add_zset_script));
	conf->add_multikv_script = std::string(YAJL_GET_STRING(add_multikv_script));
	conf->incrby_script = std::string(YAJL_GET_STRING(incrby_script));
	conf->add_rset_script = std::string(YAJL_GET_STRING(add_rset_script));

	yajl_tree_free(node);
	return QKPACK_OK;
}


int QkPackYajlJSON::ReadACLQuotation(qkpack_acl_t *acl, yajl_val &quotation)
{
	yajl_val					node;

	if ( YAJL_IS_OBJECT(quotation) ) {
		node = quotation;
	}
	else if ( YAJL_IS_STRING(quotation) ) { 
		node = yajl_tree_parse(YAJL_GET_STRING(quotation), NULL, 0);
		if ( node == NULL ) {
			return QKPACK_ERROR;
		}
	}
	else {
		return QKPACK_ERROR;
	}
	
	yajl_val expire = yajl_tree_get(node, (const char *[]){QKPACK_ACL_QUOTATION_EXPIRE, 0}, yajl_t_number);
	yajl_val compress_threshold = yajl_tree_get(node, (const char *[]){QKPACK_ACL_QUOTATION_COMMPRESSTHRESHOLD, 0}, yajl_t_number);

	if ( !expire || !compress_threshold ) {
		yajl_tree_free(node);
		return QKPACK_ERROR;
	}

	acl->quotation.expire = YAJL_GET_INTEGER(expire);
	acl->quotation.compress_threshold = YAJL_GET_INTEGER(compress_threshold);

	yajl_val limitPoliciesArray = yajl_tree_get(node, (const char *[]){QKPACK_ACL_QUOTATION_LIMITPOLICIES, 0}, yajl_t_array);
	if ( limitPoliciesArray && YAJL_IS_ARRAY(limitPoliciesArray) ) {
		int len = YAJL_GET_ARRAY(limitPoliciesArray)->len;
		
		for (int i = 0; i < len; i++) {
			yajl_val val = YAJL_GET_ARRAY(limitPoliciesArray)->values[i];
			yajl_val min = yajl_tree_get(val, (const char *[]){QKPACK_ACL_QUOTATION_LIMITPOLICIES_MIN, 0}, yajl_t_number);
			yajl_val max = yajl_tree_get(val, (const char *[]){QKPACK_ACL_QUOTATION_LIMITPOLICIES_MAX, 0}, yajl_t_number);
			yajl_val limit = yajl_tree_get(val, (const char *[]){QKPACK_ACL_QUOTATION_LIMITPOLICIES_LIMIT, 0}, yajl_t_number);
			
			if ( !min || !max || !limit ) {
				yajl_tree_free(node);
				return QKPACK_ERROR;
			}
		
			limit_policies_t limit_policies;
			limit_policies.min = YAJL_GET_INTEGER(min);
			limit_policies.max = YAJL_GET_INTEGER(max);
			limit_policies.limit = YAJL_GET_INTEGER(limit);
			
			acl->quotation.vector_policies.push_back(limit_policies);
		}  
	}
	
	yajl_val timeoutsArray = yajl_tree_get(node, (const char *[]){QKPACK_ACL_QUOTATION_TIMEOUTS, 0}, yajl_t_array);
	if ( timeoutsArray && YAJL_IS_ARRAY(timeoutsArray) ) {
		int len = YAJL_GET_ARRAY(timeoutsArray)->len;

		for (int i = 0; i < len; i++) {
			yajl_val val = YAJL_GET_ARRAY(timeoutsArray)->values[i];
			yajl_val interface_name = yajl_tree_get(val, 
					(const char *[]){QKPACK_ACL_QUOTATION_TIMEOUTS_INTERFACENAME, 0}, yajl_t_string);
			yajl_val timeout = yajl_tree_get(val, (const char *[]){QKPACK_ACL_QUOTATION_TIMEOUTS_TIMEOUT, 0}, yajl_t_number);
			yajl_val tries = yajl_tree_get(val, (const char *[]){QKPACK_ACL_QUOTATION_TIMEOUTS_TRIES, 0}, yajl_t_number);
			
			if ( !interface_name || !timeout || !tries ) {
				yajl_tree_free(node);
				return QKPACK_ERROR;
			}
			
			timeouts_t timeouts;
			timeouts.interface_name = std::string(YAJL_GET_STRING(interface_name));
			timeouts.timeout = YAJL_GET_INTEGER(timeout);
			timeouts.tries = YAJL_GET_INTEGER(tries);
			
			acl->quotation.vector_timeouts.push_back(timeouts);
		}
	}

	yajl_val limitsLengthArray = yajl_tree_get(node, (const char *[]){QKPACK_ACL_QUOTATION_LIMITSLENGTH, 0}, yajl_t_array);
	if ( limitsLengthArray && YAJL_IS_ARRAY(limitsLengthArray) ) {
		int len = YAJL_GET_ARRAY(limitsLengthArray)->len;

		for (int i = 0; i < len; i++) {
			yajl_val val = YAJL_GET_ARRAY(limitsLengthArray)->values[i];
			yajl_val interface_name = yajl_tree_get(val, 
					(const char *[]){QKPACK_ACL_QUOTATION_LIMITSLENGTH_INTERFACENAME, 0}, yajl_t_string);
			yajl_val keylen = yajl_tree_get(val, (const char *[]){QKPACK_ACL_QUOTATION_LIMITSLENGTH_KEYLEN, 0}, yajl_t_number);
			yajl_val vallen = yajl_tree_get(val, (const char *[]){QKPACK_ACL_QUOTATION_LIMITSLENGTH_VALLEN, 0}, yajl_t_number);
			yajl_val datalen = yajl_tree_get(val, (const char *[]){QKPACK_ACL_QUOTATION_LIMITSLENGTH_DATALEN, 0}, yajl_t_number);
			
			if ( !interface_name || !keylen || !vallen || !datalen ) {
				yajl_tree_free(node);
				return QKPACK_ERROR;
			}
			
			limits_length_t limits_len;
			limits_len.interface_name = std::string(YAJL_GET_STRING(interface_name));
			limits_len.keylen = YAJL_GET_INTEGER(keylen);
			limits_len.vallen = YAJL_GET_INTEGER(vallen);
			limits_len.datalen = YAJL_GET_INTEGER(datalen);
			
			acl->quotation.vector_limits_length.push_back(limits_len);
		}
	}

	if ( YAJL_IS_STRING(quotation) ) { 
		yajl_tree_free(node);
	}
	return QKPACK_OK;
}





/**
 * 
 * Parse ACL Response JSON
 * 
*/
int QkPackYajlJSON::ReadACLResponse(qkpack_acl_t *acl,const std::string &response_buffer)
{
	/*
	{
	  "uri_id": 1453,     // 数据库中存储的uri主键，唯一标识一个uri资源
	  "operation": 3,    // 1: 读 2:写 3:读写
	  "quotation": {       // 配额配置，此段落可以灵活配置和扩展
		"expire": 86400,      // 针对某个uri+appkey组合的key过期时间，单位秒
		"compressThreshold": 1,         // 大于等于多少byte则启用压缩算法
		"limitPolicies": [       // 针对zfixedset的最大长度的限制策略。min<=keylen<=max，则zfixedset的成员数上线<=limit
		  {
			"min": 0,
			"max": 11,
			"limit": 1000
		  },
		  {
			"min": 12,
			"max": 20,
			"limit": 30
		  }
		]
		"timeouts" :
	  	[
	  	  {
	  	  	"interfaceName": "kv/get",
	  	  	"timeout" : 10 ,
	  	  	"tries" : 5	
	  	  },
	  	  {
	  	  	"interfaceName": "set/sadd",
	  	  	"timeout" : 10 ,
	  	  	"tries" : 5	
	  	  },
	  	  {
	  	  	"interfaceName": "zfixedset/add",
	  	  	"timeout" : 30 ,
	  	  	"tries" : 5	
	  	  }
	  	]
	  
	  },
	  "route": {  // 集群路由配置，包括网段限制策略和redis cluster seeds
		"default": "10.100.23.78:17880,10.100.23.79:17980,10.100.23.80:18080",        // 当client网段没有命中server时，使用的默认路由策略
		"client": { // client网段对应的集群唯一标识，比如client是10.100网段的，对应"BJ02"
		  "10.100": "BJ02"
		},
		"server": {        // redis集群唯一标识对应的集群seeds
		  "BJ02": "10.100.23.78:17880,10.100.23.79:17980,10.100.23.80:18080"
		}
	  }
	}
	*/
	int							rc;
	yajl_val					node;

	node = yajl_tree_parse(response_buffer.c_str(), NULL, 0);
	if ( node == NULL ) {
		return QKPACK_ERROR;
	}
	
	yajl_val uri_id = yajl_tree_get(node, (const char *[]){QKPACK_ACL_URI_ID, 0}, yajl_t_number);
	yajl_val operation = yajl_tree_get(node, (const char *[]){QKPACK_ACL_OPERATION, 0}, yajl_t_number);
	yajl_val quotation = yajl_tree_get(node, (const char *[]){QKPACK_ACL_QUOTATION, 0}, yajl_t_string);
	
	if ( !uri_id || !operation ) {
		yajl_tree_free(node);
		return QKPACK_ERROR;
	}
	
	acl->uri_id = StringUtil::ToString(YAJL_GET_INTEGER(uri_id));
	acl->operation = YAJL_GET_INTEGER(operation);
	
	if ( !quotation ) {
		quotation = yajl_tree_get(node, (const char *[]){QKPACK_ACL_QUOTATION, 0}, yajl_t_object);
	}

	rc = ReadACLQuotation(acl, quotation);
	if ( rc != QKPACK_OK ) {
		yajl_tree_free(node);
		return QKPACK_ERROR;
	}
	
	yajl_val routeObject = yajl_tree_get(node, (const char *[]){QKPACK_ACL_ROUTE, 0}, yajl_t_object);
	yajl_val defaults = yajl_tree_get(routeObject, (const char *[]){QKPACK_ACL_ROUTE_DEFAULT, 0}, yajl_t_string);
	acl->route.defaults = std::string(YAJL_GET_STRING(defaults));
	
	yajl_val clientObject = yajl_tree_get(routeObject, (const char *[]){QKPACK_ACL_ROUTE_CLIENT, 0}, yajl_t_object);
	if ( !clientObject ) {
		yajl_tree_free(node);
		return QKPACK_ERROR;
	}
	
	int len = YAJL_GET_OBJECT(clientObject)->len;
	for (int i = 0; i < len; i++) {
		const char *key = YAJL_GET_OBJECT(clientObject)->keys[i];
		yajl_val   val = YAJL_GET_OBJECT(clientObject)->values[i];
		
		if ( !key || !val ) {
			yajl_tree_free(node);
			return QKPACK_ERROR;
		}
		
		acl->route.map_client[std::string(key)]= std::string(YAJL_GET_STRING(val));
	}
	
	yajl_val serverObject = yajl_tree_get(routeObject, (const char *[]){QKPACK_ACL_ROUTE_SERVER, 0}, yajl_t_object);
	if ( !serverObject ) {
		yajl_tree_free(node);
		return QKPACK_ERROR;
	}
	
	len = YAJL_GET_OBJECT(serverObject)->len;
	for (int i = 0; i < len; i++) {
		const char *key = YAJL_GET_OBJECT(serverObject)->keys[i];
		yajl_val   val = YAJL_GET_OBJECT(serverObject)->values[i];
		
		if ( !key || !val ) {
			yajl_tree_free(node);
			return QKPACK_ERROR;
		}
		
		acl->route.map_server[std::string(key)]= std::string(YAJL_GET_STRING(val));
	}

	yajl_tree_free(node);
	return QKPACK_OK;
}


/**
 * 
 * Parse Interface Request Body Json(GET,TTL,INCR,DEL,SCARD,SMEMBERS)
 * 
*/
int QkPackYajlJSON::ReadJsonGet(qkpack_request_t *request)
{
	/*
	{
		"uri" : "qkd://BJTEST/Bd",
		"ak" : "51v6sHF9Mk",
		"k" : "qkdtestkey"
	}
	*/
	int							rc;
	yajl_val					node;

	node = yajl_tree_parse(request->request_buffer.c_str(), NULL, 0);
	if ( node == NULL ) {
		request->desc =  QKPACK_ERROR_JSON_FORMAT; 
		return QKPACK_ERROR;
	}
	
	yajl_val uri = yajl_tree_get(node, (const char *[]){QKPACK_JSON_URI, 0}, yajl_t_string);
	yajl_val ak = yajl_tree_get(node, (const char *[]){QKPACK_JSON_AK, 0}, yajl_t_string);
	yajl_val key = yajl_tree_get(node, (const char *[]){QKPACK_JSON_KEY, 0}, yajl_t_string);
	
	//acl validate exits and not empty
	rc = qkpack_yajl_validate_->ValidateACLInfo(node, uri, ak , request->desc);
	if ( rc !=  QKPACK_OK ) {
		yajl_tree_free(node);
		return QKPACK_ERROR;
	}
	
	if ( request->kvpair.command_type == REDIS_SCARD ||
			request->kvpair.command_type == REDIS_SMEMBERS ) {
	
		//set key validate exits and not empty
		rc = qkpack_yajl_validate_->ValidateSetByKey(key , request->desc);
		if ( rc !=  QKPACK_OK ) {
			yajl_tree_free(node);
			return QKPACK_ERROR;
		}

	}else {
	
		//kvpair key validate exits and not empty
		rc = qkpack_yajl_validate_->ValidateKVPairByKey(key , request->desc);
		if ( rc !=  QKPACK_OK ) {
			yajl_tree_free(node);
			return QKPACK_ERROR;
		}
	}	

	request->uri = std::string(YAJL_GET_STRING(uri));
	request->ak = std::string(YAJL_GET_STRING(ak));
	request->kvpair.key = std::string(YAJL_GET_STRING(key));
	
	yajl_tree_free(node);
	return QKPACK_OK;
}

/**
 * 
 * Create Interface Response Body Json
 * 
*/
void QkPackYajlJSON::WriterJsonCommon(qkpack_request_t *request, yajl_gen  &g)
{
	yajl_gen_string(g, (const unsigned char *) QKPACK_JSON_E, strlen(QKPACK_JSON_E));
	
	yajl_gen_map_open(g);
	yajl_gen_string(g, (const unsigned char *) QKPACK_JSON_CODE, strlen(QKPACK_JSON_CODE));
	yajl_gen_integer(g, request->code);
	
	yajl_gen_string(g, (const unsigned char *) QKPACK_JSON_DESC, strlen(QKPACK_JSON_DESC));
	yajl_gen_string(g, (const unsigned char *) request->desc.c_str(), request->desc.length());
	yajl_gen_map_close(g);
	
	
	yajl_gen_string(g, (const unsigned char *) QKPACK_JSON_COST, strlen(QKPACK_JSON_COST));
	yajl_gen_integer(g, request->cost);	
}



/**
 * 
 * Create Interface Response Body Json(GET)
 * 
*/
int QkPackYajlJSON::WriterJsonGet(qkpack_request_t *request) 
{
	/*
	{
		"e":{
			"code":0,    // 错误码，程序可以通过此错误码做不同的错误处理，或便于联调及快速定位问题
			"desc":""    // 针对错误码的描述，方便快速定位问题
		},
		"cost":2,  // 服务的处理耗时，单位毫秒
		"k" : "qkdtestkey",
		"v" : null  // 返回key所关联的字符串值，如果key不存在则会返回null
	}
	*/
	yajl_gen                  g;
	size_t 					  len;
	const unsigned char 	  *buf;
    
	g = yajl_gen_alloc(NULL);
	if (g == NULL) {
        return QKPACK_ERROR;
    }
	yajl_gen_config(g, yajl_gen_beautify, 1);
	
	yajl_gen_map_open(g);
	
	WriterJsonCommon(request,g);
	
	yajl_gen_string(g, (const unsigned char *) QKPACK_JSON_KEY, strlen(QKPACK_JSON_KEY));
	yajl_gen_string(g, (const unsigned char *) request->kvpair.key.c_str(), request->kvpair.key.length());
	
	yajl_gen_string(g, (const unsigned char *) QKPACK_JSON_VALUE, strlen(QKPACK_JSON_VALUE));
	yajl_gen_string(g, (const unsigned char *) request->kvpair.value.c_str(), request->kvpair.value.length());
	
	yajl_gen_map_close(g);
	
	yajl_gen_status status = yajl_gen_get_buf(g, &buf, &len);
	if(status != yajl_gen_status_ok) {
		yajl_gen_free(g);
		return QKPACK_ERROR;
	}
	
	request->response_buffer = std::string((const char*)buf,len);
	yajl_gen_free(g);
	
	return QKPACK_OK;
}


/**
 * 
 * Create Interface Response Body Json(SET,TTL,DEL,INCR,INCRBY,SADD,SREM,SCARD,SISMEMBER)
 * 
*/
int QkPackYajlJSON::WriterJsonTtl(qkpack_request_t *request) 
{
	/*
	{
	"e":{
		"code":0,    // 错误码，程序可以通过此错误码做不同的错误处理，或便于联调及快速定位问题
		"desc":""    // 针对错误码的描述，方便快速定位问题
	},
	  "cost": 2,     // 服务的处理耗时，单位毫秒
	  "data": 86400 || true || "OK"  // 以秒为单位，返回 key 的剩余生存时间，如果指定key不存在则会返回e.code=5000的错误。
	}
	*/
	yajl_gen                  g;
	size_t 					  len;
	const unsigned char 	  *buf;
    
	g = yajl_gen_alloc(NULL);
	if (g == NULL) {
        return QKPACK_ERROR;
    }
	yajl_gen_config(g, yajl_gen_beautify, 1);
	
	yajl_gen_map_open(g);
	
	WriterJsonCommon(request,g);
		
	if ( request->kvpair.command_type == REDIS_SISMEMBER ) {
		
		yajl_gen_string(g, (const unsigned char *) QKPACK_JSON_DATA, strlen(QKPACK_JSON_DATA));
		yajl_gen_bool(g, (request->kvpair.num?true:false));
	}
	else if ( request->kvpair.command_type == REDIS_SET ) {
		
		yajl_gen_string(g, (const unsigned char *) QKPACK_JSON_DATA, strlen(QKPACK_JSON_DATA));
		yajl_gen_string(g, (const unsigned char *) request->kvpair.value.c_str(), request->kvpair.value.length());
	}
	else {
		
		yajl_gen_string(g, (const unsigned char *) QKPACK_JSON_DATA, strlen(QKPACK_JSON_DATA));
		yajl_gen_integer(g, request->kvpair.num);
	}
	
    yajl_gen_map_close(g);
	
	yajl_gen_status status = yajl_gen_get_buf(g, &buf, &len);
	if(status != yajl_gen_status_ok) {
		yajl_gen_free(g);
		return QKPACK_ERROR;
	}
    
	request->response_buffer = std::string((const char*)buf,len);
	yajl_gen_free(g);

	return QKPACK_OK;
}


/**
 * 
 * Parse Interface Request Body Json(SET,INCRBY,SISMEMBER)
 * 
*/
int QkPackYajlJSON::ReadJsonSet(qkpack_request_t *request)
{
	/*
	{
		"uri" : "qkd://BJTEST/Bd",
		"ak" : "51v6sHF9Mk",
		"k" : "qkdtestkey",
		"v" : "test123"
	}
	*/
	int							rc;
	yajl_val					node;

	node = yajl_tree_parse(request->request_buffer.c_str(), NULL, 0);
	if ( node == NULL ) {
		request->desc =  QKPACK_ERROR_JSON_FORMAT; 
		return QKPACK_ERROR;
	}
	
	yajl_val uri = yajl_tree_get(node, (const char *[]){QKPACK_JSON_URI, 0}, yajl_t_string);
	yajl_val ak = yajl_tree_get(node, (const char *[]){QKPACK_JSON_AK, 0}, yajl_t_string);
	yajl_val key = yajl_tree_get(node, (const char *[]){QKPACK_JSON_KEY, 0}, yajl_t_string);
	yajl_val value = yajl_tree_get(node, (const char *[]){QKPACK_JSON_VALUE, 0}, yajl_t_string);
	
	//acl validate exits and not empty
	rc = qkpack_yajl_validate_->ValidateACLInfo(node, uri, ak , request->desc);
	if ( rc !=  QKPACK_OK ) {
		yajl_tree_free(node);
		return QKPACK_ERROR;
	}

	if ( request->kvpair.command_type == REDIS_SISMEMBER ) {
	
		//set key validate exits and not empty
		rc = qkpack_yajl_validate_->ValidateSetByKV(key,value,request->desc);
		if ( rc !=  QKPACK_OK ) {
			yajl_tree_free(node);
			return QKPACK_ERROR;
		}

	}
	else {

		//kvpair key validate exits and not empty
		rc = qkpack_yajl_validate_->ValidateKVPair(key,value,request->desc);
		if ( rc !=  QKPACK_OK ) {
			yajl_tree_free(node);
			return QKPACK_ERROR;
		}
	}

	char *val = YAJL_GET_STRING(value);
	if ( request->kvpair.command_type == REDIS_INCRBY ) {
		
		if ( atoi(val) == 0 ) {
			request->desc = QKPACK_ERROR_KVPAIR_NUMERIC_VALUE;
			yajl_tree_free(node);
			return QKPACK_ERROR;
		}
	}


	request->uri = std::string(YAJL_GET_STRING(uri));
	request->ak = std::string(YAJL_GET_STRING(ak));
	
	request->kvpair.key = std::string(YAJL_GET_STRING(key));
	request->kvpair.value = std::string(val);
	
	yajl_tree_free(node);
	return QKPACK_OK;
}


/**
 * 
 * Parse Interface Request Body Json(SADD,SREM)
 * 
*/
int QkPackYajlJSON::ReadJsonSadd(qkpack_request_t *request)
{
	/*
	{
		"uri" : "qkd://BJTEST/Bd",
		"ak" : "51v6sHF9Mk",
		"k" : "gstestkey2",
		"mbs" : [ "m2", "m3", "m4" ]
	}
	*/
	int							rc;
	yajl_val					node;

	node = yajl_tree_parse(request->request_buffer.c_str(), NULL, 0);
	if ( node == NULL ) {
		request->desc =  QKPACK_ERROR_JSON_FORMAT; 
		return QKPACK_ERROR;
	}
	
	yajl_val uri = yajl_tree_get(node, (const char *[]){QKPACK_JSON_URI, 0}, yajl_t_string);
	yajl_val ak = yajl_tree_get(node, (const char *[]){QKPACK_JSON_AK, 0}, yajl_t_string);
	yajl_val key = yajl_tree_get(node, (const char *[]){QKPACK_JSON_KEY, 0}, yajl_t_string);
	yajl_val mbs = yajl_tree_get(node, (const char *[]){QKPACK_JSON_MBS, 0}, yajl_t_array);
	
	//acl validate exits and not empty
	rc = qkpack_yajl_validate_->ValidateACLInfo(node, uri, ak , request->desc);
	if ( rc !=  QKPACK_OK ) {
		yajl_tree_free(node);
		return QKPACK_ERROR;
	}
	
	rc = qkpack_yajl_validate_->ValidateSet(key,mbs,request->desc);
	if ( rc !=  QKPACK_OK ) {
		yajl_tree_free(node);
		return QKPACK_ERROR;
	}
		
	request->uri = std::string(YAJL_GET_STRING(uri));
	request->ak = std::string(YAJL_GET_STRING(ak));
	request->kvpair.key = std::string(YAJL_GET_STRING(key));

	int len = YAJL_GET_ARRAY(mbs)->len;
	for (int i = 0; i < len; i++) {
		
		yajl_val val = YAJL_GET_ARRAY(mbs)->values[i];
		
		rc = qkpack_yajl_validate_->ValidateStrEmpty(val);
		if ( rc != QKPACK_OK ) {
			
			request->desc = QKPACK_ERROR_SET_MEMBERS_VALUE_NOT_EMPTY;
			yajl_tree_free(node);
			return QKPACK_ERROR;
		}
		
		zset_member_t mb;
		mb.member = std::string(YAJL_GET_STRING(val));
		
		request->vector_mbs.push_back(mb);
	}

	yajl_tree_free(node);
	return QKPACK_OK;
}



/**
 * 
 * Create Interface Response Body Json(SMEMBERS)
 * 
*/
int QkPackYajlJSON::WriterJsonSmembers(qkpack_request_t *request) 
{
	/*
	{
		"e" : {
			"code" : 0,
			"desc" : ""
		},
		"k" : "gstestkey2",    // 请求key
		"mbs" : [ "m3", "m2", "m1", "m4", ],    // 集合key的所有成员 
		"cost" : 2     // 服务的处理耗时，单位毫秒
	}
	*/
	yajl_gen                  g;
	size_t 					  len;
	const unsigned char 	  *buf;
    
	g = yajl_gen_alloc(NULL);
	if (g == NULL) {
        return QKPACK_ERROR;
    }
	yajl_gen_config(g, yajl_gen_beautify, 1);
	
	yajl_gen_map_open(g);
	
	WriterJsonCommon(request,g);
	
	yajl_gen_string(g, (const unsigned char *) QKPACK_JSON_KEY, strlen(QKPACK_JSON_KEY));
	yajl_gen_string(g, (const unsigned char *) request->kvpair.key.c_str(), request->kvpair.key.length());
	
	yajl_gen_string(g, (const unsigned char *) QKPACK_JSON_MBS, strlen(QKPACK_JSON_MBS));
	yajl_gen_array_open(g);
	
	std::vector<zset_member_t>::iterator it = request->vector_mbs.begin();
	
	for ( ; it != request->vector_mbs.end(); ++it ) {
		zset_member_t  &mb = *it;
		yajl_gen_string(g,(const unsigned char *) mb.member.c_str(), mb.member.length());
	}
	
	yajl_gen_array_close(g);
	
	yajl_gen_map_close(g);
	
	yajl_gen_status status = yajl_gen_get_buf(g, &buf, &len);
	if(status != yajl_gen_status_ok) {
		yajl_gen_free(g);
		return QKPACK_ERROR;
	}
    
	request->response_buffer = std::string((const char*)buf,len);
	yajl_gen_free(g);

	return QKPACK_OK;
}


/**
 * 
 * Parse Interface Request Body Json(MGET)
 * 
*/
int QkPackYajlJSON::ReadJsonMget(qkpack_request_t *request)
{
	/*
	{
		"uri" : "qkd://BJTEST/Bd",
		"ak" : "51v6sHF9Mk",
		"ks" : [ {
			"k" : "testkey1"
		}, {
			"k" : "testkey2"
		} ]
	}
	*/
	int							rc;
	yajl_val					node;

	node = yajl_tree_parse(request->request_buffer.c_str(), NULL, 0);
	if ( node == NULL ) {
		request->desc =  QKPACK_ERROR_JSON_FORMAT; 
		return QKPACK_ERROR;
	}
	
	yajl_val uri = yajl_tree_get(node, (const char *[]){QKPACK_JSON_URI, 0}, yajl_t_string);
	yajl_val ak = yajl_tree_get(node, (const char *[]){QKPACK_JSON_AK, 0}, yajl_t_string);
	yajl_val keysArray = yajl_tree_get(node, (const char *[]){QKPACK_JSON_KEYS, 0}, yajl_t_array);
	
	//acl validate exits and not empty
	rc = qkpack_yajl_validate_->ValidateACLInfo(node, uri, ak , request->desc);
	if ( rc !=  QKPACK_OK ) {
		yajl_tree_free(node);
		return QKPACK_ERROR;
	}
	
	//keys validate not exits and not empty
	rc =  qkpack_yajl_validate_->ValidateMultiKVPair(keysArray,request->desc);
	if ( rc != QKPACK_OK ) {
		yajl_tree_free(node);
		return QKPACK_ERROR;
	}
	
	request->uri = std::string(YAJL_GET_STRING(uri));
	request->ak = std::string(YAJL_GET_STRING(ak));
	
	int len = YAJL_GET_ARRAY(keysArray)->len;
	for (int i = 0; i < len; i++) {
		yajl_val val = YAJL_GET_ARRAY(keysArray)->values[i];
		
		yajl_val key_yajl = yajl_tree_get(val, (const char *[]){QKPACK_JSON_KEY, 0}, yajl_t_string);
		
		//key validate not exits and not empty
		rc = qkpack_yajl_validate_->ValidateKVPairByKey(key_yajl,request->desc);
		if ( rc != QKPACK_OK ) {
			yajl_tree_free(node);
			return QKPACK_ERROR;
		}
		
		kvpair_t kvpair;
		kvpair.key = std::string(YAJL_GET_STRING(key_yajl));

		request->vector_kv.push_back(kvpair);
	}
	
	int index = request->uri.find_last_of("/");
	if ( index <= 0 ) {
		yajl_tree_free(node);
		return QKPACK_OK;
	}
		
	request->kvpair.namespaces = request->uri.substr(index+1);
	if ( request->kvpair.namespaces.length() != QKPACK_ACL_NAMESPACE_LENGTH ) {
		yajl_tree_free(node);
		return QKPACK_OK;
	}
	
	int							slot_id;
	std::string					tmp_key;
	std::vector<kvpair_t>		&vector_kv = request->vector_kv;
	for( size_t i = 0; i < vector_kv.size(); ++i ) {
		tmp_key = request->kvpair.namespaces + vector_kv[i].key;
		slot_id = key_hash_slot((char*)tmp_key.c_str(),tmp_key.length());
		
		request->map_slots_kv[slot_id].push_back(&vector_kv[i]);
	}

	yajl_tree_free(node);
	return QKPACK_OK;
}



/**
 * 
 * Create Interface Response Body Json(MGET)
 * 
*/
int QkPackYajlJSON::WriterJsonMget(qkpack_request_t *request) 
{
	/*
	{
		"e" : {
			"code" : 0,
			"desc" : ""
		},
		"cost" : 11,
		"ks" : [ {
			"k" : "testkey1",
			"v" : "value1"	// 针对testkey1返回的value
		}, {
			"k" : "testkey2",
			"v" : null		// 查不到的key会返回null，返回的key顺序保证和请求的顺序相同
		} ]
	}
	*/
	yajl_gen                  g;
	size_t 					  len;
	const unsigned char 	  *buf;
    
	g = yajl_gen_alloc(NULL);
	if (g == NULL) {
        return QKPACK_ERROR;
    }
	yajl_gen_config(g, yajl_gen_beautify, 0);
	
	yajl_gen_map_open(g);
	
	WriterJsonCommon(request,g);
	
	yajl_gen_string(g, (const unsigned char *) QKPACK_JSON_KEYS, strlen(QKPACK_JSON_KEYS));
	yajl_gen_array_open(g);
	
	std::vector<kvpair_t> &vector_kv = request->vector_kv;
    for( size_t i = 0; i < vector_kv.size(); ++i ) {
		yajl_gen_map_open(g);
		
		kvpair_t  &kvpair = vector_kv[i];

		yajl_gen_string(g, (const unsigned char *) QKPACK_JSON_KEY, strlen(QKPACK_JSON_KEY));
		yajl_gen_string(g, (const unsigned char *) kvpair.key.c_str(), kvpair.key.length());
		
		yajl_gen_string(g, (const unsigned char *) QKPACK_JSON_VALUE, strlen(QKPACK_JSON_VALUE));

		if ( kvpair.value.empty() ) {
			yajl_gen_string(g, (const unsigned char *) "null", strlen("null"));
		}else {
			yajl_gen_string(g, (const unsigned char *) kvpair.value.c_str(), kvpair.value.length());
		}

		yajl_gen_map_close(g);
	}
	
	yajl_gen_array_close(g);
	
	yajl_gen_map_close(g);
	
	yajl_gen_status status = yajl_gen_get_buf(g, &buf, &len);
	if(status != yajl_gen_status_ok) {
		yajl_gen_free(g);
		return QKPACK_ERROR;
	}
    
	request->response_buffer = std::string((const char*)buf,len);
	yajl_gen_free(g);

	return QKPACK_OK;
}


/**
 * 
 * Parse Interface Request Body Json(MSET)
 * 
*/
int QkPackYajlJSON::ReadJsonMset(qkpack_request_t *request)
{
	/*
	{
		"uri" : "qkd://BJTEST/Bd",
		"ak" : "51v6sHF9Mk",
		"ks" : [ {
			"k" : "testkey1",
			"v" : "value1"
		}, {
			"k" : "testkey2",
			"v" : "value2"
		} ]
	}
	*/
	int							rc;
	yajl_val					node;

	node = yajl_tree_parse(request->request_buffer.c_str(), NULL, 0);
	if ( node == NULL ) {
		request->desc =  QKPACK_ERROR_JSON_FORMAT; 
		return QKPACK_ERROR;
	}
	
	yajl_val uri = yajl_tree_get(node, (const char *[]){QKPACK_JSON_URI, 0}, yajl_t_string);
	yajl_val ak = yajl_tree_get(node, (const char *[]){QKPACK_JSON_AK, 0}, yajl_t_string);
	yajl_val keysArray = yajl_tree_get(node, (const char *[]){QKPACK_JSON_KEYS, 0}, yajl_t_array);
	
	//acl validate exits and not empty
	rc = qkpack_yajl_validate_->ValidateACLInfo(node, uri, ak , request->desc);
	if ( rc !=  QKPACK_OK ) {
		yajl_tree_free(node);
		return QKPACK_ERROR;
	}
	
	//keys validate not exits and not empty
	rc =  qkpack_yajl_validate_->ValidateMultiKVPair(keysArray,request->desc);
	if ( rc != QKPACK_OK ) {
		yajl_tree_free(node);
		return QKPACK_ERROR;
	}
	
	request->uri = std::string(YAJL_GET_STRING(uri));
	request->ak = std::string(YAJL_GET_STRING(ak));
	
	int					len = YAJL_GET_ARRAY(keysArray)->len;
	for (int i = 0; i < len; i++) {
		yajl_val val = YAJL_GET_ARRAY(keysArray)->values[i];
		
		yajl_val key = yajl_tree_get(val, (const char *[]){QKPACK_JSON_KEY, 0}, yajl_t_string);
		yajl_val value = yajl_tree_get(val, (const char *[]){QKPACK_JSON_VALUE, 0}, yajl_t_string);
		
		//key validate not exits and not empty
		rc = qkpack_yajl_validate_->ValidateKVPair(key,value,request->desc);
		if ( rc != QKPACK_OK ) {
			yajl_tree_free(node);
			return QKPACK_ERROR;
		}
		
		kvpair_t	kvpair;
		kvpair.key = std::string(YAJL_GET_STRING(key));
		kvpair.value = std::string(YAJL_GET_STRING(value));
		
		request->vector_kv.push_back(kvpair);
	}
	
	int index = request->uri.find_last_of("/");
	if ( index <= 0 ) {
		yajl_tree_free(node);
		return QKPACK_OK;
	}
		
	request->kvpair.namespaces = request->uri.substr(index+1);
	if ( request->kvpair.namespaces.length() != QKPACK_ACL_NAMESPACE_LENGTH ) {
		yajl_tree_free(node);
		return QKPACK_OK;
	}
	
	int							slot_id;
	std::string					tmp_key;
	std::vector<kvpair_t>		&vector_kv = request->vector_kv;
	for( size_t i = 0; i < vector_kv.size(); ++i ) {
		tmp_key = request->kvpair.namespaces + vector_kv[i].key;
		slot_id = key_hash_slot((char*)tmp_key.c_str(),tmp_key.length());
		
		request->map_slots_kv[slot_id].push_back(&vector_kv[i]);
	}	

	yajl_tree_free(node);
	return QKPACK_OK;
}


/**
 * 
 * Create Interface Response Body Json(MSET)
 * 
*/
int QkPackYajlJSON::WriterJsonMset(qkpack_request_t *request) 
{
	/*
	{
		"e" : {
			"code" : 0,
			"desc" : ""
		},
		"cost" : 11,
	}
	*/
	yajl_gen                  g;
	size_t 					  len;
	const unsigned char 	  *buf;
    
	g = yajl_gen_alloc(NULL);
	if (g == NULL) {
        return QKPACK_ERROR;
    }
	yajl_gen_config(g, yajl_gen_beautify, 1);
	
	yajl_gen_map_open(g);
	
	WriterJsonCommon(request,g);
	
	yajl_gen_map_close(g);
	
	yajl_gen_status status = yajl_gen_get_buf(g, &buf, &len);
	if(status != yajl_gen_status_ok) {
		yajl_gen_free(g);
		return QKPACK_ERROR;
	}
    
	request->response_buffer = std::string((const char*)buf,len);
	yajl_gen_free(g);

	return QKPACK_OK;
}


