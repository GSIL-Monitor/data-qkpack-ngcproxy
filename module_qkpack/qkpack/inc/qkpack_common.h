#ifndef __QKPACK_COMMON_H__
#define __QKPACK_COMMON_H__

#include "common.h"

/******************************************
	*状态
	*
******************************************/

#define QKPACK_HTTP_OK														200

#define QKPACK_OK															0
#define QKPACK_ERROR						   							    -1
#define QKPACK_AGAIN						   							    -2
#define QKPACK_UPSTREAM_INVALID_HEADER     								    40

#define REDIS_OK															0
#define REDIS_ERROR														    -1
#define REDIS_AGAIN														    -2
#define REDIS_MOVED															2
#define REDIS_ASK															3
#define REDIS_ASKING														4
#define REDIS_ASKING_END													5
#define REDIS_TRYAGAIN														6
#define REDIS_CLUSTER														40


#define INT_LEN																11
#define LONG_LEN															21


/******************************************
	*读写
	*
******************************************/

#define QKPACK_OPERATION_READ												1
#define QKPACK_OPERATION_WRITE												2


/******************************************
	*cluster nodes
	*
******************************************/

#define CLUSTER_NODES_ID													0
#define CLUSTER_NODES_ADDR													1
#define CLUSTER_NODES_ROLE													2
#define CLUSTER_NODES_SID													3
#define CLUSTER_NODES_CONNECTED												7
#define CLUSTER_NODES_LENGTH												8
#define CLUSTER_NODES_SLAVE_LEN												7


/******************************************
	*全局配置文件
	*
******************************************/

#define QKPACK_CONF_ACL_URL													"aclUrl"
#define QKPACK_CONF_TIMEOUT													"timeout"
#define QKPACK_CONF_ADD_ZSET_SCRIPT											"addZSetScript"
#define QKPACK_CONF_ADD_MULTIKV_SCRIPT										"addMultiKVScript"
#define QKPACK_CONF_INCRBY_SCRIPT											"incrByScript"
#define QKPACK_CONF_ADD_RSET_SCRIPT											"addRSetScript"


/******************************************
	*ACL配置
	*
******************************************/
#define QKPACK_ACL_URI_FORMAT												"qkd://"
#define QKPACK_ACL_URI_ID													"uri_id"
#define QKPACK_ACL_OPERATION												"operation"
#define QKPACK_ACL_QUOTATION												"quotation"
#define QKPACK_ACL_QUOTATION_EXPIRE											"expire"
#define QKPACK_ACL_QUOTATION_COMMPRESSTHRESHOLD 							"compressThreshold"
#define QKPACK_ACL_QUOTATION_LIMITPOLICIES									"limitPolicies"
#define QKPACK_ACL_QUOTATION_LIMITPOLICIES_MIN								"min"
#define QKPACK_ACL_QUOTATION_LIMITPOLICIES_MAX								"max"
#define QKPACK_ACL_QUOTATION_LIMITPOLICIES_LIMIT							"limit"
#define QKPACK_ACL_QUOTATION_TIMEOUTS										"timeouts"
#define QKPACK_ACL_QUOTATION_TIMEOUTS_INTERFACENAME							"interfaceName"
#define QKPACK_ACL_QUOTATION_TIMEOUTS_TIMEOUT								"timeout"
#define QKPACK_ACL_QUOTATION_TIMEOUTS_TRIES									"tries"

#define QKPACK_ACL_QUOTATION_LIMITSLENGTH									"limitsLength"
#define QKPACK_ACL_QUOTATION_LIMITSLENGTH_INTERFACENAME						"interfaceName"
#define QKPACK_ACL_QUOTATION_LIMITSLENGTH_KEYLEN							"keylen"
#define QKPACK_ACL_QUOTATION_LIMITSLENGTH_VALLEN							"vallen"
#define QKPACK_ACL_QUOTATION_LIMITSLENGTH_DATALEN							"datalen"


#define QKPACK_ACL_ROUTE													"route"
#define QKPACK_ACL_ROUTE_DEFAULT											"default"
#define QKPACK_ACL_ROUTE_CLIENT												"client"
#define QKPACK_ACL_ROUTE_SERVER												"server"

