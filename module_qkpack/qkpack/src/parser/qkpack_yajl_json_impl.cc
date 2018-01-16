#include "parser/qkpack_yajl_json_impl.h"
#include "core/redis_slot.h"

extern "C" {
#include <yajl/yajl_parse.h>
#include <yajl/yajl_gen.h>
#include <yajl/yajl_tree.h>
}

using namespace com::youku::data::qkpack::parser;



/**
 * 
 * 解析Request Body Json(ZADD)
 * 
*/
int QkPackYajlJSONImpl::ReadJsonZfixedsetAdd(qkpack_request_t *request)
{
	/*
	{
		"k":"userIdOrCookieId",
		"uri":"qkpack://SH/abc",
		"ak":"5uxPT7drew",
		"mbs":[
			{
				"mb":"vvid1",
				"sc":1427883739,
				"v":"要存储的第一条member信息的具体内容"
			},
			{
				"mb":"vvid2",
				"sc":1427884322,
				"v":"要存储的第二条member信息的具体内容"
			},
			{
				"mb":"vvid3",
				"sc":1427887589,
				"v":"要存储的第三条member信息的具体内容"
			}
			
		]
	}
	*/
	int							rc;
	yajl_val					node;

	node = yajl_tree_parse(request->request_buffer.c_str(), NULL, 0);
	if ( node == NULL ) {
		request->desc = QKPACK_ERROR_JSON_FORMAT;
		return QKPACK_ERROR;
	}
	
	yajl_val uri = yajl_tree_get(node, (const char *[]){QKPACK_JSON_URI, 0}, yajl_t_string);
	yajl_val ak = yajl_tree_get(node, (const char *[]){QKPACK_JSON_AK, 0}, yajl_t_string);
	yajl_val key = yajl_tree_get(node, (const char *[]){QKPACK_JSON_KEY, 0}, yajl_t_string);
	yajl_val mbsArray = yajl_tree_get(node, (const char *[]){QKPACK_JSON_MBS, 0}, yajl_t_array);
	
	//acl validate exist and not empty
	rc = qkpack_yajl_validate_->ValidateACLInfo(node, uri, ak , request->desc);
	if ( rc !=  QKPACK_OK ) {
		yajl_tree_free(node);
		return QKPACK_ERROR;
	}

	//key,mbs validate exist , array and empty
	rc = qkpack_yajl_validate_->ValidateZSet(key,mbsArray,request->desc);
	if ( rc !=  QKPACK_OK ) {
		yajl_tree_free(node);
		return QKPACK_ERROR;
	}
	
	request->uri = std::string(YAJL_GET_STRING(uri));
	request->ak = std::string(YAJL_GET_STRING(ak));
	std::string key_ns = std::string(YAJL_GET_STRING(key));
	
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

	int len = YAJL_GET_ARRAY(mbsArray)->len;
	
	for (int i = 0; i < len; i++) {
		yajl_val val = YAJL_GET_ARRAY(mbsArray)->values[i];
		
		yajl_val member = yajl_tree_get(val, (const char *[]){QKPACK_JSON_MB, 0}, yajl_t_string);
		yajl_val value = yajl_tree_get(val, (const char *[]){QKPACK_JSON_V, 0}, yajl_t_string);
		yajl_val sc = yajl_tree_get(val, (const char *[]){QKPACK_JSON_SC, 0}, yajl_t_number);
		
		rc = qkpack_yajl_validate_->ValidateZSetMembers(member,value,sc,request->desc);
		if ( rc != QKPACK_OK ) {
			yajl_tree_free(node);
			return QKPACK_ERROR;
		}
	
		//kvpair
		kvpair_t kvpair;
		kvpair.key = std::string(YAJL_GET_STRING(member));
		kvpair.value =  std::string(YAJL_GET_STRING(value));
		kvpair.score = YAJL_GET_DOUBLE(sc);
			
		request->map_kv[key_ns].push_back(kvpair);	
	}

	int						slot_id;
	std::string				kv_key;
	std::vector<kvpair_t>	&vector_kv = request->map_kv[key_ns];
	std::string				&namespaces = request->kvpair.namespaces;

	for( size_t i = 0; i< vector_kv.size(); ++i ) {
		kv_key = namespaces + vector_kv[i].key;
		slot_id = key_hash_slot((char*)kv_key.c_str(), kv_key.length());
		
		request->map_slots_kv[slot_id].push_back(&vector_kv[i]);
	}

	yajl_tree_free(node);
	return QKPACK_OK;
}


