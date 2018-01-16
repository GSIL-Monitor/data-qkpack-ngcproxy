/*
 * Author: lihang <lihang01@youku.com>
 *
 * Create Date: 2015-08
 *
 */

#include "ngx_upstream_handler.h"

#include "ngx_http_qkpack.h"
#include "ngx_upstream_util.h"
#include "ngx_upstream_interface.h"
#include "ngx_redis_protocol.h"

//upstream phase	
static ngx_int_t ngx_http_qkupstream_create_request(ngx_http_request_t *r);
static ngx_int_t ngx_http_qkupstream_reinit_request(ngx_http_request_t *r);
static ngx_int_t ngx_http_qkupstream_process_header(ngx_http_request_t *r);
static ngx_int_t ngx_http_qkupstream_filter_init(void *data);
static ngx_int_t ngx_http_qkupstream_filter(void *data, ssize_t bytes);
static void ngx_http_qkupstream_abort_request(ngx_http_request_t *r);
static void ngx_http_qkupstream_finalize_request(ngx_http_request_t *r,
    ngx_int_t rc);

	
ngx_int_t
ngx_http_qkupstream_handler(ngx_http_request_t *r)
{
    ngx_int_t                       	rc;
	ngx_str_t                       	target;
    ngx_url_t                       	url;
    ngx_http_upstream_t             	*u;
    ngx_http_qkpack_ctx_t           	*ctx;
    ngx_http_qkupstream_loc_conf_t      *rlcf;
	
	ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
        "[qkupstream] ngx_http_qkupstream_handler");
    
    if (ngx_http_set_content_type(r) != NGX_OK) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    if (ngx_http_upstream_create(r) != NGX_OK) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    u = r->upstream;

    rlcf = ngx_http_get_module_loc_conf(r, ngx_http_qkupstream_module);

    if (rlcf->complex_target) {
        /* variables used in the qkpack_pass directive */

        if (ngx_http_complex_value(r, rlcf->complex_target, &target)
                != NGX_OK) {

            return NGX_ERROR;
        }

        if (target.len == 0) {

            ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
            		"[qkupstream] ngx_http_qkupstream_handler handler empty \"qkpack_pass\" target");

            return NGX_HTTP_INTERNAL_SERVER_ERROR;
        }
		
		ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
				"[qkupstream] upstream  \"%V\" iplist", &target);
		
        url.host = target;
        url.port = 0;
        url.no_resolve = 1;

        rlcf->upstream.upstream = ngx_http_qkupstream_upstream_add(r, &url);

        if (rlcf->upstream.upstream == NULL) {

            ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                   "[qkupstream] ngx_http_qkupstream_handler upstream \"%V\" not found", &target);

            return NGX_HTTP_INTERNAL_SERVER_ERROR;
        }
    }

    ngx_str_set(&u->schema, "qkpack://");
    u->output.tag = (ngx_buf_tag_t) &ngx_http_qkupstream_module;

    u->conf = &rlcf->upstream;
	u->buffering = rlcf->upstream.buffering;
	
	ctx = ngx_pcalloc(r->pool, sizeof(ngx_http_qkpack_ctx_t));
    if (ctx == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
	
    ctx->request = r;
    ctx->has_processed = 0;
    ctx->moved_ask = QKUPSTREAM_OK;
	
    ngx_http_set_ctx(r, ctx, ngx_http_qkupstream_module);
	
	
    u->create_request = ngx_http_qkupstream_create_request;
    u->reinit_request = ngx_http_qkupstream_reinit_request;
    u->process_header = ngx_http_qkupstream_process_header;
    u->abort_request = ngx_http_qkupstream_abort_request;
    u->finalize_request = ngx_http_qkupstream_finalize_request;

	u->input_filter_init = ngx_http_qkupstream_filter_init;
    u->input_filter = ngx_http_qkupstream_filter;
	u->input_filter_ctx = ctx;
	

    rc = ngx_http_read_client_request_body(r, ngx_http_upstream_init);
    if (rc >= NGX_HTTP_SPECIAL_RESPONSE) {
        return rc;
    }
	
    return NGX_DONE;
}


