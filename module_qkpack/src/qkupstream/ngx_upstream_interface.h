/*
 * Author: lihang <lihang01@youku.com>
 *
 * Create Date: 2015-08
 *
 */

#ifndef NGX_UPSTREAM_INTERFACE_H
#define NGX_UPSTREAM_INTERFACE_H


#if __cplusplus
extern "C" {
#endif

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include "ngx_upstream_util.h"

ngx_int_t ngx_http_qkupstream_upstream_peer_set(ngx_http_request_t *r, ngx_http_upstream_rr_peer_t *peer);
ngx_http_upstream_rr_peer_t * ngx_http_qkupstream_upstream_peer_get(ngx_http_request_t *r);

ngx_int_t ngx_http_qkupstream_init(ngx_http_request_t *r);
ngx_int_t ngx_http_qkupstream_destroy(ngx_http_request_t *r);

ngx_int_t ngx_http_qkupstream_process_request(ngx_http_request_t *r);
ngx_int_t ngx_http_qkupstream_process_response(ngx_http_request_t *r,ngx_buf_t * upstream_buf);

ngx_int_t ngx_http_qkupstream_process_reinit_request(ngx_http_request_t *r);

#if __cplusplus
}
#endif


#endif //NGX_UPSTREAM_INTERFACE_H