/**
 * 
 * 解析Request Body Json(ZADD Batch)
 * 
*/
int QkPackYajlJSONImpl::ReadJsonZfixedsetBatchAdd(qkpack_request_t *request)
{
	/*
	{
		"uri":"qkpack://SH/abc",
		"ak":"5uxPT7drew",
		"ks":[
			{
				"k":"userid1",
				"mbs":[
					{
						"mb":"vvid1",
						"sc":1427883739,
						"v":"要存储的第一条member信息的具体内容"
					},
					{
						"mb":"vvid2",
						"sc":1427884322,
						"v":"要存储的第二条member信息的具体内容"
					},
					{
						"mb":"vvid3",
						"sc":1427887589,
						"v":"要存储的第三条member信息的具体内容"
					}
					
				]
				
			},
			{
				"k":"userid2",
				"mbs":[
					{
						"mb":"vvid1",
						"sc":1427883739,
						"v":"要存储的第一条member信息的具体内容"
					},
					{
						"mb":"vvid2",
						"sc":1427884322,
						"v":"要存储的第二条member信息的具体内容"
					},
					{
						"mb":"vvid3",
						"sc":1427887589,
						"v":"要存储的第三条member信息的具体内容"
					}
					
				]
				
			}
			
		]
	}
	*/
	int							rc;
	yajl_val					node;

	node = yajl_tree_parse(request->request_buffer.c_str(), NULL, 0);
	if ( node == NULL ) {
		request->desc = QKPACK_ERROR_JSON_FORMAT;
		return QKPACK_ERROR;
	}
	
	yajl_val uri = yajl_tree_get(node, (const char *[]){QKPACK_JSON_URI, 0}, yajl_t_string);
	yajl_val ak = yajl_tree_get(node, (const char *[]){QKPACK_JSON_AK, 0}, yajl_t_string);
	yajl_val keysArray = yajl_tree_get(node, (const char *[]){QKPACK_JSON_KEYS, 0}, yajl_t_array);
	
	//acl validate exist and not empty
	rc = qkpack_yajl_validate_->ValidateACLInfo(node, uri, ak , request->desc);
	if ( rc !=  QKPACK_OK ) {
		yajl_tree_free(node);
		return QKPACK_ERROR;
	}
	
	//keys validate not exist and not empty
	rc =  qkpack_yajl_validate_->ValidateZSetKeys(keysArray,request->desc);
	if ( rc != QKPACK_OK ) {
		yajl_tree_free(node);
		return QKPACK_ERROR;
	}
	
	request->uri = std::string(YAJL_GET_STRING(uri));
	request->ak = std::string(YAJL_GET_STRING(ak));
	
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

	int len = YAJL_GET_ARRAY(keysArray)->len;
	for (int i = 0; i < len; i++) {
		yajl_val val = YAJL_GET_ARRAY(keysArray)->values[i];

		yajl_val key = yajl_tree_get(val, (const char *[]){QKPACK_JSON_KEY, 0}, yajl_t_string);
		yajl_val mbsArray = yajl_tree_get(val, (const char *[]){QKPACK_JSON_MBS, 0}, yajl_t_array);

		rc =  qkpack_yajl_validate_->ValidateZSet(key,mbsArray,request->desc);
		if ( rc != QKPACK_OK ) {
			yajl_tree_free(node);
			return QKPACK_ERROR;
		}
		
		int mbs_len = YAJL_GET_ARRAY(mbsArray)->len;
		
		for(int i = 0; i < mbs_len; ++i) {
			yajl_val mbsObject = YAJL_GET_ARRAY(mbsArray)->values[i];
			
			yajl_val mb = yajl_tree_get(mbsObject, (const char *[]){QKPACK_JSON_MB, 0}, yajl_t_string);
			yajl_val sc = yajl_tree_get(mbsObject, (const char *[]){QKPACK_JSON_SC, 0}, yajl_t_number);
			yajl_val value = yajl_tree_get(mbsObject, (const char *[]){QKPACK_JSON_V, 0}, yajl_t_string);
			
			rc =  qkpack_yajl_validate_->ValidateZSetMembers(mb,value,sc,request->desc);
			if ( rc != QKPACK_OK ) {
				yajl_tree_free(node);
				return QKPACK_ERROR;
			}
			
			//kvpair
			kvpair_t kvpair;
			kvpair.key = std::string(YAJL_GET_STRING(mb));
			kvpair.value =  std::string(YAJL_GET_STRING(value));
			kvpair.score = YAJL_GET_DOUBLE(sc);
				
			request->map_kv[std::string(YAJL_GET_STRING(key))].push_back(kvpair);	
		}	
	}
	
	int														 slot_id;
	std::string												 kv_key;
	std::string												 &namespaces = request->kvpair.namespaces;
	std::map<std::string, std::vector<kvpair_t> >			 &map_kv = request->map_kv;	
	std::map<std::string, std::vector<kvpair_t> >::iterator  it;
	
	for( it = map_kv.begin(); it != map_kv.end(); ++it ) {
	
		for( size_t i = 0; i< it->second.size(); ++i ) {
			kv_key = namespaces + it->second[i].key;
			slot_id = key_hash_slot((char*)kv_key.c_str(), kv_key.length());
			
			request->map_slots_kv[slot_id].push_back(&it->second[i]);
		}
	}

	yajl_tree_free(node);
	return QKPACK_OK;
}


