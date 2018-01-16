/*
 * Author: lihang <lihang01@youku.com>
 *
 * Create Date: 2015-08
 *
 */

#include "ngx_subrequest_interface.h"

#include "ngx_http_qkpack.h"
#include "ngx_qkplugin_manager.h"
#include "plugin.h"
#include "common.h"


ngx_int_t ngx_http_qksubrequest_start(ngx_http_request_t *r);
void ngx_subrequest_post_body(ngx_http_request_t *r);

ngx_int_t subrequest_post_handler(ngx_http_request_t *r, void *data, ngx_int_t rc);


ngx_int_t
ngx_http_qksubrequest_init(ngx_http_request_t *r)
{
	ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0, 
            "[qksubrequest] ngx_http_qksubrequest_init");
    
	if( r->method & NGX_HTTP_POST ) {
        return ngx_http_read_client_request_body(r, ngx_subrequest_post_body);
    }
	
	ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, 
            "[qksubrequest] only POST request accepted");

	return NGX_ERROR;
}


ngx_int_t 
ngx_http_qksubrequest_init_plugin(ngx_http_request_t *r)
{
    ngx_http_qkpack_ctx_t 				*ctx;
	ngx_http_variable_value_t   		*vv;
	
	ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0, 
            "[qksubrequest] ngx_http_qksubrequest_init_plugin");
	
	ctx = (ngx_http_qkpack_ctx_t*)ngx_http_get_module_ctx(r, ngx_http_qksubrequest_module);
	if ( ctx == NULL ) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, 
                    "[qksubrequest] ngx_http_qksubrequest_init_plugin ctx is null");
					
		return NGX_ERROR;
	}
	
	vv = ngx_http_get_plugin(r);
    if ( vv == NULL ) {

    	ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, 
                    "[qksubrequest] ngx_http_qksubrequest_init_plugin plugin_name variable is not found!");
    
    	return NGX_ERROR;
    }
	
    ctx->plugin = plugin_getbyname((char*)vv->data, vv->len);
    if(ctx->plugin == NULL) {

	    ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, 
                "[qksubrequest] ngx_http_qksubrequest_init_plugin plugin handler is null_ptr!");

	    return NGX_ERROR;
    }
	
	if ( ctx->data ) {
		return NGX_ERROR;
	}
	
	ctx->data = new qkpack_request_t;


	return NGX_OK;
}


ngx_int_t
ngx_http_qksubrequest_destroy(ngx_http_request_t	*r)
{
	ngx_http_qkpack_ctx_t 				*ctx;
	CPlugin 							*plugin;
	qkpack_request_t 					*request;
	
	ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
			"[qksubrequest] ngx_qkpack_destroy_crequest");
	
	ctx = (ngx_http_qkpack_ctx_t*)ngx_http_get_module_ctx(r, ngx_http_qksubrequest_module);
	if ( ctx == NULL ) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, 
                    "[qksubrequest] ngx_qkpack_destroy_crequest ctx is null");
					
		return NGX_ERROR;
	}
	
	plugin = (CPlugin *)ctx->plugin;
	if ( plugin == NULL ) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                    "[qksubrequest] ngx_qkpack_destroy_crequest plugin is null");

		return NGX_ERROR;
	}
	
	request = (qkpack_request_t*)ctx->data;
	if ( request == NULL ) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
				"[qksubrequest] ngx_qkpack_destroy_crequest request is null");

        return NGX_ERROR;
	}
	
	plugin->Destroy(request);
	
	if ( request ) {
		delete request;
		request = NULL;
	}
	
	return NGX_OK;
}


