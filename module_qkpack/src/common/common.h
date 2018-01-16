/*
 * Author: lihang <lihang01@youku.com>
 *
 * Create Date: 2015-08
 *
 */

#ifndef __COMMON_H__
#define __COMMON_H__

#if __cplusplus
extern "C" {
#endif

//#include <ngx_config.h>
//#include <ngx_core.h>
	
#if __cplusplus
}
#endif

#include "plugin.h"
#include <assert.h>
#include <stdio.h>
#include <iostream>
#include <list>
#include <stdint.h>
#include <assert.h>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <dlfcn.h>
#include <tr1/memory>

struct qkpack_conf_t {
	std::string																conf_str;
	std::string																add_zset_script;
	std::string																add_multikv_script;
	std::string																incrby_script;
	std::string																add_rset_script;
	std::string																acl_url;
	int																		timeout;
};


struct zset_member_t {
	std::string 															member;
	std::string																value;
	double																	score;
};

struct zset_query_t {
	double																	min;//最小score，结果集将大于等于此score
	double 																	max;//最大score，结果集将小于等于此score
	bool																	ws;//可选的 withscores 参数决定结果集是只返回有序集的成员，还是将有序集成员及其 score 值一起返回。默认值为false
	bool																	asc;//可选的 asc 参数决定返回的结果集是否要按score正序排序。默认值为false
	zset_query_t(
			bool ws=false,
			bool asc=false):
		ws(ws),
		asc(asc){};
};

struct kvpair_t {
	int																		exptime;
	int																		slot_id;
	int																		num;
	int																		moved_ask;
	int																		command_type;
	int																		operation_type;
	int																		compress_threshold;
	int																		limit;
	std::string																key;
	std::string																value;
	double																	score;
	std::string     														proto_str;
	std::string																command;
	std::string																script_name;
	std::string																metrics_command;
	std::string																namespaces;
	std::string                                     						uri_id;

	kvpair_t(
			int exptime=604800,
			int slot_id=-1,
			int num=0,
			std::string namespaces="null",
			std::string uri_id="null"):
		exptime(exptime),
		slot_id(slot_id),
		num(num),
		namespaces(namespaces),
		uri_id(uri_id){};
};


struct route_t {
	std::string																defaults;
	std::map<std::string,std::string>	   									map_client;
	std::map<std::string,std::string>										map_server;
	bool																	is_match_ip;
};


struct limit_policies_t {
    int             														min;
    int             														max;
    int             														limit;
};


struct timeouts_t {
	std::string																interface_name;
	int																		timeout; //毫秒单位
	int																		tries; //重试次数
};


struct limits_length_t {
	std::string																interface_name;
	int																		keylen;
	int																		vallen;
	int																		datalen;
};

struct quotation_t {
    int                                     								expire;
    int                                     								compress_threshold;
    std::vector<limit_policies_t>    										vector_policies;
	std::vector<timeouts_t>													vector_timeouts;
	std::vector<limits_length_t>											vector_limits_length;
	int																		limit;
};


struct qkpack_acl_t {
	std::string                                     						uri_id;
    int                                     								operation;
    quotation_t                      										quotation;
	route_t																	route;
};


	
struct qkpack_request_t {
	qkpack_request_t														*parent;
	void																	*handler;

	std::vector<qkpack_request_t>		 									vector_sub;
	std::vector<qkpack_request_t>		 									subrequest;
	uint32_t																is_subrequest;
	int																		slot_id;

	std::string																request_uri;
	std::string																args;
	std::string																uri;
	std::string																ak;
	std::string																host;
	std::string																x_real_ip;
	std::string     														request_buffer;
	std::string     														response_buffer;
	std::string																node_ip;
	std::string																cluster_name;
	
    uint32_t																status;
    uint32_t																sec;     
    uint32_t																usec;

	int																		pid;
	struct timeval															tv_begin;
	struct timeval															tv_end;

	uint32_t																timeout;
	uint32_t																tries;
	uint32_t																now_tries;

	int																		code; //错误码
	std::string																desc;//针对错误码的描述
	int																		cost;//服务器的处理耗时，单位毫秒
	int																		phase;

	kvpair_t																kvpair;
	std::map<std::string, std::vector<kvpair_t> >							map_kv;
	std::map<int,std::vector<kvpair_t*> >									map_slots_kv;
	std::vector<kvpair_t>													vector_kv;
	
	std::map<std::string, zset_query_t>										map_query;
	std::vector<zset_member_t> 												vector_mbs;

	qkpack_request_t(
			qkpack_request_t *parent=NULL,
			void *handler=NULL,
			int is_subrequest=0,
			int slot_id=-1,
			std::string ak="null",
			uint32_t timeout=0,
			uint32_t tries=0,
			uint32_t now_tries=0,
			int code=0,
			int cost=0,
			int phase=0):
		parent(parent),
		handler(handler),
		is_subrequest(is_subrequest),
		slot_id(slot_id),
		ak(ak),
		timeout(timeout),
		tries(tries),
		now_tries(now_tries),
		code(code),
		cost(cost),
		phase(phase){};
};



#endif //__COMMON_H__