/**
 * 
 * 解析Request Body Json(ZRANGEBYSCORE,ZREVRANGEBYSCORE)
 * 
*/
int QkPackYajlJSONImpl::ReadJsonZfixedsetGetByScore(qkpack_request_t *request)
{
	/*
	{
		"k":"xxxUserIdOrCookieId",
		"min":1427883739,
		"max":1427891877,
		"ws":true,
		"asc":false,
		"uri":"qkpack://SH/abc",
		"ak":"5uxPT7drew"
	}
	*/
	int							rc;
	yajl_val					node;

	node = yajl_tree_parse(request->request_buffer.c_str(), NULL, 0);
	if ( node == NULL ) {
		request->desc = QKPACK_ERROR_JSON_FORMAT;
		return QKPACK_ERROR;
	}
	
	yajl_val uri = yajl_tree_get(node, (const char *[]){QKPACK_JSON_URI, 0}, yajl_t_string);
	yajl_val ak = yajl_tree_get(node, (const char *[]){QKPACK_JSON_AK, 0}, yajl_t_string);
	yajl_val key = yajl_tree_get(node, (const char *[]){QKPACK_JSON_KEY, 0}, yajl_t_string);
	yajl_val min = yajl_tree_get(node, (const char *[]){QKPACK_JSON_MIN, 0}, yajl_t_number);
	yajl_val max = yajl_tree_get(node, (const char *[]){QKPACK_JSON_MAX, 0}, yajl_t_number);
	yajl_val ws = yajl_tree_get(node, (const char *[]){QKPACK_JSON_WS, 0}, yajl_t_true);
	yajl_val asc = yajl_tree_get(node, (const char *[]){QKPACK_JSON_ASC, 0}, yajl_t_true);
	
	//acl validate exist and not empty
	rc = qkpack_yajl_validate_->ValidateACLInfo(node, uri, ak , request->desc);
	if ( rc !=  QKPACK_OK ) {
		yajl_tree_free(node);
		return QKPACK_ERROR;
	}
	
	//key,min,manx validate exist
	rc = qkpack_yajl_validate_->ValidateZSetQuery(key, min, max , request->desc);
	if ( rc !=  QKPACK_OK ) {
		yajl_tree_free(node);
		return QKPACK_ERROR;
	}
	
	request->uri = std::string(YAJL_GET_STRING(uri));
	request->ak = std::string(YAJL_GET_STRING(ak));
	std::string key_ns = std::string(YAJL_GET_STRING(key));
	
	zset_query_t zset_query;
	zset_query.min = YAJL_GET_DOUBLE(min);
	zset_query.max = YAJL_GET_DOUBLE(max);
	
	if ( ws ) {
		zset_query.ws = YAJL_IS_TRUE(ws)?true:false;
	}
	
	if ( asc ) {
		zset_query.asc = YAJL_IS_TRUE(asc)?true:false;
	}
	
	request->map_query[key_ns] = zset_query;
	
	yajl_tree_free(node);
	return QKPACK_OK;
}


