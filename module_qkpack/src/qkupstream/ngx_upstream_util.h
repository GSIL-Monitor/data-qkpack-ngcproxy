/*
 * Author: lihang <lihang01@youku.com>
 *
 * Create Date: 2015-08
 *
 */

#ifndef NGX_UPSTREAM_UTIL_H
#define NGX_UPSTREAM_UTIL_H

#include "ngx_http_qkpack.h"

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

#define NGX_DELAY_DELETE 100 * 1000

typedef struct {
    ngx_event_t                              delay_delete_ev;

    time_t                                   start_sec;
    ngx_msec_t                               start_msec;

    void                                    *data;
} ngx_delay_event_t;


ngx_int_t ngx_http_qkupstream_add_server(ngx_http_request_t *r,
		ngx_str_t upstream_name, ngx_str_t upstream_ip);

ngx_int_t ngx_http_qkupstream_add_peer(ngx_http_request_t *r, 
		ngx_str_t upstream_name, ngx_str_t upstream_ip);
		
ngx_http_upstream_rr_peer_t *ngx_http_qkupstream_get_peers(ngx_http_request_t *r,
		ngx_str_t upstream_name,ngx_str_t upstream_ip);


char * ngx_http_qkupstream_set_complex_value_slot(ngx_conf_t *cf, 
		ngx_command_t *cmd, void *conf);

ngx_http_upstream_srv_conf_t * ngx_http_qkupstream_upstream_add(
		ngx_http_request_t *r, ngx_url_t *url);

	
ngx_http_variable_value_t * ngx_http_qkupstream_get_upstream_ip_name(ngx_http_request_t *r);	
ngx_http_variable_value_t * ngx_http_qkupstream_get_upstream_name(ngx_http_request_t *r);

		
#endif /* NGX_UPSTREAM_UTIL_H */