#define QKPACK_ACL_NAMESPACE_LENGTH											2

/******************************************
	*JSON配置
	*
******************************************/

#define QKPACK_JSON_URI														"uri"
#define QKPACK_JSON_AK														"ak"
#define QKPACK_JSON_KEY														"k"
#define QKPACK_JSON_VALUE													"v"
#define QKPACK_JSON_KEYS													"ks"
#define QKPACK_JSON_MBS														"mbs"
#define QKPACK_JSON_MB														"mb"
#define QKPACK_JSON_SC														"sc"
#define QKPACK_JSON_V														"v"
#define QKPACK_JSON_MIN														"min"
#define QKPACK_JSON_MAX														"max"
#define QKPACK_JSON_WS														"ws"
#define QKPACK_JSON_ASC														"asc"
#define QKPACK_JSON_DATA													"data"
#define QKPACK_JSON_CODE													"code"
#define QKPACK_JSON_DESC													"desc"
#define QKPACK_JSON_E														"e" 
#define QKPACK_JSON_COST													"cost"
#define QKPACK_JSON_MC														"mc"
#define QKPACK_JSON_KC														"kc"



/******************************************
	*redis命令名称
	*
******************************************/

#define REDIS_PING_COMMAND													"ping"
#define REDIS_SET_COMMAND													"SETEX"
#define REDIS_GET_COMMAND													"GET"
#define REDIS_TTL_COMMAND													"TTL"
#define REDIS_DEL_COMMAND													"DEL"
#define REDIS_INCR_COMMAND													"INCR"
#define REDIS_INCRBY_COMMAND												"INCRBY"
#define REDIS_MGET_COMMAND													"MGET"
#define REDIS_SADD_COMMAND													"SADD"
#define REDIS_SREM_COMMAND													"SREM"
#define REDIS_SCARD_COMMAND													"SCARD"
#define REDIS_SMEMBERS_COMMAND												"SMEMBERS"
#define REDIS_SISMEMBER_COMMAND												"SISMEMBER"
#define REDIS_ZRANGE_COMMAND												"ZRANGE"
#define REDIS_ZREVRANGE_COMMAND												"ZREVRANGE"
#define REDIS_ZRANGEBYSCORE_COMMAND											"ZRANGEBYSCORE"
#define REDIS_ZREVRANGEBYSCORE_COMMAND										"ZREVRANGEBYSCORE"
#define REDIS_ZRANGE_WITHSCORES_COMMAND										"WITHSCORES"
#define REDIS_SCRIPT_EVALSHA_COMMAND										"EVALSHA"


/******************************************
	*请求URI
	*
******************************************/

#define QKPACK_URI_GET														"/hdp/kvstore/kv/get"
#define QKPACK_URI_SET														"/hdp/kvstore/kv/set"
#define QKPACK_URI_DEL														"/hdp/kvstore/kv/del"
#define QKPACK_URI_TTL														"/hdp/kvstore/kv/ttl"
#define QKPACK_URI_INCR														"/hdp/kvstore/kv/incr"
#define QKPACK_URI_INCRBY													"/hdp/kvstore/kv/incrby"
#define QKPACK_URI_MGET														"/hdp/kvstore/kv/mget"
#define QKPACK_URI_MSET														"/hdp/kvstore/kv/mset"

#define QKPACK_URI_SET_SADD													"/hdp/kvstore/set/sadd"
#define QKPACK_URI_SET_SREM													"/hdp/kvstore/set/srem"
#define QKPACK_URI_SET_SCARD												"/hdp/kvstore/set/scard"
#define QKPACK_URI_SET_SMEMBERS												"/hdp/kvstore/set/smembers"
#define QKPACK_URI_SET_SISMEMBER											"/hdp/kvstore/set/sismember"