ngx_int_t
ngx_http_qksubrequest_process_request(ngx_http_request_t *r) 
{
    ngx_int_t 							rc;
	ngx_http_qkpack_ctx_t 				*ctx;
	CPlugin 							*plugin;
	qkpack_request_t					*request;
	ngx_http_variable_value_t   		*vv;
	
	ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
				"[qksubrequest] ngx_http_qksubrequest_process_request");
	
	ctx = (ngx_http_qkpack_ctx_t*)ngx_http_get_module_ctx(r, ngx_http_qksubrequest_module);
	if ( ctx == NULL ) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, 
                "[qksubrequest] ngx_http_qksubrequest_process_request ngx_http_qkpack_ctx_t is null");
		
		return NGX_ERROR;
	}
	
	rc = ngx_http_qksubrequest_init_plugin(r); 
	if ( rc != NGX_OK ) {
		return NGX_ERROR;
	}
	
	plugin = (CPlugin *)ctx->plugin;
	if ( plugin == NULL ) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
				"[qksubrequest] ngx_http_qksubrequest_process_request plugin is null");

        return NGX_ERROR;
	}
	
	request = (qkpack_request_t*)ctx->data;
	if ( request == NULL ) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
				"[qksubrequest] ngx_http_qksubrequest_process_request request is null");

        return NGX_ERROR;
	}

	rc = ngx_http_get_post_body(r,request);
	if ( rc != NGX_OK ) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
				"[qksubrequest] ngx_http_qksubrequest_process_request reuqest_buffer is null");

		return NGX_ERROR;
	}
	
	vv = ngx_http_get_server(r);
    if ( vv == NULL ) {

    	ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, 
                    "[qksubrequest] ngx_http_get_server server_addr variable is not found!");
    
    	return NGX_ERROR;
    }

	request->request_uri = std::string((char*)ctx->request->uri.data, ctx->request->uri.len);
	request->pid = ngx_getpid();
	request->host = std::string((char*)vv->data,vv->len);
	
	//get x_real_ip
	ngx_table_elt_t *x_real_ip = r->headers_in.x_real_ip; 
	if(x_real_ip) { 
		request->x_real_ip = std::string((char *)x_real_ip->value.data,x_real_ip->value.len); 
	}else {
		request->x_real_ip = std::string((char *)r->connection->addr_text.data,
				r->connection->addr_text.len);
	}
	
	rc = plugin->CreateRequest(request);
    if(rc != NGX_AGAIN) {
		
		ctx->response_buffer.data = (u_char*)request->response_buffer.c_str();
		ctx->response_buffer.len  = request->response_buffer.length();
		
		return NGX_OK;
	}
	
	rc = ngx_http_qksubrequest_start(r);
	if(rc != NGX_OK) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
				"[qksubrequest] plugin start subrequest error");
		
		return NGX_ERROR;
	}

	return NGX_AGAIN;  
}


ngx_int_t 
ngx_http_qksubrequest_check(ngx_http_request_t *r) 
{
    subrequest_t            			*st;
    ngx_http_qkpack_ctx_t 				*ctx;
	ngx_http_upstream_t     			*up;
	size_t								n;
	qkpack_request_t					*request;
    
	ctx = (ngx_http_qkpack_ctx_t*)ngx_http_get_module_ctx(r, ngx_http_qksubrequest_module);
	if ( ctx == NULL ) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, 
                "[qksubrequest] ngx_http_qksubrequest_check ngx_http_qkpack_ctx_t is null");
		
		return NGX_ERROR;
	}

	request = (qkpack_request_t*)ctx->data;
	if ( request == NULL ) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
				"[qksubrequest] ngx_http_qksubrequest_process_request request is null");

        return NGX_ERROR;
	}

	ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0, 
            "[qksubrequest] plugin check subrequest, count = %d", r->main->count);

    n = ctx->subrequests->nelts;
	st = (subrequest_t *)ctx->subrequests->elts; 
    std::vector<qkpack_request_t>::iterator it = request->subrequest.begin();
    for(size_t i = 0; i < n; i++, st++, it++) {
        
        if(st->subr->done != 1) {
            return NGX_AGAIN;
        } 
		
		ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0, 
				"[qksubrequest] all subrequest done");
		
		up = st->subr->upstream;
        if ( up == NULL || up->state == NULL ) {
            ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, 
                    "[subrequest] plugin subrequest upstream null, location not found ?");

			it->status = st->subr->headers_out.status;
            return NGX_ERROR;
        }
		else {

			if ( up->state->status != NGX_HTTP_OK ) {
				request->cost = 4000;
			}
			//it->sec = up->state->response_sec;
			//it->usec = up->state->response_msec;
			//it->response_buffer = std::string((char *)up->buffer.pos, up->buffer.last - up->buffer.pos);  
		}	
	}
			
    return NGX_OK;
}


