#ifndef __QKPACK_YAJL_VALIDATE_H__
#define __QKPACK_YAJL_VALIDATE_H__

#include "common.h"
#include "qkpack_common.h"

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


class QkPackYajlValidate
{

public:
    QkPackYajlValidate(){};
	virtual ~QkPackYajlValidate(){};
	
public:
	/**
	* 
	* Validate String Empty
	*
	* 1. string not empty
	* 2. string Space
	* 
	*/
	int ValidateStrEmpty(yajl_val &val);
	
	/**
	 * 
	 * Validate ACL Interface Info
	 *
	 * 1. uri,appkey not exits 
	 * 2. uri,appkey not empty
	 * 
	*/
	int ValidateACLInfo(yajl_val &node, yajl_val &uri, yajl_val &ak, std::string &desc);
	
	/**
	 * 
	 * Validate KVPair Interface KEY Info
	 *
	 * 1. key not exits
	 * 2. key not empty
	 * 
	*/
	int ValidateKVPairByKey(yajl_val &key,std::string &desc);
	
	/**
	 * 
	 * Validate KVPair Interface  KEY/VALUE Info
	 *
	 * 1. key,value not exits
	 * 2. key,value not empty
	 * 
	*/
	int ValidateKVPair(yajl_val &key,yajl_val &value,std::string &desc);
	
	/**
	 * 
	 * Validate MultiKVPair Interface KEYS Info
	 * 
	 * 1. keys not exits
	 * 2. keys not array and not empty list
	*/
	int ValidateMultiKVPair(yajl_val &keys,std::string &desc);
	
	/**
	 * 
	 * Validate Set Interface KEY Info
	 *
	 * 1. key not exits
	 * 2. key not empty
	 * 
	*/
	int ValidateSetByKey(yajl_val &key,std::string &desc);
	
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
	int ValidateSet(yajl_val &key,yajl_val &mbs,std::string &desc);
	
	/**
	 * 
	 * Validate SET Interface  KEY/VALUE Info
	 *
	 * 1. key,value not exits
	 * 2. key,value not empty
	 * 
	*/
	int ValidateSetByKV(yajl_val &key,yajl_val &value,std::string &desc);
	
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
	int ValidateZSet(yajl_val &key,yajl_val &mbs,std::string &desc);
	
	/**
	 * 
	 * Validate ZSet Interface KEYS Info
	 * 
	 * 1. keys not exits
	 * 2. keys not array and not empty list
	*/
	int ValidateZSetKeys(yajl_val &keys,std::string &desc);
	
	/**
	 * 
	 * Validate ZSET Interface MEMBERS Info
	 * 
	 * 1. member,value,score not exits
	 * 2. member,value not empty
	*/
	int ValidateZSetMembers(yajl_val &member, yajl_val &value, yajl_val &score, std::string &desc);
	
	/**
	 * 
	 * Validate ZSET Interface QUERY Info
	 * 
	 * 1. key,min,max not exits
	 * 2. key  not empty
	*/
	int ValidateZSetQuery(yajl_val &key, yajl_val &min, yajl_val &max, std::string &desc);
	
};


} /* namespace parser */
} /* namespace qkpack */
} /* namespace data */
} /* namespace youku */
} /* namespace com */

#endif //__QKPACK_YAJL_VALIDATE_H__