#define QKPACK_URI_ZFIXEDSET_ADD											"/hdp/kvstore/zfixedset/add"
#define QKPACK_URI_ZFIXEDSET_BATCHADD										"/hdp/kvstore/zfixedset/batchadd"
#define QKPACK_URI_ZFIXEDSET_GETBYSCORE										"/hdp/kvstore/zfixedset/getbyscore"
#define QKPACK_URI_ZFIXEDSET_GETBYRANK										"/hdp/kvstore/zfixedset/getbyrank"
#define QKPACK_URI_ZFIXEDSET_BATCHGETBYSCORE								"/hdp/kvstore/zfixedset/batchgetbyscore"


/******************************************
	*子请求
	*
******************************************/

#define QKPACK_SUBREQUEST_START												0
#define QKPACK_SUBREQUEST_MGET												1
#define QKPACK_SUBREQUEST_MSET												2
#define QKPACK_SUBREQUEST_ZFIXEDSET_ADD										3
#define QKPACK_SUBREQUEST_ZFIXEDSET_BATCHADD								4
#define QKPACK_SUBREQUEST_ZFIXEDSET_GETBYSCORE								5
#define QKPACK_SUBREQUEST_ZFIXEDSET_BATCHGETBYSCORE							6
#define QKPACK_SUBREQUEST_DONE												7



/*****************************************
	 * 错误码
	 * 
*****************************************/

#define QKPACK_RESPONSE_CODE_OK												0
#define QKPACK_RESPONSE_CODE_UNKNOWN										1000
#define QKPACK_RESPONSE_CODE_PARSE_ERROR									2000
#define QKPACK_RESPONSE_CODE_ILLEGAL_RIGHT									3000
#define QKPACK_RESPONSE_CODE_5XX_TIMEOUT									4000
#define QKPACK_RESPONSE_CODE_REDIS_STORAGE_ERROR							5000

/*****************************************
	 * 错误提示
	 * 
*****************************************/
#define QKPACK_ERROR_JSON_FORMAT											"json format is illegal"

#define QKPACK_ERROR_INTERFACE_OPERATION_READ								"interface operation to read"
#define QKPACK_ERROR_INTERFACE_OPERATION_WRITE								"interface operation to write"


#define QKPACK_ERROR_KVPAIR_KEY_NOT_EXIST									"kvpair key not exist"
#define QKPACK_ERROR_KVPAIR_KEY_NOT_EMPTY									"kvpair key may not be empty"
#define QKPACK_ERROR_KVPAIR_VALUE_NOT_EXIST									"kvpair value not exist"
#define QKPACK_ERROR_KVPAIR_VALUE_NOT_EMPTY									"kvpair value may not be empty"
#define QKPACK_ERROR_KVPAIR_KEYS_NOT_EXIST									"kvpair ks not exist"
#define QKPACK_ERROR_KVPAIR_KEYS_NOT_EMPTY									"kvpair ks may not be empty"
#define QKPACK_ERROR_KVPAIR_NUMERIC_VALUE									"kvpair value must be a number"

#define  QKPACK_ERROR_TTL_KEY_NO_EXPIRE										"kvpair the key exists but has no associated expire"
#define  QKPACK_ERROR_TTL_KEY_NOT_EXIST										"kvpair the key does not exist"


#define QKPACK_ERROR_SET_KEY_NOT_EXIST										"set key not exist"
#define QKPACK_ERROR_SET_KEY_NOT_EMPTY										"set key may not be empty"
#define QKPACK_ERROR_SET_VALUE_NOT_EXIST									"set value not exist"
#define QKPACK_ERROR_SET_VALUE_NOT_EMPTY									"set value may not be empty"
#define QKPACK_ERROR_SET_MEMBERS_NOT_EXIST									"set mbs not exist"
#define QKPACK_ERROR_SET_MEMBERS_NOT_EMPTY									"set mbs may not be empty"
#define QKPACK_ERROR_SET_MEMBERS_VALUE_NOT_EMPTY							"set mbs value may not be empty"