static ngx_int_t
ngx_http_qkupstream_create_request(ngx_http_request_t *r)
{

	ngx_int_t 							rc;
    ngx_chain_t 						*cl;
    ngx_buf_t 							*buf;
	ngx_http_qkpack_ctx_t 				*ctx;
	
	ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
				"[qkupstream] ngx_http_qkupstream_create_request");
	
	ctx = ngx_http_get_module_ctx(r, ngx_http_qkupstream_module);
	if ( ctx == NULL ) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                    "[qkupstream] ngx_http_qkupstream_create_request ctx is null");

		return NGX_ERROR;
	}
	
	rc = ngx_http_qkupstream_init(r);
	if ( rc != NGX_OK ) {
		return NGX_ERROR;
	}
	
	rc= ngx_http_qkupstream_process_request(r);
    if ( rc != NGX_OK ) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
				"[qkupstream] ngx_http_qkupstream_create_request ngx_http_qkupstream_request_body is error");
		
		ngx_http_qkupstream_destroy(r);

        return NGX_ERROR;
    }
	
    /* Create temporary buffer for request with size len. */
    buf = ngx_create_temp_buf(r->pool, ctx->request_buffer.len);
    if (buf == NULL)
    {
        return NGX_ERROR;
    }
	
	ngx_memcpy(buf->pos, ctx->request_buffer.data, ctx->request_buffer.len);
	buf->last = buf->pos + ctx->request_buffer.len;
	
    cl = ngx_alloc_chain_link(r->pool);
    if (cl == NULL) {
        return NGX_ERROR;
    }

    cl->buf = buf;
    cl->next = NULL;
    r->upstream->request_bufs = cl;
	
    return NGX_OK;
}



static ngx_int_t
ngx_http_qkupstream_reinit_request(ngx_http_request_t *r)
{
	ngx_chain_t 						*cl;
    ngx_buf_t 							*buf = NULL;
	ngx_http_qkpack_ctx_t 				*ctx;
	
	ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
			"[qkupstream] ngx_http_qkupstream_reinit_request start");
	
   
	ctx = ngx_http_get_module_ctx(r, ngx_http_qkupstream_module);
	if ( ctx == NULL || ctx->request_buffer.data == NULL || ctx->request_buffer.len == 0 ) {
		return NGX_ERROR;
	}

	if ( ctx->moved_ask == QKUPSTREAM_OK ) {
		return NGX_OK;
	}
	
	if ( ctx->moved_ask == QKUPSTREAM_MOVED ) {
		ctx->moved_ask = QKUPSTREAM_OK; 
		
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
			"[qkupstream] ngx_http_qkupstream_reinit_request QKUPSTREAM_MOVED");
		
		return NGX_OK;
	}
	
	else if ( ctx->moved_ask == QKUPSTREAM_ASKING ) {
		
		ctx->moved_ask = QKUPSTREAM_ASKING_END;
		
		if ( ctx->request_buffer.data == NULL || ctx->request_buffer.len == 0 ) {
			return NGX_ERROR;
		}
		
		buf = ngx_create_temp_buf(r->pool, ctx->request_buffer.len); 
		if (buf == NULL) {
			return NGX_ERROR;
		}
		ngx_memcpy(buf->pos, ctx->request_buffer.data, ctx->request_buffer.len);
		buf->last = buf->pos + ctx->request_buffer.len;
		
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
			"[qkupstream] ngx_http_qkupstream_reinit_request QKUPSTREAM_ASKING buffer is %s ",buf->pos);
		
	}
	
	if ( buf == NULL ) {
		return NGX_ERROR;
	}
	
    cl = ngx_alloc_chain_link(r->pool);
    if (cl == NULL) {
        return NGX_ERROR;
    }

    cl->buf = buf;
    cl->next = NULL;
    r->upstream->request_bufs = cl;
	
	ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
			"[qkupstream] ngx_http_qkupstream_reinit_request end");
					   
    return NGX_OK;
}


