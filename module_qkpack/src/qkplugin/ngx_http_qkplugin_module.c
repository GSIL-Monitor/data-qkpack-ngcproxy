/*
 * Author: lihang <lihang01@youku.com>
 *
 * Create Date: 2015-08
 *
 */

#include "ngx_http_qkplugin_module.h"
#include "ngx_qkplugin_manager.h"

static void *ngx_http_qkplugin_create_main_conf(ngx_conf_t *cf);
static char *ngx_http_qkplugin_set_conf(ngx_conf_t *cf, 
		ngx_command_t *cmd, void *conf); 

static ngx_int_t ngx_http_qkplugin_init_module(ngx_cycle_t *cycle);
static ngx_int_t ngx_http_qkplugin_init_process(ngx_cycle_t *cycle);
static void ngx_http_qkplugin_exit_process(ngx_cycle_t *cycle);
static void ngx_http_qkplugin_exit_master(ngx_cycle_t *cycle);

static ngx_int_t ngx_http_publish_init_process(ngx_cycle_t *cycle); 
static void ngx_http_publish_handler(ngx_event_t *ev); 

static ngx_event_t  publish_event;

static ngx_command_t  ngx_http_qkplugin_commands[] = 
{

    { ngx_string("qkplugin_conf_path"),
        NGX_HTTP_MAIN_CONF | NGX_CONF_TAKE3,
        ngx_http_qkplugin_set_conf,
        0,
        0,
        NULL },

    ngx_null_command
};


static ngx_http_module_t  ngx_http_qkplugin_module_ctx = 
{
    NULL,                                   /* preconfiguration */
    NULL,                                   /* postconfiguration */

    ngx_http_qkplugin_create_main_conf,       /* create main configuration */
    NULL,							         /* init main configuration */

    NULL,                                   /* create server configuration */
    NULL,                                   /* merge server configuration */

    NULL,                                   /* create location configuration */
    NULL                                    /* merge location configuration */
};


ngx_module_t  ngx_http_qkplugin_module = 
{
    NGX_MODULE_V1,
    &ngx_http_qkplugin_module_ctx,            /* module context */
    ngx_http_qkplugin_commands,               /* module directives */
    NGX_HTTP_MODULE,                        /* module type */
    NULL,                                   /* init master */
    ngx_http_qkplugin_init_module,            /* init module */
    ngx_http_qkplugin_init_process,           /* init process */
    NULL,                                   /* init thread */
    NULL,                                   /* exit thread */
    ngx_http_qkplugin_exit_process,           /* exit process */
    ngx_http_qkplugin_exit_master,            /* exit master */
    NGX_MODULE_V1_PADDING
};


static void *
ngx_http_qkplugin_create_main_conf(ngx_conf_t *cf) 
{
    ngx_http_qkplugin_main_conf_t *conf;
    
    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_qkplugin_main_conf_t)); 
    if(conf == NULL) {
        return NGX_CONF_ERROR;
    }

    return conf;
}


static char *
ngx_http_qkplugin_set_conf(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) 
{
    ngx_str_t *value;
    qkplugin_conf_ctx_t *pcf;
    ngx_array_t *qkplugin_conf = &((ngx_http_qkplugin_main_conf_t *)conf)->qkplugin_conf;

    if(qkplugin_conf->elts == NULL) {
        if(ngx_array_init(qkplugin_conf, cf->pool, 1, 
                    sizeof(qkplugin_conf_ctx_t)) != NGX_OK) {
            return NGX_CONF_ERROR;
        }
    }

    pcf = ngx_array_push(qkplugin_conf);
    value = cf->args->elts;

    /* name */
    if(ngx_strncmp(value[1].data, "name=", 5) != 0) {
        ngx_log_error(NGX_LOG_ERR, cf->log, 0, 
                "[qkplugin] invalid qkplugin_conf %V", value + 1);

        return NGX_CONF_ERROR;
    }

    pcf->name.data = value[1].data + 5;
    pcf->name.len = value[1].len - 5;

    /* so_path */
    if(ngx_strncmp(value[2].data, "so_path=", 8) != 0) {
        ngx_log_error(NGX_LOG_ERR, cf->log, 0, 
                "[qkplugin] invalid qkplugin_conf %V", value + 2);

        return NGX_CONF_ERROR;
    }

    pcf->so_path.data = value[2].data + 8;
    pcf->so_path.len = value[2].len - 8;

    /* so_conf */
    if(ngx_strncmp(value[3].data, "so_conf=", 8) != 0) {
        ngx_log_error(NGX_LOG_ERR, cf->log, 0, 
                "[qkplugin] invalid qkplugin_conf %V", value + 3);

        return NGX_CONF_ERROR;
    }

    pcf->so_conf.data = value[3].data + 8;
    pcf->so_conf.len = value[3].len - 8;

    return NGX_CONF_OK; 
}