#define QKPACK_ERROR_ZSET_KEY_NOT_EXIST										"zset key not exist"
#define QKPACK_ERROR_ZSET_KEY_NOT_EMPTY										"zset key may not be empty"
#define QKPACK_ERROR_ZSET_MEMBERS_NOT_EXIST									"zset mbs not exist"
#define QKPACK_ERROR_ZSET_MEMBERS_NOT_EMPTY									"zset mbs may not be empty"
#define QKPACK_ERROR_ZSET_MEMBERS_MEMBER_NOT_EXIST 							"zset member not exist"
#define QKPACK_ERROR_ZSET_MEMBERS_MEMBER_NOT_EMPTY							"zset member may not be empty"
#define QKPACK_ERROR_ZSET_MEMBERS_SCORE_NOT_EXIST							"zset score not exist"
#define QKPACK_ERROR_ZSET_MEMBERS_VALUE_NOT_EXIST							"zset value not exist"
#define QKPACK_ERROR_ZSET_MEMBERS_VALUE_NOT_EMPTY							"zset value may not be empty"

#define QKPACK_ERROR_ZSET_KEYS_NOT_EXIST									"zset ks not exist"
#define QKPACK_ERROR_ZSET_KEYS_NOT_EMPTY									"zset ks may not be empty"

#define QKPACK_ERROR_ZSET_QUERY_KEY_NOT_EXIST								"zset query key not exist"
#define QKPACK_ERROR_ZSET_QUERY_KEY_NOT_EMPTY								"zset query key may not be empty"
#define QKPACK_ERROR_ZSET_QUERY_MIN_NOT_EXIST								"zset query min not exist"
#define QKPACK_ERROR_ZSET_QUERY_MAX_NOT_EXIST								"zset query max not exist"

#define QKPACK_ERROR_ZSET_QUERIES_NOT_EXIST									"zset queries not exist"
#define QKPACK_ERROR_ZSET_QUERIES_NOT_EMPTY									"zset queries may not be empty"


#define QKPACK_ERROR_ACL_NOT_NULL											"acl obj may not be null"
#define QKPACK_ERROR_ACL_URI_NOT_EXIST										"acl uri not exist"
#define QKPACK_ERROR_ACL_APPKEY_NOT_EXIST									"acl appkey not exist"
#define QKPACK_ERROR_ACL_URI_NOT_EMPTY										"acl uri may not be empty"
#define QKPACK_ERROR_ACL_APPKEY_NOT_EMPTY									"acl appkey may not be empty"
#define QKPACK_ERROR_ACL_NO_AUTH											"acl unauthorized"
#define QKPACK_ERROR_ACL_FORMAT												"acl format illegal"

#define  QKPACK_ERROR_ACL_LIMITPOLICIES										"acl limit member count too big"
#define  QKPACK_ERROR_ACL_LIMITSLENGTH_KEYLEN								"acl limit keylen too big"
#define  QKPACK_ERROR_ACL_LIMITSLENGTH_VALLEN								"acl limit vallen too big"
#define  QKPACK_ERROR_ACL_LIMITSLENGTH_DATALEN								"acl limit datalen too big"
#define  QKPACK_ERROR_ACL_NAMESPACE_NOT_EMPTY								"acl namespace may not be empty"
#define  QKPACK_ERROR_ACL_NAMESPACE_LENGTH									"acl namespace length must be equal to 2"
#define  QKPACK_ERROR_ACL_LIMITPOLICIES_LIMIT								"key length could not match limit policy, keyLength="


#define  QKPACK_ERROR_ACL_ROUTE_NOT_BLANK									"acl route may not be empty"
#define  QKPACK_ERROR_ACL_ROUTE_NOT_MATCH									"acl route must match {regexp}"
#define  QKPACK_ERROR_ACL_COMPRESS_THRESHOLD								"acl compress threshold must be greater than or equal to {value}"


/******************************************
	*场景ID
	*
******************************************/
#define QKPACK_USERSCENE_SYS_ID												 "23"
#define QKPACK_USERSCENE_SCE_ID												 "1533"
#define QKPACK_GLOBALSCENE_G_SYS_ID											 "23"
#define QKPACK_GLOBALSCENE_G_SCE_ID											 "1534"


/******************************************
	*埋点维度
	*
******************************************/

