#include "parser/qkpack_yajl_validate.h"

using namespace com::youku::data::qkpack::parser;


/**
 * 
 * Validate String Empty
 *
 * 1. string not empty
 * 2. string Space
 * 
*/
int QkPackYajlValidate::ValidateStrEmpty(yajl_val &val)
{
	int 								i;
	int 								val_len;	
	char								*val_str;

	i = 0;
	val_str = YAJL_GET_STRING(val);
	val_len = strlen(val_str);

	if ( !val_len  ) {
		return QKPACK_ERROR;
	}

	if ( val_len == 1 && val_str[0] == ' ' ) {
		return QKPACK_ERROR;
	}

	while(i < val_len && val_str[i++] == ' ');
	
	if ( i != 1 && i == val_len ) {
		return QKPACK_ERROR;
	}
	
	return QKPACK_OK;
}


/**
 * 
 * Validate ACL Interface Info
 *
 * 1. uri,appkey not exits 
 * 2. uri,appkey not empty
 * 
*/
int QkPackYajlValidate::ValidateACLInfo(yajl_val &node, yajl_val &uri, yajl_val &ak, std::string &desc)
{
	int									uri_status;
	int									ak_status;

	if ( !node ) {	
		desc = QKPACK_ERROR_ACL_NOT_NULL;
		return QKPACK_ERROR;
	}
	
	if ( !uri && !ak ) {
		desc = QKPACK_ERROR_ACL_NOT_NULL;
		return QKPACK_ERROR;
	}

	if ( !uri ) {
		desc = QKPACK_ERROR_ACL_URI_NOT_EXIST;
		return QKPACK_ERROR;
	}

	if ( !ak ) {
		desc = QKPACK_ERROR_ACL_APPKEY_NOT_EXIST;
		return QKPACK_ERROR;
	}
	
	uri_status = ValidateStrEmpty(uri);
	ak_status = ValidateStrEmpty(ak);

	if ( uri_status != QKPACK_OK && ak_status != QKPACK_OK ) {
		desc = QKPACK_ERROR_ACL_NOT_NULL;
		return QKPACK_ERROR;
	}

	if ( uri_status != QKPACK_OK ) {
		desc = QKPACK_ERROR_ACL_URI_NOT_EMPTY;
		return QKPACK_ERROR;
	}
	
	if ( ak_status != QKPACK_OK ) {
		desc = QKPACK_ERROR_ACL_APPKEY_NOT_EMPTY;
		return QKPACK_ERROR;
	}
	
	return QKPACK_OK;
}


/**
 * 
 * Validate KVPair Interface KEY Info
 *
 * 1. key not exits
 * 2. key not empty
 * 
*/
int QkPackYajlValidate::ValidateKVPairByKey(yajl_val &key,std::string &desc)
{
	int									rc;
	
	if ( !key ) {
		desc = QKPACK_ERROR_KVPAIR_KEY_NOT_EXIST;
		return QKPACK_ERROR;
	}
	
	rc = ValidateStrEmpty(key);
	if ( rc != QKPACK_OK ) {
		desc = QKPACK_ERROR_KVPAIR_KEY_NOT_EMPTY;
		return QKPACK_ERROR;
	}
	
	return QKPACK_OK;
}

/**
 * 
 * Validate KVPair Interface  KEY/VALUE Info
 *
 * 1. key,value not exits
 * 2. key,value not empty
 * 
*/
int QkPackYajlValidate::ValidateKVPair(yajl_val &key,yajl_val &value,std::string &desc)
{
	int									rc;
	
	rc = ValidateKVPairByKey(key,desc);
	if ( rc != QKPACK_OK ) {
		return QKPACK_ERROR;
	}
	
	if ( !value ) {
		desc = QKPACK_ERROR_KVPAIR_VALUE_NOT_EXIST;
		return QKPACK_ERROR;
	}
	
	rc = ValidateStrEmpty(value);
	if ( rc != QKPACK_OK ) {
		desc = QKPACK_ERROR_KVPAIR_VALUE_NOT_EMPTY;
		return QKPACK_ERROR;
	}

	return QKPACK_OK;
}

