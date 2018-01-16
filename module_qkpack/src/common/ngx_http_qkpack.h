/*
 * Author: lihang <lihang01@youku.com>
 *
 * Create Date: 2015-08
 *
 */

#ifndef __NGX_HTTP_QKPACK_H__
#define __NGX_HTTP_QKPACK_H__

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <sys/time.h>

#define QKUPSTREAM_OK								0
#define QKUPSTREAM_MOVED							2
#define QKUPSTREAM_ASK								3
#define QKUPSTREAM_ASKING							4
#define QKUPSTREAM_ASKING_END						5

#define QKSUBREQUEST_STATE_INIT						0
#define QKSUBREQUEST_STATE_PROCESS					1
#define QKSUBREQUEST_STATE_WAIT_SUBREQUEST			2
#define QKSUBREQUEST_STATE_POST_SUBREQUEST		    3
#define QKSUBREQUEST_STATE_FINAL					4
#define QKSUBREQUEST_STATE_DONE						5
#define QKSUBREQUEST_STATE_ERROR					6


typedef struct {
    ngx_http_upstream_conf_t   upstream;
    ngx_http_complex_value_t   *complex_target;
} ngx_http_qkupstream_loc_conf_t;


typedef struct {
    ngx_str_t           		uri;
    ngx_str_t           		args;
	ngx_str_t 					request_buffer;
	ngx_flag_t					is_subrequest;
    ngx_http_request_t  		*subr;   
} subrequest_t;


typedef struct {
    //subrequest 
    ngx_uint_t			  		phase;
    ngx_array_t         		*subrequests;
	ngx_flag_t					is_subrequest;
	//upstream 
	ngx_flag_t 				  	moved_ask;
	ngx_flag_t                	has_processed;
    ngx_str_t				  	request_buffer;
	ngx_str_t				  	response_buffer;
	ngx_str_t				  	upstream_name;
	ngx_str_t				  	upstream_ip;
	ngx_uint_t					tries;
	ngx_uint_t					waiting_more_body;

	struct timeval      		time_start;
    struct timeval      		time_end;
	
	ngx_http_request_t        	*request;
    void                		*plugin;
	void						*data;
} ngx_http_qkpack_ctx_t;

extern ngx_module_t ngx_http_qkupstream_module;
extern ngx_module_t  ngx_http_qksubrequest_module;

#endif //__NGX_HTTP_QKPACK_H__