ngx_int_t 
ngx_http_qksubrequest_post(ngx_http_request_t *r) 
{
    
	ngx_int_t 							rc;
	ngx_http_qkpack_ctx_t 				*ctx;
	CPlugin 							*plugin;
	qkpack_request_t					*request;
	
	ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0, 
            "[qksubrequest] ngx_http_qksubrequest_post");
	
	ctx = (ngx_http_qkpack_ctx_t*)ngx_http_get_module_ctx(r, ngx_http_qksubrequest_module);
	if ( ctx == NULL ) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, 
                "[qksubrequest] ngx_http_qksubrequest_post ngx_http_qkpack_ctx_t is null");
		
		return NGX_ERROR;
	}
	
    plugin = (CPlugin *)ctx->plugin;
	if ( plugin == NULL ) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, 
                "[qksubrequest] ngx_http_qksubrequest_post plugin is null");
		
		return NGX_ERROR;
	} 
 
	request = (qkpack_request_t*)ctx->data;
	if ( request == NULL ) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
				"[qksubrequest] ngx_http_qksubrequest_post request is null");

        return NGX_ERROR;
	}
    
	request->response_buffer = std::string((char*)ctx->response_buffer.data, ctx->response_buffer.len);
	
	rc = plugin->ProcessBody(request);
    if( rc != NGX_AGAIN ) {
		
		ctx->response_buffer.data = (u_char*)request->response_buffer.c_str();
		ctx->response_buffer.len  = request->response_buffer.length();
		
		return NGX_OK;
	}
    
	rc = ngx_http_qksubrequest_start(r);
	if(rc != NGX_OK) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
				"[qksubrequest] plugin start subrequest error");
		
		return NGX_ERROR;
	}

	return NGX_AGAIN;
}

ngx_int_t 
ngx_http_qksubrequest_output(ngx_http_request_t *r)
{
	
	ngx_int_t 							rc;
	ngx_str_t 							type = ngx_string("text/plain");
	ngx_buf_t							*buf;
	ngx_chain_t							*chain;
	ngx_http_qkpack_ctx_t 				*ctx;
	
	ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0, 
		"[qksubrequest] ngx_http_qksubrequest_output");
	
	ctx = (ngx_http_qkpack_ctx_t*)ngx_http_get_module_ctx(r, ngx_http_qksubrequest_module);
	if ( ctx == NULL ) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, 
                "[qksubrequest] ngx_http_qksubrequest_output ngx_http_qkpack_ctx_t is null");
		
		return NGX_ERROR;
	}
	
	
	r->headers_out.content_type = type;
	r->headers_out.content_type_len = type.len;
	r->headers_out.content_length_n = ctx->response_buffer.len;
	r->headers_out.status = NGX_HTTP_OK;

	rc = ngx_http_send_header(r);
	if ( rc == NGX_ERROR || rc > NGX_OK || r->header_only ) {
		return rc;
	}
	
	chain = ngx_alloc_chain_link(r->pool);
	if(chain == NULL){
		return NULL;
	}

	if ( ctx->response_buffer.len == 0 ) {
		buf = (ngx_buf_t *)ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
		if ( buf == NULL ) {
			return NGX_ERROR;
		}
	}
	else {

		buf = ngx_create_temp_buf(r->pool, ctx->response_buffer.len);
		if (buf == NULL) {
			return NGX_ERROR;
		}
		ngx_memcpy(buf->pos, ctx->response_buffer.data, ctx->response_buffer.len);
		buf->last = buf->pos + ctx->response_buffer.len;
	}

    buf->last_buf = 1;
    buf->last_in_chain = 1;

    chain->buf = buf;
    chain->next = NULL;

	return ngx_http_output_filter(r, chain);
}


ngx_int_t 
ngx_http_qksubrequest_done(ngx_http_request_t *r) 
{
    ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0, 
            "[qksubrequest] done request, count = %d", r->main->count);

    return ngx_http_output_filter(r, NULL);
}


