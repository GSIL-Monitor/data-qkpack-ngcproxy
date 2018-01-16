/*
 * Author: lihang <lihang01@youku.com>
 *
 * Create Date: 2015-08
 *
 */

#include "ngx_upstream_interface.h"

#include "ngx_http_qkpack.h"
#include "ngx_qkplugin_manager.h"
#include "plugin.h"
#include "common.h"

static ngx_uint_t upstream_timeout = 0;
static ngx_uint_t upstream_tries = 0;
static std::map<std::string, std::map<std::string,ngx_http_upstream_rr_peer_t *> > _upstream_peer_map;

ngx_int_t 
ngx_http_qkupstream_upstream_peer_set(ngx_http_request_t *r, ngx_http_upstream_rr_peer_t *peer)
{
	ngx_http_qkpack_ctx_t 				*ctx;
	
	ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
				"[qkupstream] ngx_http_set_upstream_peer_map");
	
	ctx = (ngx_http_qkpack_ctx_t*)ngx_http_get_module_ctx(r, ngx_http_qkupstream_module);
	if ( ctx == NULL ) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                    "[qkupstream] ngx_http_set_upstream_peer_map ctx is null");

		return NGX_ERROR;
	}
	
	if ( ctx->upstream_name.len == 0 || ctx->upstream_ip.len == 0 ) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                    "[qkupstream] ngx_http_set_upstream_peer_map upstream_name|upstream_ip is null");

		return NGX_ERROR;
	}

	std::string upstream_name = std::string((char*)ctx->upstream_name.data,ctx->upstream_name.len);
	std::string upstream_ip = std::string((char*)ctx->upstream_ip.data,ctx->upstream_ip.len);
	
	_upstream_peer_map[upstream_name][upstream_ip] = peer;
	
	ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
                "[qkupstream] ngx_http_set_upstream_peer_map upstream_name=[%V] upstream_ip=[%V]",
				&ctx->upstream_name, &ctx->upstream_ip);

    return NGX_OK;
}


ngx_http_upstream_rr_peer_t * 
ngx_http_qkupstream_upstream_peer_get(ngx_http_request_t *r)
{
	ngx_http_qkpack_ctx_t 				*ctx;
	
	ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
				"[qkupstream] ngx_http_get_upstream_peer_map");
	
	ctx = (ngx_http_qkpack_ctx_t*)ngx_http_get_module_ctx(r, ngx_http_qkupstream_module);
	if ( ctx == NULL ) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                    "[qkupstream] ngx_http_get_upstream_peer_map ctx is null");

		return NULL;
	}
	
	if ( ctx->upstream_name.len == 0 || ctx->upstream_ip.len == 0 ) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                    "[qkupstream] ngx_http_get_upstream_peer_map upstream_name|upstream_ip is null");

		return NULL;
	}
	
	std::string upstream_name = std::string((char*)ctx->upstream_name.data,ctx->upstream_name.len);
	std::string upstream_ip = std::string((char*)ctx->upstream_ip.data,ctx->upstream_ip.len);
	
	ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
                "[qkupstream] ngx_http_get_upstream_peer_map upstream_name=[%V] upstream_ip=[%V] ",
				&ctx->upstream_name, &ctx->upstream_ip);
	
	return _upstream_peer_map[upstream_name][upstream_ip];
}


