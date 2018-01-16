/*
 * Author: lihang <lihang01@youku.com>
 *
 * Create Date: 2015-08
 *
 */

#ifndef __NGX_SUBREQUEST_INTERFACE_H__
#define __NGX_SUBREQUEST_INTERFACE_H__

#if __cplusplus
extern "C" {
#endif

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

ngx_int_t ngx_http_qksubrequest_init(ngx_http_request_t *r);
ngx_int_t ngx_http_qksubrequest_destroy(ngx_http_request_t *r);

ngx_int_t ngx_http_qksubrequest_process_request(ngx_http_request_t *r);
ngx_int_t ngx_http_qksubrequest_check(ngx_http_request_t *r);
ngx_int_t ngx_http_qksubrequest_output(ngx_http_request_t *r);

ngx_int_t ngx_http_qksubrequest_post(ngx_http_request_t *r);
ngx_int_t ngx_http_qksubrequest_done(ngx_http_request_t *r);


#if __cplusplus
}
#endif

#endif  