// global dims 
#define QKPACK_METRICS_G_ZRANGE												"ZRANGE"
#define QKPACK_METRICS_G_MGET												"MGET"
// cmd RT
#define QKPACK_METRICS_GET													"get"
#define QKPACK_METRICS_GETMISS												"getMiss"
#define QKPACK_METRICS_SET													"set"
#define QKPACK_METRICS_DEL													"del"
#define QKPACK_METRICS_TTL													"ttl"
#define QKPACK_METRICS_INCR													"incr"
#define QKPACK_METRICS_INCRBY												"incrBy"
#define QKPACK_METRICS_MGET													"mget"
#define QKPACK_METRICS_MGETMISS												"mgetMiss"
#define QKPACK_METRICS_MSET													"mset"
		
#define QKPACK_METRICS_SADD													"sadd"
#define QKPACK_METRICS_SREM													"srem"
#define QKPACK_METRICS_SCARD												"scard"
#define QKPACK_METRICS_SMEMBERS												"smembers"
#define QKPACK_METRICS_SMEMBERSMISS											"smembersMiss"
#define QKPACK_METRICS_SISMEMBER											"sismember"
		
#define QKPACK_METRICS_ZFIXEDSETADD											"zfixedsetAdd"
#define QKPACK_METRICS_ZFIXEDSETBATCHADD									"zfixedsetBatchAdd"
#define QKPACK_METRICS_ZFIXEDSETGETBYSCORE									"zfixedsetGetByScore"
#define QKPACK_METRICS_ZFIXEDSETGETBYSCOREMISS								"zfixedsetGetByScoreMiss"
#define QKPACK_METRICS_ZFIXEDSETGETBYRANK									"zfixedsetGetByRank"
#define QKPACK_METRICS_ZFIXEDSETGETBYRANKMISS								"zfixedsetGetByRankMiss"
#define QKPACK_METRICS_ZFIXEDSETBATCHGETBYSCORE								"zfixedsetBatchGetByScore"
		
#define QKPACK_METRICS_QPS													"QPS"
#define QKPACK_METRICS_TPS													"TPS"
//#define MISS = "MISS";
		// data status
#define QKPACK_METRICS_KEY_LEN												"KEYLEN"
#define QKPACK_METRICS_VAL_LEN												"VALLEN"
		// error code distribution
#define QKPACK_METRICS_JSON_PARSER_ERROR_METER								"E_JPASER"
#define QKPACK_METRICS_ACL_ERROR_METER										"E_ACLER"
#define QKPACK_METRICS_CLUSTER_NODES_ERROR_METER							"E_CLUSERNODESER"
#define QKPACK_METRICS_REDIS_PROTO_ERROR_METER								"E_REDISPROTOER"
#define QKPACK_METRICS_UNKNOWN_ERROR_METER									"E_UNKNOWN"

#define QKPACK_METRICS_REDIS_STORAGE_ERROR_METER							"E_REDISERR"
#define QKPACK_METRICS_5XX_ERROR_METER										"E_5xxER"


/******************************************
	*metrics name
	*
******************************************/
//metrics
#define QKPACK_METRICS_SYSNAME												"sysName"
//meters
#define QKPACK_METRICS_METERS												"meters"
#define QKPACK_METRICS_METERS_NAME											"name"
#define QKPACK_METRICS_METERS_COUNT											"count"
#define QKPACK_METRICS_METERS_M15_RATE										"m15_rate"
#define QKPACK_METRICS_METERS_M1_RATE										"m1_rate"
#define QKPACK_METRICS_METERS_M5_RATE										"m5_rate"
#define QKPACK_METRICS_METERS_MEAN_RATE										"mean_rate"
#define QKPACK_METRICS_METERS_UNITS											"units"
//timers
#define QKPACK_METRICS_TIMERS												"timers"
#define QKPACK_METRICS_TIMERS_NAME											"name"
#define QKPACK_METRICS_TIMERS_COUNT											"count"
#define QKPACK_METRICS_TIMERS_MAX											"max"
#define QKPACK_METRICS_TIMERS_MEAN											"mean"
#define QKPACK_METRICS_TIMERS_MIN											"min"
#define QKPACK_METRICS_TIMERS_P50											"p50"
#define QKPACK_METRICS_TIMERS_P75											"p75"
#define QKPACK_METRICS_TIMERS_P95											"p95"
#define QKPACK_METRICS_TIMERS_P98											"p98"
#define QKPACK_METRICS_TIMERS_P99											"p99"
#define QKPACK_METRICS_TIMERS_P999											"p999"
#define QKPACK_METRICS_TIMERS_STDDEV										"stddev"
#define QKPACK_METRICS_TIMERS_M15_RATE										"m15_rate"
#define QKPACK_METRICS_TIMERS_M1_RATE										"m1_rate"
#define QKPACK_METRICS_TIMERS_M5_RATE										"m5_rate"
#define QKPACK_METRICS_TIMERS_MEAN_RATE										"mean_rate"
#define QKPACK_METRICS_TIMERS_DURATION_UNITS								"duration_units"
#define QKPACK_METRICS_TIMERS_RATE_UNITS									"rate_units"