ngx_int_t
ngx_http_qkupstream_init(ngx_http_request_t	*r)
{
	ngx_http_qkpack_ctx_t 				*ctx;
	ngx_http_variable_value_t   		*vv;
	ngx_int_t							 rc;

	ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
				"[qkupstream] ngx_http_qkupstream_init");
	
	ctx = (ngx_http_qkpack_ctx_t*)ngx_http_get_module_ctx(r, ngx_http_qkupstream_module);
	if ( ctx == NULL ) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, 
                    "[qkupstream] ngx_http_qkupstream_init ctx is null");
					
		return NGX_ERROR;
	}
	
	vv = ngx_http_get_plugin(r);
    if ( vv == NULL ) {

    	ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, 
                    "[qkupstream] ngx_http_qkupstream_init plugin_name variable is not found!");
    
    	return NGX_ERROR;
    }
	
    ctx->plugin = plugin_getbyname((char*)vv->data, vv->len);
    if(ctx->plugin == NULL) {

	    ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, 
                "[qkupstream] ngx_http_qkupstream_init plugin handler is null_ptr!");

	    return NGX_ERROR;
    }


	//这个表示主的request，也就是当前的request链中最上面的那个request，通过这个域我们就能判断当前的request是不是subrequest。
	if ( r->main == r ) {
		ctx->is_subrequest = 0;
		qkpack_request_t *request = new qkpack_request_t;
		
		rc = ngx_http_get_post_body(r,request);
		if ( rc != NGX_OK ) {
			ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
					"[qksubrequest] ngx_http_qkupstream_init reuqest_buffer is null");

			return NGX_ERROR;
		}
		
		ctx->data = request;

		return NGX_OK;
	}
		
	ctx->is_subrequest = 1;	

	qkpack_request_t				*request = new qkpack_request_t;
	static ngx_str_t				args_slotid = ngx_string("slotid");
	static ngx_str_t				args_key	= ngx_string("key");

	ngx_str_t						slotid_str = ngx_null_string;
	ngx_str_t						key_str = ngx_null_string;
	qkpack_request_t				*request_parent;				
	ngx_http_qkpack_ctx_t			*ctx_parent;
	
	ctx_parent = (ngx_http_qkpack_ctx_t*)ngx_http_get_module_ctx(r->parent, ngx_http_qksubrequest_module);
	if ( ctx_parent == NULL ) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, 
				"[qkupstream] ngx_http_qkupstream_init ctx_parent is null");	
				
		return NGX_ERROR;
	}
	
	request_parent = (qkpack_request_t*)ctx_parent->data;
	if ( request_parent == NULL ) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
				"[qkupstream] ngx_http_qkupstream_init request_parent is null");
					
		return NGX_ERROR;
	}
	request->parent =  request_parent;

	ngx_http_arg(r, args_slotid.data, args_slotid.len, &slotid_str);

	if ( slotid_str.data != NULL && slotid_str.len ) {
		request->slot_id =  ngx_atoi(slotid_str.data, slotid_str.len);
	}

	if ( request->slot_id < 0 ) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
				"[qkupstream] ngx_http_qkupstream_init subrequest slotid less than zero");

		return NGX_ERROR;
	}

	ngx_http_arg(r, args_key.data, args_key.len, &key_str);

	if ( key_str.data != NULL && key_str.len ) {
		request->kvpair.key = std::string((char*)key_str.data,key_str.len);
	}

	ctx->data = request;
	
	return NGX_OK;
}