static ngx_int_t
ngx_http_qkupstream_process_header(ngx_http_request_t *r)
{

	u_char                         		*rdata;
	ngx_int_t							rc;
	ssize_t								len;
	ngx_http_upstream_t         		*u;
	ngx_http_qkpack_ctx_t 				*ctx;	

	ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
			"[qkupstream] upstream_process_header");
			   
    ctx = ngx_http_get_module_ctx(r, ngx_http_qkupstream_module);
	if ( ctx == NULL ) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                    "[qkupstream] ngx_http_qkupstream_process_header ctx is null");

		return NGX_ERROR;
	}
	
	if ( ctx->moved_ask == QKUPSTREAM_ASK  || ctx->moved_ask == QKUPSTREAM_ASKING ) {
		
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                    "[qkupstream] ngx_http_qkupstream_process_header moved_ask=[%d]", ctx->moved_ask);
		
		return NGX_HTTP_UPSTREAM_INVALID_HEADER;
	}
	
	u = r->upstream;
	rdata = u->buffer.pos;	
	len = u->buffer.last - u->buffer.pos;

	if ( ctx->moved_ask == QKUPSTREAM_ASKING_END ) {
		rdata+=5;
		len = len - 5;
	}

	rc = ngx_redis_proto_decode((char*)rdata, len);
	if ( rc != NGX_OK ) {
		ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
                    "[qkupstream] ngx_http_qkupstream_process_header len=[%d], rdata=[%s] NGX_AGAIN", len ,rdata);
		return rc; //NGX_AGAIN
	}

	ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
			"[qkupstream] ngx_http_qkupstream_process_header len=[%d] NGX_OK", len);
	
	if ( ctx->moved_ask == QKUPSTREAM_ASKING_END ) {
		 ctx->response_buffer.len = u->buffer.last - u->buffer.pos - 5;
		 ctx->response_buffer.data = u->buffer.pos + 5;
	}
	else {
		 ctx->response_buffer.len = u->buffer.last - u->buffer.pos;
		 ctx->response_buffer.data = u->buffer.pos;
	}

	rc = ngx_http_qkupstream_process_response(r, &u->buffer);
	ctx->has_processed = 1;
	
	return rc;
}


static ngx_int_t
ngx_http_qkupstream_filter_init(void *data)
{
	ngx_http_qkpack_ctx_t  			*ctx = data;
	ngx_http_upstream_t  			*u = ctx->request->upstream;

	u->length = u->headers_in.content_length_n;
				   
	ngx_log_error(NGX_LOG_DEBUG, ctx->request->connection->log, 0,
			"[qkupstream] ngx_http_redis_filter_init u->length=[%z]", u->length);
	
    return NGX_OK;
}

static ngx_int_t
ngx_http_qkupstream_filter(void *data, ssize_t bytes)
{

	u_char               				*last = NULL;
	ngx_buf_t            				*b = NULL;
	ngx_chain_t          				*cl, **ll;
	ngx_http_upstream_t  				*u = NULL;
	ngx_http_qkpack_ctx_t				*ctx = data;
	
    ngx_log_error(NGX_LOG_DEBUG, ctx->request->connection->log, 0,
    		"[qkupstream] ngx_http_qkupstream_filter bytes:%z", bytes);

    u = ctx->request->upstream;
    b = &u->buffer;
	
    for (cl = u->out_bufs, ll = &u->out_bufs; cl; cl = cl->next) {
        ll = &cl->next;
    }

    cl = ngx_chain_get_free_buf(ctx->request->pool, &u->free_bufs);
    if (cl == NULL) {
        return NGX_ERROR;
    }

    cl->buf->flush = 1;
    cl->buf->memory = 1;

    *ll = cl;

    last = b->last;
    cl->buf->pos = last;
    b->last += bytes;
    cl->buf->last = b->last;
    cl->buf->tag = u->output.tag;

    ngx_log_error(NGX_LOG_DEBUG, ctx->request->connection->log, 0,
    		"[qkupstream] ngx_http_qkupstream_filter filter bytes:%z size:%z length:%z",
    		bytes, b->last - b->pos, u->length);

    u->length -= bytes;

    if(u->length == 0) {
        u->keepalive = 1;
    }
   	
    return NGX_OK;
}

static void
ngx_http_qkupstream_abort_request(ngx_http_request_t *r)
{	
    ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
                   "[qkupstream] upsteam_abort_http_request");

	ngx_http_qkupstream_destroy(r);
				   
    return;
}

static void
ngx_http_qkupstream_finalize_request(ngx_http_request_t *r, ngx_int_t rc)
{
    ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
    		"[qkupstream] ngx_http_qkupstream_finalize_request");

	ngx_http_qkupstream_destroy(r);
	
    return;
}