static ngx_int_t 
ngx_http_qkplugin_init_module(ngx_cycle_t *cycle) 
{
    return NGX_OK;
}


static ngx_int_t 
ngx_http_qkplugin_init_process(ngx_cycle_t *cycle) 
{
	size_t 								i;
	ngx_int_t							rc;
    ngx_http_qkplugin_main_conf_t 		*conf;
    conf = ngx_http_cycle_get_module_main_conf(cycle, ngx_http_qkplugin_module);

    ngx_log_error(NGX_LOG_DEBUG, cycle->log, 0, "[qkplugin] create qkplugin manager start");
	
    qkplugin_conf_ctx_t *ctx = (qkplugin_conf_ctx_t *)conf->qkplugin_conf.elts;
    for(i = 0; i < conf->qkplugin_conf.nelts; ++i) {
        rc = ngx_load_qkplugin(cycle, ctx);
        if( rc != NGX_OK ) {
            return NGX_ERROR;
        }
    }
	
	rc = ngx_init_master_qkplugin();
    if( rc != NGX_OK ) {
        ngx_log_error(NGX_LOG_ERR, cycle->log, 0, "[qkplugin] qkplugin init module fail");
        return NGX_ERROR;   
    }

    ngx_log_error(NGX_LOG_DEBUG, cycle->log, 0, "[qkplugin] create qkplugin manager success");

	rc = ngx_init_process_qkplugin();
    if( rc != NGX_OK ) {
        ngx_log_error(NGX_LOG_ERR, cycle->log, 0, "[qkplugin] qkplugin init process fail");
        return NGX_ERROR;   
    }

	//add timer
	return ngx_http_publish_init_process(cycle);	
}


static ngx_int_t
ngx_http_publish_init_process(ngx_cycle_t *cycle) 
{

	publish_event.handler = ngx_http_publish_handler;
	publish_event.log = cycle->log;
    publish_event.timer_set = 0;

	ngx_add_timer(&publish_event, 60000);

	return NGX_OK;
}


static void
ngx_http_publish_handler(ngx_event_t *ev) 
{
	ngx_int_t			rc;

	rc = ngx_publish_process_qkplugin();	
    if( rc != NGX_OK ) {
        ngx_log_error(NGX_LOG_ERR, ev->log, 0, "[qkplugin] qkplugin publish handler fail");
    }
	
	if ( ngx_exiting ) {
        ngx_log_error(NGX_LOG_ERR, ev->log, 0, "[qkplugin] qkplugin publish handler exiting");
		return;
	}

	ngx_add_timer(ev, 60000);
}



static void 
ngx_http_qkplugin_exit_process(ngx_cycle_t *cycle) 
{
    ngx_log_error(NGX_LOG_ERR, cycle->log, 0, "[qkplugin] exit process");

    ngx_exit_process_qkplugin();
    
	if (publish_event.timer_set) {
        ngx_del_timer(&publish_event);
    }
   
	ngx_exit_master_qkplugin();
}


static void 
ngx_http_qkplugin_exit_master(ngx_cycle_t *cycle) 
{
    ngx_log_error(NGX_LOG_ERR, cycle->log, 0, "[qkplugin] exit master");
}