ngx_int_t
ngx_http_qkupstream_destroy(ngx_http_request_t	*r)
{
	ngx_http_qkpack_ctx_t 				*ctx;
	CPlugin 							*plugin;
	qkpack_request_t 					*request;
	
	ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
			"[qkupstream] ngx_qkpack_destroy_crequest");
	
	ctx = (ngx_http_qkpack_ctx_t*)ngx_http_get_module_ctx(r, ngx_http_qkupstream_module);
	if ( ctx == NULL ) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, 
                    "[qkupstream] ngx_qkpack_destroy_crequest ctx is null");
					
		return NGX_ERROR;
	}
	
	plugin = (CPlugin *)ctx->plugin;
	if ( plugin == NULL ) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                    "[qkupstream] ngx_qkpack_destroy_crequest plugin is null");

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
ngx_http_qkupstream_process_request(ngx_http_request_t	*r)
{
	ngx_int_t 							rc;
	ngx_http_qkpack_ctx_t 				*ctx;
	ngx_http_variable_value_t 			*vv_upstream_name;
	ngx_http_variable_value_t 			*vv_upstream_ip;
	ngx_http_variable_value_t 			*vv_server_addr;
	CPlugin 							*plugin;
	qkpack_request_t					*request;
	ngx_http_upstream_t     			*u = r->upstream;
	
	ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
				"[qkupstream] ngx_http_qkupstream_process_request");

	
	vv_upstream_name = ngx_http_qkupstream_get_upstream_name(r);
	if ( vv_upstream_name == NULL ) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                    "[qkupstream] ngx_http_qkupstream_process_request upstream_name is null");

		return NGX_ERROR;
	}
	
	vv_upstream_ip = ngx_http_qkupstream_get_upstream_ip_name(r);
	if ( vv_upstream_ip == NULL ) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                    "[qkupstream] ngx_http_qkupstream_process_request upstream_ip is null");

		return NGX_ERROR;
	}
	
	vv_server_addr = ngx_http_get_server(r);
    if ( vv_server_addr == NULL ) {

    	ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, 
                    "[qkupstream] ngx_http_get_server server_addr variable is not found!");
    
    	return NGX_ERROR;
    }
	
	ctx = (ngx_http_qkpack_ctx_t*)ngx_http_get_module_ctx(r, ngx_http_qkupstream_module);
	if ( ctx == NULL ) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                    "[qkupstream] ngx_http_qkupstream_process_request ctx is null");

		return NGX_ERROR;
	}
	
	plugin = (CPlugin *)ctx->plugin;
	if ( plugin == NULL ) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                    "[qkupstream] ngx_http_qkupstream_process_request plugin is null");

		return NGX_ERROR;
	}
	
	request = (qkpack_request_t*)ctx->data;
	if ( request == NULL ) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                    "[qkupstream] ngx_http_qkupstream_process_request qkpack_request_t is null");

		return NGX_ERROR;
	}
	
	ctx->upstream_name.data = vv_upstream_name->data;
	ctx->upstream_name.len = vv_upstream_name->len;
	ctx->upstream_ip.data = vv_upstream_ip->data;
	ctx->upstream_ip.len = vv_upstream_ip->len;


	//这个表示主的request，也就是当前的request链中最上面的那个request，通过这个域我们就能判断当前的request是不是subrequest。
	if ( r->main == r ) {

		request->pid = ngx_getpid();
		request->host = std::string((char*)vv_server_addr->data,vv_server_addr->len);

		//get x_real_ip
		ngx_table_elt_t *x_real_ip = r->headers_in.x_real_ip; 
		if(x_real_ip) { 
			request->x_real_ip = std::string((char *)x_real_ip->value.data,x_real_ip->value.len); 
		}else {
			request->x_real_ip = std::string((char *)r->connection->addr_text.data,
					r->connection->addr_text.len);
		}
	}	
	
	request->request_uri = std::string((char*)r->uri.data,r->uri.len);
	request->is_subrequest = ctx->is_subrequest;
	request->cluster_name = std::string((char*)ctx->upstream_name.data, ctx->upstream_name.len);
	request->node_ip = std::string((char*)ctx->upstream_ip.data, ctx->upstream_ip.len);

	if ( upstream_tries == 0 ) {
		upstream_tries  =  u->conf->upstream_tries;
	}

	if ( upstream_timeout == 0 ) {
		upstream_timeout = u->conf->connect_timeout;
	}	

	request->tries = upstream_tries;
	request->timeout = upstream_timeout;

    rc = plugin->CreateRequest(request);
    if (rc != NGX_OK ) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                "[qkupstream] ngx_http_qkupstream_process_request plugin->CreateRequest is error");
				
        return NGX_ERROR;
    }
	
	ctx->request_buffer.data = (u_char*)request->kvpair.proto_str.c_str();
	ctx->request_buffer.len = request->kvpair.proto_str.length();

	ctx->upstream_ip.data = (u_char*)request->node_ip.c_str();
	ctx->upstream_ip.len  = request->node_ip.length();

	ctx->upstream_name.data = (u_char*)request->cluster_name.c_str();
	ctx->upstream_name.len  = request->cluster_name.length();
	
	//set upstream_ip
	vv_upstream_ip->len = ctx->upstream_ip.len;
	vv_upstream_ip->data = ctx->upstream_ip.data;	
	
	if ( request->tries > 0 ) {
		u->conf->upstream_tries = request->tries;
	}
	
	if ( request->timeout > 0 ) {
		u->conf->connect_timeout = request->timeout;
		u->conf->send_timeout = request->timeout;
		u->conf->read_timeout = request->timeout;
	}

	return NGX_OK;
}