/**
 * 
 * Validate MultiKVPair Interface KEYS Info
 * 
 * 1. keys not exits
 * 2. keys not array and not empty list
*/
int QkPackYajlValidate::ValidateMultiKVPair(yajl_val &keys,std::string &desc)
{
	if ( !keys ) {
		desc = QKPACK_ERROR_KVPAIR_KEYS_NOT_EXIST;
		return QKPACK_ERROR;
	}
	
	if ( !YAJL_IS_ARRAY(keys) || !YAJL_GET_ARRAY(keys)->len ) {
		desc = QKPACK_ERROR_KVPAIR_KEYS_NOT_EMPTY;
		return QKPACK_ERROR;
	}
	
	return QKPACK_OK;
}

/**
 * 
 * Validate Set Interface KEY Info
 *
 * 1. key not exits
 * 2. key not empty
 * 
*/
int QkPackYajlValidate::ValidateSetByKey(yajl_val &key,std::string &desc)
{
	int									rc;
	
	if ( !key ) {
		desc = QKPACK_ERROR_SET_KEY_NOT_EXIST;
		return QKPACK_ERROR;
	}
	
	rc = ValidateStrEmpty(key);
	if ( rc != QKPACK_OK ) {
		desc = QKPACK_ERROR_SET_KEY_NOT_EMPTY;
		return QKPACK_ERROR;
	}
	
	return QKPACK_OK;
}


/**
 * 
 * Validate Set Interface Info
 *
 * 1. key not exits
 * 2. key not empty
 * 3. mbs not exits
 * 4. mbs not array and not empty list
 * 
*/
int QkPackYajlValidate::ValidateSet(yajl_val &key,yajl_val &mbs,std::string &desc)
{
	int									rc;
	
	rc = ValidateSetByKey(key,desc);
	if ( rc != QKPACK_OK ) {
		return QKPACK_ERROR;
	}
	
	if ( !mbs ) {
		desc = QKPACK_ERROR_SET_MEMBERS_NOT_EXIST;
		return QKPACK_ERROR;
	}
	
	if ( !YAJL_IS_ARRAY(mbs) || !YAJL_GET_ARRAY(mbs)->len ) {
		desc = QKPACK_ERROR_SET_MEMBERS_NOT_EMPTY;
		return QKPACK_ERROR;
	}
	
	return QKPACK_OK;
}


/**
 * 
 * Validate SET Interface  KEY/VALUE Info
 *
 * 1. key,value not exits
 * 2. key,value not empty
 * 
*/
int QkPackYajlValidate::ValidateSetByKV(yajl_val &key,yajl_val &value,std::string &desc)
{
	int									rc;
	
	rc = ValidateSetByKey(key,desc);
	if ( rc != QKPACK_OK ) {
		return QKPACK_ERROR;
	}
	
	if ( !value ) {
		desc = QKPACK_ERROR_SET_VALUE_NOT_EXIST;
		return QKPACK_ERROR;
	}
	
	rc = ValidateStrEmpty(value);
	if ( rc != QKPACK_OK ) {
		desc = QKPACK_ERROR_SET_VALUE_NOT_EMPTY;
		return QKPACK_ERROR;
	}

	return QKPACK_OK;
}