/*---------------------------- local function --------------------------------*/
ngx_int_t 
ngx_http_qksubrequest_start(ngx_http_request_t *r) 
{
	int 								flags;
	size_t 								n;
	ngx_int_t 							rc;
	subrequest_t 						*st;
	ngx_http_qkpack_ctx_t 				*ctx;
    ngx_http_post_subrequest_t 			*psr;
	qkpack_request_t 					*request;
	
	ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
			"[qksubrequest] ngx_http_qksubrequest_start");

    ctx = (ngx_http_qkpack_ctx_t *)ngx_http_get_module_ctx(r, ngx_http_qksubrequest_module);
    if ( ctx == NULL ) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
				"[qksubrequest] ngx_http_qksubrequest_start ctx is null");

        return NGX_ERROR;
	}
	
	request = (qkpack_request_t*)ctx->data;
	if ( request == NULL ) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
				"[qksubrequest] ngx_http_qksubrequest_start request is null");

        return NGX_ERROR;
	}
	
    if(ctx->subrequests == NULL ) {
        ctx->subrequests = ngx_array_create(r->pool, 1, sizeof(subrequest_t)); 
    }
	else {
		rc = ngx_array_init(ctx->subrequests, r->pool, 1, sizeof(subrequest_t));
		if(rc != NGX_OK) {
			return NGX_ERROR;
		}	
	}	
    
    n = request->subrequest.size();
	flags = NGX_HTTP_SUBREQUEST_IN_MEMORY | NGX_HTTP_SUBREQUEST_WAITED;
    for(size_t i = 0; i < n; i++) {
		
        qkpack_request_t& ups = request->subrequest[i];
        
        st = (subrequest_t *)ngx_array_push(ctx->subrequests); 
        if(st == NULL) {
            return NGX_ERROR;
        }
		
		st->uri.data = (u_char*)ups.request_uri.c_str();
		st->uri.len = ups.request_uri.length();

		st->args.data = (u_char*)ups.args.c_str();
		st->args.len = ups.args.length();


        psr = (ngx_http_post_subrequest_t *)ngx_palloc(r->pool, sizeof(ngx_http_post_subrequest_t));		
        if(psr == NULL) {
            return NGX_ERROR;
        }

        psr->handler = subrequest_post_handler;
        psr->data = NULL;
		
		rc = ngx_http_subrequest(r, &st->uri, &st->args, &st->subr, psr, flags);
		if(rc != NGX_OK) {
			ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
				"[qksubrequest] subrequest uri=[%s]",st->uri.data);
			return NGX_ERROR;
		}
		
		ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
				"[qksubrequest] subrequest uri=[%s]",st->uri.data);
    }

    return NGX_OK;
}


ngx_int_t
subrequest_post_handler(ngx_http_request_t *r, void *data, ngx_int_t rc) 
{
    ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
            "[qksubrequest] subrequest finish %V?%V", &r->uri, &r->args);

    r->parent->write_event_handler = ngx_http_core_run_phases;

    return NGX_OK;
}


void 
ngx_subrequest_post_body(ngx_http_request_t *r) 
{
	ngx_http_qkpack_ctx_t 				*ctx;
    
    ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
            "[qksubrequest] ngx_subrequest_post_body");
    
	ctx = (ngx_http_qkpack_ctx_t*)ngx_http_get_module_ctx(r, ngx_http_qksubrequest_module);
	if ( ctx == NULL ) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, 
                "[qksubrequest] ngx_subrequest_post_body ngx_http_qkpack_ctx_t is null");
		
		return;
	}

	
	if ( ctx->waiting_more_body ) {
		
		ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
			    "[qksubrequest] ngx_subrequest_post_body waiting_more_body");
		
		ctx->waiting_more_body = 0;
		ngx_http_core_run_phases(r);
		return;
	}
   
	/* read whole request body at first time */
    if( ctx->phase == QKSUBREQUEST_STATE_INIT ) {
		
		ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
			    "[qksubrequest] ngx_subrequest_post_body QKSUBREQUEST_STATE_INIT");
	
		ngx_http_finalize_request(r, NGX_DONE);
        return;
    }

	
    ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
            "[qksubrequest] ngx_subrequest_post_body end");
	
    ctx->phase = QKSUBREQUEST_STATE_PROCESS;
	
	ngx_http_finalize_request(r, r->content_handler(r));
	ngx_http_run_posted_requests(r->connection);
}


