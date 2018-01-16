/*
 * Author: lihang <lihang01@youku.com>
 *
 * Create Date: 2015-08
 *
 */

#include "ngx_http_qkpack.h"
#include "ngx_subrequest_interface.h"
#include "ngx_qkplugin_manager.h"


static char *ngx_http_qksubrequest_cb(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static ngx_int_t ngx_http_qksubrequest_handler(ngx_http_request_t *r);


static ngx_command_t  ngx_http_qksubrequest_commands[] = 
{

    { ngx_string("qksubrequest"),
      NGX_HTTP_LOC_CONF | NGX_CONF_NOARGS,
      ngx_http_qksubrequest_cb,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL },

    ngx_null_command
};


static ngx_http_module_t  ngx_http_qksubrequest_module_ctx = 
{
    NULL,                                   /* preconfiguration */
    NULL,                                   /* postconfiguration */

    NULL,                                   /* create main configuration */
    NULL,                                   /* init main configuration */

    NULL,                                   /* create server configuration */
    NULL,                                   /* merge server configuration */

    NULL,    								/* create location configration */
    NULL							       /* merge location configration */
};


ngx_module_t  ngx_http_qksubrequest_module = 
{
    NGX_MODULE_V1,
    &ngx_http_qksubrequest_module_ctx,        /* module context */
    ngx_http_qksubrequest_commands,           /* module directives */
    NGX_HTTP_MODULE,                        /* module type */
    NULL,                                   /* init master */
    NULL,                                   /* init module */
    NULL,                                   /* init process */
    NULL,                                   /* init thread */
    NULL,                                   /* exit thread */
    NULL,                                   /* exit process */
    NULL,                                   /* exit master */
    NGX_MODULE_V1_PADDING
};


static char *ngx_http_qksubrequest_cb(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) 
{
    ngx_http_core_loc_conf_t 			*clcf;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module); 
    clcf->handler = ngx_http_qksubrequest_handler; 

    return NGX_OK;
}


static ngx_int_t ngx_http_qksubrequest_handler(ngx_http_request_t *r) 
{
    ngx_int_t 							rc;
    ngx_http_qkpack_ctx_t 				*ctx;
	
    ctx = ngx_http_get_module_ctx(r, ngx_http_qksubrequest_module);    
    if(ctx == NULL) {
        
		ctx = ngx_pcalloc(r->pool, sizeof(ngx_http_qkpack_ctx_t));
        if(ctx == NULL) {
            return NGX_ERROR;
        }

        ngx_http_set_ctx(r, ctx, ngx_http_qksubrequest_module);

        ctx->phase= QKSUBREQUEST_STATE_INIT;
        gettimeofday(&ctx->time_start, NULL);
		
		ctx->request = r;
		ctx->waiting_more_body = 0;
    }

    if(ctx->phase == QKSUBREQUEST_STATE_INIT) {
		
        rc = ngx_http_qksubrequest_init(r);
		if ( rc >= NGX_HTTP_SPECIAL_RESPONSE )  {
			
			ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0, 
                "[qksubrequest] ngx_http_qksubrequest_init NGX_HTTP_SPECIAL_RESPONSE");
			return rc;
		}

		if ( rc == NGX_AGAIN ) {
			/* 
             * POST body read incompletely
             * Don't need r->main->count++ because ngx_http_read_client_body 
             * has already increased main request count.
             */
			ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0, 
                "[qksubrequest] ngx_http_qksubrequest_init NGX_AGAIN");

			ctx->waiting_more_body = 1;
			return NGX_DONE;
		}

        if( rc == NGX_OK ) {              
            /* GET or POST body read completely */
			ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0, 
                "[qksubrequest] ngx_http_qksubrequest_init NGX_OK");
            ctx->phase = QKSUBREQUEST_STATE_PROCESS;
        } 
		else {
			ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0, 
                "[qksubrequest] ngx_http_qksubrequest_init NGX_ERROR");
			ctx->phase = QKSUBREQUEST_STATE_ERROR;
        }
    }

    if(ctx->phase == QKSUBREQUEST_STATE_PROCESS) {
		
        rc = ngx_http_qksubrequest_process_request(r);
		if ( rc == NGX_AGAIN ) {
			ctx->phase = QKSUBREQUEST_STATE_WAIT_SUBREQUEST;
			
            r->main->count++;
            return NGX_DONE;
		}

        if(rc == NGX_OK) {
            ctx->phase = QKSUBREQUEST_STATE_FINAL;
        } 
		else {
            ctx->phase = QKSUBREQUEST_STATE_ERROR;
        }
    }

    if(ctx->phase == QKSUBREQUEST_STATE_WAIT_SUBREQUEST) {
        
		rc = ngx_http_qksubrequest_check(r);
		if ( rc == NGX_AGAIN ) {
			/* ctx->phase = QKSUBREQUEST_STATE_WAIT_SUBREQUEST; */
            r->main->count++;
            return NGX_DONE;
		}

        if(rc == NGX_OK) {
            ctx->phase = QKSUBREQUEST_STATE_POST_SUBREQUEST;
        } 
		else {
            ctx->phase = QKSUBREQUEST_STATE_ERROR;
        }
    }

    if(ctx->phase == QKSUBREQUEST_STATE_POST_SUBREQUEST) {
        
		rc = ngx_http_qksubrequest_post(r);
		if ( rc == NGX_AGAIN ) {
			ctx->phase = QKSUBREQUEST_STATE_WAIT_SUBREQUEST;
            r->main->count++;
			
            return NGX_DONE;
		}

        if(rc == NGX_OK) {
            ctx->phase = QKSUBREQUEST_STATE_FINAL;
        } 
		else {
            ctx->phase = QKSUBREQUEST_STATE_ERROR;
        }
    }   

    if(ctx->phase == QKSUBREQUEST_STATE_FINAL) {
		
        ctx->phase = QKSUBREQUEST_STATE_DONE; 

        gettimeofday(&ctx->time_end, NULL);
        int ts = ctx->time_end.tv_sec- ctx->time_start.tv_sec;
        int tms = (ctx->time_end.tv_usec - ctx->time_start.tv_usec) / 1000;
        if(tms < 0) {
            ts--;
            tms += 1000;
        }
		
        ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0, 
                "[qksubrequest] final request, time consume: %ds.%dms", ts, tms);

        
		rc = ngx_http_qksubrequest_output(r);
		
		ngx_http_qksubrequest_destroy(r);
		
		return rc;
    }

    if(ctx->phase == QKSUBREQUEST_STATE_DONE) {
        return ngx_http_qksubrequest_done(r); 
    }

    if(ctx->phase == QKSUBREQUEST_STATE_ERROR) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                "[qksubrequest] plugin process error, destory request context");

        /* destroy request context exactly once */
        ngx_http_qksubrequest_destroy(r);

        return NGX_ERROR;
    }

    return NGX_OK;
}