/**
 * 
 * 构建Response Body Json(ZRANGEBYSCORE,ZREVRANGEBYSCORE)
 * 
*/
int QkPackYajlJSONImpl::WriterJsonZfixedsetGetByScore(qkpack_request_t *request)
{
	/*
	{
		"k":"xxxUserIdOrCookieId",
		"mc":2,
		"mbs":[
				{
					"mb":"vvid1",
					"sc":1427883739,
					"v":"abc"
				},
				{
					"mb":"vvid2",
					"sc":1427891877,
					"v":"def"
				}
			],
		"e":{
			"code":0,
			"desc":""
		},
		"cost":4
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

	std::map<std::string, std::vector<kvpair_t> >				&map_kv = request->map_kv;
	std::map<std::string, std::vector<kvpair_t> >::iterator		it;

	for( it = map_kv.begin(); it != map_kv.end(); ++it ) {

		yajl_gen_string(g, (const unsigned char *) QKPACK_JSON_KEY, strlen(QKPACK_JSON_KEY));
		yajl_gen_string(g, (const unsigned char *) it->first.c_str(), it->first.length());
	
		std::vector<kvpair_t>		&vector_kv = it->second;

		yajl_gen_string(g, (const unsigned char *) QKPACK_JSON_MC, strlen(QKPACK_JSON_MC));
		yajl_gen_integer(g, vector_kv.size());
	
		yajl_gen_string(g, (const unsigned char *) QKPACK_JSON_MBS, strlen(QKPACK_JSON_MBS));
		yajl_gen_array_open(g);
	
		for( size_t i = 0; i < vector_kv.size(); ++i ) {
			kvpair_t &kvpair = vector_kv[i];
		
			yajl_gen_map_open(g);
		
			yajl_gen_string(g, (const unsigned char *) QKPACK_JSON_MB, strlen(QKPACK_JSON_MB));
			yajl_gen_string(g, (const unsigned char *) kvpair.key.c_str(), kvpair.key.length());
		
			yajl_gen_string(g, (const unsigned char *) QKPACK_JSON_SC, strlen(QKPACK_JSON_SC));
			yajl_gen_double(g, kvpair.score);
		
			yajl_gen_string(g, (const unsigned char *) QKPACK_JSON_V, strlen(QKPACK_JSON_V));
			yajl_gen_string(g, (const unsigned char *) kvpair.value.c_str(), kvpair.value.length());
		
			yajl_gen_map_close(g);
		}
		yajl_gen_array_close(g);
		
		break;
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
 * 解析Request Body Json Batch(ZRANGEBYSCORE,ZREVRANGEBYSCORE)
 * 
*/
int QkPackYajlJSONImpl::ReadJsonZfixedsetBatchGetByScore(qkpack_request_t *request)
{
	/*
	{
		"uri":"qkpack://SH/abc",
		"ak":"5uxPT7drew",
		"ks":[
				{
					"k":"userid1",
					"min":1427883739,
					"max":1427891877,
					"ws":true,
					"asc":false
				},
				{
					"k":"userid2",
					"min":1427883739,
					"max":1427891877,
					"ws":true,
					"asc":false
				}
				
	}
	*/
	int							rc;
	yajl_val					node;

	node = yajl_tree_parse(request->request_buffer.c_str(), NULL, 0);
	if ( node == NULL  ) {
		request->desc = QKPACK_ERROR_JSON_FORMAT;
		return QKPACK_ERROR;
	}
	
	yajl_val uri = yajl_tree_get(node, (const char *[]){QKPACK_JSON_URI, 0}, yajl_t_string);
	yajl_val ak = yajl_tree_get(node, (const char *[]){QKPACK_JSON_AK, 0}, yajl_t_string);
	yajl_val keysArray = yajl_tree_get(node, (const char *[]){QKPACK_JSON_KEYS, 0}, yajl_t_array);
	
	//acl validate exist and empty
	rc = qkpack_yajl_validate_->ValidateACLInfo(node,uri,ak,request->desc);
	if ( rc != QKPACK_OK ) {
		yajl_tree_free(node);
		return QKPACK_ERROR;
	}
	
	//keys validate exits and empty
	rc = qkpack_yajl_validate_->ValidateZSetKeys(keysArray,request->desc);
	if ( rc != QKPACK_OK ) {
		yajl_tree_free(node);
		return QKPACK_ERROR;
	}
	
	request->uri = std::string(YAJL_GET_STRING(uri));
	request->ak = std::string(YAJL_GET_STRING(ak));
	
	int len = YAJL_GET_ARRAY(keysArray)->len;
	for (int i = 0; i < len; i++) {
		yajl_val val = YAJL_GET_ARRAY(keysArray)->values[i];

		yajl_val key = yajl_tree_get(val, (const char *[]){QKPACK_JSON_KEY, 0}, yajl_t_string);
		yajl_val min = yajl_tree_get(val, (const char *[]){QKPACK_JSON_MIN, 0}, yajl_t_number);
		yajl_val max = yajl_tree_get(val, (const char *[]){QKPACK_JSON_MAX, 0}, yajl_t_number);
		yajl_val ws = yajl_tree_get(val, (const char *[]){QKPACK_JSON_WS, 0}, yajl_t_true);
		yajl_val asc = yajl_tree_get(val, (const char *[]){QKPACK_JSON_ASC, 0}, yajl_t_true);
	
		//key,min,manx validate exist
		rc = qkpack_yajl_validate_->ValidateZSetQuery(key, min, max , request->desc);
		if ( rc !=  QKPACK_OK ) {
			yajl_tree_free(node);
			return QKPACK_ERROR;
		}
		
		zset_query_t zset_query;
		zset_query.min = YAJL_GET_DOUBLE(min);
		zset_query.max = YAJL_GET_DOUBLE(max);
		
		if ( ws ) {
			zset_query.ws = YAJL_IS_TRUE(ws)?true:false;
		}
		
		if ( asc ) {
			zset_query.asc = YAJL_IS_TRUE(asc)?true:false;
		}
		kvpair_t kvpair;
		kvpair.key = std::string(YAJL_GET_STRING(key));
		
		request->vector_kv.push_back(kvpair);
		request->map_query[kvpair.key] = zset_query;
	}
	
	yajl_tree_free(node);
	return QKPACK_OK;
}


/**
 * 
 * 构建Response Body Json Batch(ZRANGEBYSCORE,ZREVRANGEBYSCORE)
 * 
*/
int QkPackYajlJSONImpl::WriterJsonZfixedsetBatchGetByScore(qkpack_request_t *request)
{
	/*
	{
		"kc":2,
		"ks":[
				{
					"k":"userid1",
					"mc":3,
					"mbs":[
							{
								"mb":"vvid1",
								"sc":1427883739,
								"v":"要存储的第一条member信息的具体内容"
							},
							{
								"mb":"vvid2",
								"sc":1427884322,
								"v":"要存储的第二条member信息的具体内容"
							},
							{
								"mb":"vvid3",
								"sc":1427887589,
								"v":"要存储的第三条member信息的具体内容"
							}
					]
				},
				{
					"k":"userid2",
					"mc":3,
					"mbs":[
							{
								"mb":"vvid1",
								"sc":1427883739,
								"v":"要存储的第一条member信息的具体内容"
							},
							{
								"mb":"vvid2",
								"sc":1427884322,
								"v":"要存储的第二条member信息的具体内容"
							},
							{
								"mb":"vvid3",
								"sc":1427887589,
								"v":"要存储的第三条member信息的具体内容"
							}
					]
				}
			],				
		"e":{
			"code":0,
			"desc":""
		},
		"cost":4
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
	
	yajl_gen_string(g, (const unsigned char *) QKPACK_JSON_KC, strlen(QKPACK_JSON_KC));
	yajl_gen_integer(g, request->vector_kv.size());
	
	yajl_gen_string(g, (const unsigned char *) QKPACK_JSON_KEYS, strlen(QKPACK_JSON_KEYS));
	
	yajl_gen_array_open(g);
	
	std::vector<kvpair_t>										&vector_key = request->vector_kv;
	std::map<std::string,std::vector<kvpair_t> >				&map_kv = request->map_kv;
	std::map<std::string,std::vector<kvpair_t> >::iterator		it_map;

	for ( size_t i = 0; i < vector_key.size(); ++i ) {

		std::string &key = vector_key[i].key;

		yajl_gen_map_open(g);
		
		yajl_gen_string(g, (const unsigned char *) QKPACK_JSON_KEY, strlen(QKPACK_JSON_KEY));
		yajl_gen_string(g, (const unsigned char *) key.c_str(), key.length());
		
		it_map = map_kv.find(key);
		if ( it_map == map_kv.end() ) {
	
			yajl_gen_string(g, (const unsigned char *) QKPACK_JSON_MC, strlen(QKPACK_JSON_MC));
			yajl_gen_integer(g, 0);
			
			yajl_gen_string(g, (const unsigned char *) QKPACK_JSON_MBS, strlen(QKPACK_JSON_MBS));
			yajl_gen_array_open(g);
			yajl_gen_array_close(g);
			yajl_gen_map_close(g);

			continue;		
		}

		std::vector<kvpair_t>				 &vector_kv = it_map->second;
		std::vector<kvpair_t>::iterator		 it = vector_kv.begin();
		
		yajl_gen_string(g, (const unsigned char *) QKPACK_JSON_MC, strlen(QKPACK_JSON_MC));
		yajl_gen_integer(g, vector_kv.size());

		yajl_gen_string(g, (const unsigned char *) QKPACK_JSON_MBS, strlen(QKPACK_JSON_MBS));
		
		yajl_gen_array_open(g);
		
		for ( ; it != vector_kv.end(); ++it) {
			kvpair_t &kvpair = *it;
			
			yajl_gen_map_open(g);
			
			yajl_gen_string(g, (const unsigned char *) QKPACK_JSON_MB, strlen(QKPACK_JSON_MB));
			yajl_gen_string(g, (const unsigned char *) kvpair.key.c_str(), kvpair.key.length());
			
			yajl_gen_string(g, (const unsigned char *) QKPACK_JSON_SC, strlen(QKPACK_JSON_SC));
			yajl_gen_double(g, kvpair.score);
			
			yajl_gen_string(g, (const unsigned char *) QKPACK_JSON_V, strlen(QKPACK_JSON_V));
			yajl_gen_string(g, (const unsigned char *) kvpair.value.c_str(), kvpair.value.length());
	
			yajl_gen_map_close(g);
		}
		
		yajl_gen_array_close(g);
		
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