/******************************************
	*redis命令类型
	*
******************************************/

enum redisCommand {
    REDIS_EXISTS,
    REDIS_PTTL,
    REDIS_TTL,
    REDIS_TYPE,
    REDIS_BITCOUNT,
    REDIS_GET,
    REDIS_GETBIT,
    REDIS_GETRANGE,
    REDIS_GETSET,
    REDIS_STRLEN,
    REDIS_HEXISTS,
    REDIS_HGET,
    REDIS_HGETALL,
    REDIS_HKEYS,
    REDIS_HLEN,
    REDIS_HVALS,
    REDIS_LINDEX,                 /* redis requests - lists */
    REDIS_LLEN,
    REDIS_LRANGE,
    REDIS_SCARD,
    REDIS_SISMEMBER,
    REDIS_SMEMBERS,
    REDIS_SRANDMEMBER,
    REDIS_ZCARD,
    REDIS_ZCOUNT,
    REDIS_ZRANGE,
    REDIS_ZRANGEBYSCORE,
    REDIS_BATCHZRANGEBYSCORE,
	REDIS_ZREVRANGE,
    REDIS_ZREVRANGEBYSCORE,
    REDIS_ZREVRANK,
    REDIS_ZSCORE,

    REDIS_DEL,                    /* redis commands - keys */
    REDIS_EXPIRE,
    REDIS_EXPIREAT,
    REDIS_PEXPIRE,
    REDIS_PEXPIREAT,
    REDIS_PERSIST,
    REDIS_APPEND,                 /* redis requests - string */
    REDIS_DECR,
    REDIS_DECRBY,
    REDIS_INCR,
    REDIS_INCRBY,
    REDIS_INCRBYFLOAT,
    REDIS_PSETEX,
    REDIS_SET,
    REDIS_SETBIT,
    REDIS_SETEX,
    REDIS_SETNX,
    REDIS_SETRANGE,
    REDIS_HDEL,                   /* redis requests - hashes */
    REDIS_HINCRBY,
    REDIS_HINCRBYFLOAT,
    REDIS_HSET,
    REDIS_HSETNX,
    REDIS_LINSERT,
    REDIS_LPOP,
    REDIS_LPUSH,
    REDIS_LPUSHX,
    REDIS_LREM,
    REDIS_LSET,
    REDIS_LTRIM,
    REDIS_RPOP,
    REDIS_RPUSH,
    REDIS_RPUSHX,
    REDIS_SADD,                   /* redis requests - sets */
    REDIS_SPOP,
    REDIS_SREM,
    REDIS_ZADD,                   /* redis requests - sorted sets */
	REDIS_ZBATCHADD,
	REDIS_ZINCRBY,
    REDIS_ZRANK,
	REDIS_ZREM,
    REDIS_ZREMRANGEBYRANK,
    REDIS_ZREMRANGEBYSCORE,
	REDIS_MGET,
	REDIS_MSET,
	REDIS_CLUSTER_NODES,
    REDIS_UNKNOWN
};



#endif  // __QKPACK_COMMON_H__ 

