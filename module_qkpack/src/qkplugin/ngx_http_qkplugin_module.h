/*
 * Author: lihang <lihang01@youku.com>
 *
 * Create Date: 2015-08
 *
 */

#ifndef __NGX_HTTP_PLUGIN_MODULE_H__
#define __NGX_HTTP_PLUGIN_MODULE_H__

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

typedef struct {
    ngx_str_t   name;
    ngx_str_t   so_path;
    ngx_str_t   so_conf;
} qkplugin_conf_ctx_t;

typedef struct {
    ngx_array_t qkplugin_conf;
} ngx_http_qkplugin_main_conf_t;


#endif