/**
 * 
 * Validate ZSet Interface Info
 *
 * 1. key not exits
 * 2. key not empty
 * 3. mbs not exits
 * 4. mbs not array and not empty list
 * 
*/
int QkPackYajlValidate::ValidateZSet(yajl_val &key,yajl_val &mbs,std::string &desc)
{
	int									rc;
	
	if ( !key ) {
		desc = QKPACK_ERROR_ZSET_KEY_NOT_EXIST;
		return QKPACK_ERROR;
	}
	
	rc = ValidateStrEmpty(key);
	if ( rc != QKPACK_OK ) {
		desc = QKPACK_ERROR_ZSET_KEY_NOT_EMPTY;
		return QKPACK_ERROR;
	}
	
	if ( !mbs ) {
		desc = QKPACK_ERROR_ZSET_MEMBERS_NOT_EXIST;
		return QKPACK_ERROR;
	}
	
	if ( !YAJL_IS_ARRAY(mbs) || !YAJL_GET_ARRAY(mbs)->len ) {
		desc = QKPACK_ERROR_ZSET_MEMBERS_NOT_EMPTY;
		return QKPACK_ERROR;
	}
	
	return QKPACK_OK;
}

/**
 * 
 * Validate ZSet Interface KEYS Info
 * 
 * 1. keys not exits
 * 2. keys not array and not empty list
*/
int QkPackYajlValidate::ValidateZSetKeys(yajl_val &keys,std::string &desc)
{
	if ( !keys ) {
		desc = QKPACK_ERROR_ZSET_KEYS_NOT_EXIST;
		return QKPACK_ERROR;
	}
	
	if ( !YAJL_IS_ARRAY(keys) || !YAJL_GET_ARRAY(keys)->len ) {
		desc = QKPACK_ERROR_ZSET_KEYS_NOT_EMPTY;
		return QKPACK_ERROR;
	}
	
	return QKPACK_OK;
}


/**
 * 
 * Validate ZSET Interface MEMBERS Info
 * 
 * 1. member,value,score not exits
 * 2. member,value not empty
*/
int QkPackYajlValidate::ValidateZSetMembers(yajl_val &member, yajl_val &value, yajl_val &score, std::string &desc)
{
	int									rc;
	
	if ( !member ) {
		desc = QKPACK_ERROR_ZSET_MEMBERS_MEMBER_NOT_EXIST;
		return QKPACK_ERROR;
	}
		
	rc = ValidateStrEmpty(member);
	if ( rc != QKPACK_OK ) {
		desc = QKPACK_ERROR_ZSET_MEMBERS_MEMBER_NOT_EMPTY;
		return QKPACK_ERROR;
	}
	
	if ( !value ) {
		desc = QKPACK_ERROR_ZSET_MEMBERS_VALUE_NOT_EXIST;
		return QKPACK_ERROR;
	}
		
	rc = ValidateStrEmpty(value);
	if ( rc != QKPACK_OK ) {
		desc = QKPACK_ERROR_ZSET_MEMBERS_VALUE_NOT_EMPTY;
		return QKPACK_ERROR;
	}
	
	if ( !score ) {
		desc = QKPACK_ERROR_ZSET_MEMBERS_SCORE_NOT_EXIST;
		return QKPACK_ERROR;
	}
		
	return QKPACK_OK;
}


/**
 * 
 * Validate ZSET Interface QUERY Info
 * 
 * 1. key,min,max not exits
 * 2. key  not empty
*/
int QkPackYajlValidate::ValidateZSetQuery(yajl_val &key, yajl_val &min, yajl_val &max, std::string &desc)
{
	int									rc;
	
	if ( !key ) {
		desc = QKPACK_ERROR_ZSET_QUERY_KEY_NOT_EXIST;
		return QKPACK_ERROR;
	}
	
	rc = ValidateStrEmpty(key);
	if ( rc != QKPACK_OK ) {
		desc = QKPACK_ERROR_ZSET_QUERY_KEY_NOT_EMPTY;
		return QKPACK_ERROR;
	}
	
	if ( !min ) {
		desc = QKPACK_ERROR_ZSET_QUERY_MIN_NOT_EXIST;
		return QKPACK_ERROR;
	}
		
	if ( !max ) {
		desc = QKPACK_ERROR_ZSET_QUERY_MAX_NOT_EXIST;
		return QKPACK_ERROR;
	}
	
	return QKPACK_OK;
}