ngx_int_t
ngx_http_qkupstream_process_response(ngx_http_request_t *r, ngx_buf_t * upstream_buf)
{
	
	ngx_int_t 							rc;
	ngx_http_qkpack_ctx_t 				*ctx;
	CPlugin 							*plugin;
	qkpack_request_t					*request;
	ngx_http_upstream_t     			*u = r->upstream;
	ngx_str_t 							type = ngx_string("text/plain");
	
	ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
				"[qkupstream] ngx_http_qkupstream_process_response");
	
	ctx = (ngx_http_qkpack_ctx_t*)ngx_http_get_module_ctx(r, ngx_http_qkupstream_module);
	if ( ctx == NULL ) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                    "[qkupstream] ngx_http_qkupstream_process_response ctx is null");

		return NGX_ERROR;
	}

	plugin = (CPlugin *)ctx->plugin;
	if ( plugin == NULL ) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                    "[qkupstream] ngx_http_qkupstream_process_response plugin is null");

		return NGX_ERROR;
	}
	
	request = (qkpack_request_t*)ctx->data;
	if ( request == NULL ) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                    "[qkupstream] ngx_http_qkupstream_process_response qkpack_request_t is null");

		return NGX_ERROR;
	}
	
	request->response_buffer = std::string((char*)ctx->response_buffer.data,ctx->response_buffer.len);
	request->kvpair.moved_ask = ctx->moved_ask;
	
    rc = plugin->ProcessBody(request);
	if ( rc != NGX_OK ) {
		//agagin	
		if ( rc ==	NGX_AGAIN ) {
			return rc;
		}

		//moved,ask
		if ( rc == NGX_HTTP_UPSTREAM_INVALID_HEADER ) {
			
			ctx->upstream_ip.data = (u_char*)request->node_ip.c_str();
			ctx->upstream_ip.len  = request->node_ip.length();
	
			ctx->upstream_name.data = (u_char*)request->cluster_name.c_str();
			ctx->upstream_name.len  = request->cluster_name.length();
			ctx->moved_ask = request->kvpair.moved_ask;
			
			ngx_log_error(NGX_LOG_ERR, r->connection->log, 1,
	                "[qkupstream] ngx_http_qkupstream_process_response plugin->ProcessBody is %s",
					(ctx->moved_ask == QKUPSTREAM_MOVED ? "moved" : "ask") );
	
			return NGX_HTTP_UPSTREAM_INVALID_HEADER;
		}

		ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
	            "[qkupstream] ngx_http_qkupstream_process_response plugin->ProcessBody is error");
					
		return NGX_ERROR;
	}

	if (  request->response_buffer.length() == 0) {
		request->response_buffer = "OK";
	}

	ctx->response_buffer.data = (u_char*)request->response_buffer.c_str();
	ctx->response_buffer.len  = request->response_buffer.length();

	ngx_memcpy(upstream_buf->pos,  ctx->response_buffer.data, ctx->response_buffer.len);
	
//	upstream_buf->pos = ctx->response_buffer.data;
	upstream_buf->last = upstream_buf->pos + ctx->response_buffer.len;
	
	r->headers_out.content_type = type;
	r->headers_out.content_type_len = type.len;
	
	u->keepalive = 1;
    u->headers_in.status_n = NGX_HTTP_OK;
    u->state->status = NGX_HTTP_OK;
	u->headers_in.content_length_n =  ctx->response_buffer.len;
	
	return NGX_OK;
}


ngx_int_t
ngx_http_qkupstream_process_reinit_request(ngx_http_request_t *r)
{
	
	ngx_int_t 							rc;
	ngx_http_qkpack_ctx_t 				*ctx;
	CPlugin 							*plugin;
	qkpack_request_t					*request;
	
	ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
				"[qkupstream] ngx_http_qkupstream_reinit_request");
	
	ctx = (ngx_http_qkpack_ctx_t*)ngx_http_get_module_ctx(r, ngx_http_qkupstream_module);
	if ( ctx == NULL ) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                    "[qkupstream] ngx_http_qkupstream_reinit_request ctx is null");

		return NGX_ERROR;
	}

	plugin = (CPlugin *)ctx->plugin;
	if ( plugin == NULL ) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                    "[qkupstream] ngx_http_qkupstream_reinit_request plugin is null");

		return NGX_ERROR;
	}
	
	request = (qkpack_request_t*)ctx->data;
	if ( request == NULL ) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                    "[qkupstream] ngx_http_qkupstream_reinit_request qkpack_request_t is null");

		return NGX_ERROR;
	}

	request->cluster_name = std::string((char*)ctx->upstream_name.data, ctx->upstream_name.len);
	request->node_ip = std::string((char*)ctx->upstream_ip.data, ctx->upstream_ip.len);
	request->now_tries = ctx->tries;
	request->kvpair.moved_ask = ctx->moved_ask;


    rc = plugin->ReinitRequest(request);
    if (rc != NGX_OK ) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                "[qkupstream] ngx_http_qkupstream_process_reinit_request plugin->ReinitRequest is error");
				
        return NGX_ERROR;
    }
	
	ctx->upstream_ip.data = (u_char*)request->node_ip.c_str();
	ctx->upstream_ip.len  = request->node_ip.length();

	ctx->upstream_name.data = (u_char*)request->cluster_name.c_str();
	ctx->upstream_name.len  = request->cluster_name.length();
		
	ctx->moved_ask = request->kvpair.moved_ask;
	
	if ( ctx->moved_ask == QKUPSTREAM_ASKING ) {
		ctx->request_buffer.data = (u_char*)request->kvpair.proto_str.c_str();
		ctx->request_buffer.len = request->kvpair.proto_str.length();
	}	

	return rc;
}


