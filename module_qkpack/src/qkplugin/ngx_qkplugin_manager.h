/*
 * Author: lihang <lihang01@youku.com>
 *
 * Create Date: 2015-08
 *
 */

#ifndef __NGX_QKPLUGIN_MANAGER_H__
#define __NGX_QKPLUGIN_MANAGER_H__

#if __cplusplus
extern "C" {
#endif

#include <ngx_config.h>
#include <ngx_core.h>

#include "ngx_http_qkplugin_module.h"


typedef struct CPluginInfo {
	
	ngx_str_t   name;
    ngx_str_t   so_path;
    ngx_str_t   so_conf;
	
    void*       qkplugin_ptr;
    void*       so_handler;

}CPluginInfo;



ngx_int_t ngx_load_qkplugin(ngx_cycle_t *cycle, qkplugin_conf_ctx_t *ctx);
ngx_int_t ngx_init_master_qkplugin();
ngx_int_t ngx_init_process_qkplugin();
ngx_int_t ngx_publish_process_qkplugin();
void ngx_exit_process_qkplugin();
void ngx_exit_master_qkplugin();

void * plugin_getbyname(const char *name, size_t len);

ngx_http_variable_value_t *ngx_http_get_plugin(ngx_http_request_t *r);

ngx_http_variable_value_t *ngx_http_get_server(ngx_http_request_t *r);

#if __cplusplus
}
#endif


ngx_int_t ngx_http_get_post_body(ngx_http_request_t *r, void *handler);


#endif // __NGX_QKPLUGIN_MANAGER_H__ 
