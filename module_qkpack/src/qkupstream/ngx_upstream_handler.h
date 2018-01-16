/*
 * Author: lihang <lihang01@youku.com>
 *
 * Create Date: 2015-08
 *
 */

#ifndef NGX_UPSTREAM_HANDLER_H
#define NGX_UPSTREAM_HANDLER_H

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


ngx_int_t ngx_http_qkupstream_handler(ngx_http_request_t *r);


#endif /* NGX_UPSTREAM_HANDLER_H */
