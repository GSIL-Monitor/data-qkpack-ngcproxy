/*
 * Author: lihang <lihang01@youku.com>
 *
 * Create Date: 2015-08
 *
 */

#include "ngx_qkplugin_manager.h"
#include "common.h"
#include "plugin.h"
#include <dlfcn.h>
#include <map>
#include <string>
#include <tr1/memory>


typedef std::tr1::shared_ptr<CPluginInfo> PluginInfoPtr;
typedef std::map<std::string, PluginInfoPtr> PluginInfoPtrMap;

typedef CPlugin *(*CreatePluginFunc)();

static PluginInfoPtrMap qkplugins_info_map_;
const static std::string kCreatePluginFunc = "create_instance";

ngx_int_t 
ngx_load_qkplugin(ngx_cycle_t *cycle, qkplugin_conf_ctx_t *ctx) 
{  
   
    void *so_handler = dlopen((char*)ctx->so_path.data, RTLD_LAZY);
    if (so_handler == NULL) {
		ngx_log_error(NGX_LOG_ERR, cycle->log, 0, "[qkqkplugin] load qkplugin is error=[%s]",dlerror());
        return NGX_ERROR;
    }

    CreatePluginFunc handler = NULL;
    handler = (CreatePluginFunc)dlsym(so_handler, kCreatePluginFunc.c_str());
    if (handler == NULL) {
        dlclose(so_handler);
		
		ngx_log_error(NGX_LOG_ERR, cycle->log, 0, "[qkqkplugin] handler is null");
        return NGX_ERROR;
    }

    CPlugin* qkplugin = (*handler)();
    if (qkplugin == NULL) {
        dlclose(so_handler);
		
		ngx_log_error(NGX_LOG_ERR, cycle->log, 0, "[qkqkplugin] qkplugin is null");
        return NGX_ERROR;
    }

    PluginInfoPtr qkplugin_info(new CPluginInfo());
    qkplugin_info->name = ctx->name;
    qkplugin_info->so_path = ctx->so_path;
    qkplugin_info->so_conf = ctx->so_conf;
    qkplugin_info->qkplugin_ptr = qkplugin;
    qkplugin_info->so_handler = so_handler;

	std::string plugin_name = std::string((char*)ctx->name.data,ctx->name.len);
    qkplugins_info_map_.insert(std::make_pair(plugin_name, qkplugin_info));

    return NGX_OK;
}


ngx_int_t 
ngx_init_master_qkplugin() 
{  
    PluginInfoPtrMap::iterator it = qkplugins_info_map_.begin();

    for(; it != qkplugins_info_map_.end(); ++it) {
		
		CPluginInfo *qkplugin_info = it->second.get();
		std::string so_conf = std::string((char*)qkplugin_info->so_conf.data, qkplugin_info->so_conf.len);
		
        ngx_int_t rc = ((CPlugin*)(qkplugin_info->qkplugin_ptr))->InitMaster(so_conf); 
        if( rc != NGX_OK ) {
            return NGX_ERROR;
        }
    }

    return NGX_OK;
}


ngx_int_t
ngx_init_process_qkplugin()
{
    PluginInfoPtrMap::iterator it = qkplugins_info_map_.begin();

    for(; it != qkplugins_info_map_.end(); ++it) {
		
        CPluginInfo *qkplugin_info = it->second.get();
		std::string so_conf = std::string((char*)qkplugin_info->so_conf.data, qkplugin_info->so_conf.len);
		
        ngx_int_t rc = ((CPlugin*)(qkplugin_info->qkplugin_ptr))->InitProcess(so_conf); 
        if( rc != NGX_OK ) {
            return NGX_ERROR;
        }
    }

    return NGX_OK;
}



ngx_int_t
ngx_publish_process_qkplugin()
{
    PluginInfoPtrMap::iterator it = qkplugins_info_map_.begin();

    for(; it != qkplugins_info_map_.end(); ++it) {
		
		CPluginInfo *qkplugin_info = it->second.get();
		if ( qkplugin_info == NULL || qkplugin_info->qkplugin_ptr == NULL ) {
			return NGX_ERROR;
		}

		CPlugin *plugin = (CPlugin*)(qkplugin_info->qkplugin_ptr);
		if ( plugin == NULL ) {
			return NGX_ERROR;
		}

		ngx_int_t rc = plugin->Publish(); 
		if( rc != NGX_OK ) {
			return NGX_ERROR;
		}
    }

    return NGX_OK;
}


void
ngx_exit_process_qkplugin()
{
    PluginInfoPtrMap::iterator it = qkplugins_info_map_.begin();

    for(; it != qkplugins_info_map_.end(); ++it) {
        CPluginInfo *qkplugin_info = it->second.get();
		if ( qkplugin_info && qkplugin_info->qkplugin_ptr ) {
			((CPlugin*)(qkplugin_info->qkplugin_ptr))->ExitProcess();
		}		
    }
}

void
ngx_exit_master_qkplugin()
{
    PluginInfoPtrMap::iterator it = qkplugins_info_map_.begin();

    for(; it != qkplugins_info_map_.end(); ++it) {
        CPluginInfo *qkplugin_info = it->second.get();
		if ( qkplugin_info && qkplugin_info->qkplugin_ptr ) {
			CPlugin* plugin = (CPlugin*)(qkplugin_info->qkplugin_ptr);
			if ( plugin ) {
				plugin->ExitMaster(); 
				
				delete plugin;
				plugin = NULL;
			}
			
			if( qkplugin_info->so_handler ) {
				dlclose(qkplugin_info->so_handler);
				qkplugin_info->so_handler = NULL;
			}
		}
    }
}


void *
plugin_getbyname(const char *name, size_t len)
{
	const std::string plugin_name = std::string(name,len);
    PluginInfoPtrMap::iterator it = qkplugins_info_map_.find(plugin_name);
    if (it == qkplugins_info_map_.end()) {
        return NULL;
    }

    return it->second->qkplugin_ptr;
}

ngx_http_variable_value_t *
ngx_http_get_plugin(ngx_http_request_t *r) 
{
    
    ngx_uint_t  				hash;
	ngx_str_t 					var = ngx_string("plugin_name");
    ngx_http_variable_value_t   *vv;
	
    hash = ngx_hash_key (var.data, var.len);
    vv = ngx_http_get_variable(r, &var, hash);
    if (vv == NULL || vv->not_found || vv->len==0) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, 
                    "[qkplugin] plugin_name variable is not found");
    
    	return NULL;
    }
	
    return vv;
}

ngx_http_variable_value_t *
ngx_http_get_server(ngx_http_request_t *r) 
{
    
    ngx_uint_t  				hash;
	ngx_str_t 					var = ngx_string("server_addr");
    ngx_http_variable_value_t   *vv;
	
    hash = ngx_hash_key (var.data, var.len);
    vv = ngx_http_get_variable(r, &var, hash);
    if (vv == NULL || vv->not_found || vv->len==0) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, 
                    "[qkplugin] server_addr variable is not found");
    
    	return NULL;
    }
	
    return vv;
}


ngx_int_t
ngx_http_get_post_body(ngx_http_request_t *r, void *handler) 
{
	qkpack_request_t				*request;	

	request = (qkpack_request_t*)handler;
	if ( request == NULL ) {
		return NGX_ERROR;
	}

	std::string &post_body = request->request_buffer;
	
	if (!(r->method & (NGX_HTTP_POST))) {
        return NGX_ERROR;
    }

	ngx_chain_t *cl = NULL;
	if ( r->upstream != NULL && r->upstream->request_bufs != NULL ) {
		cl = r->upstream->request_bufs;
	}
	else if ( r->request_body != NULL && r->request_body->bufs != NULL ) {
		cl = r->request_body->bufs;
	}

	if ( cl == NULL ) {
		return NGX_ERROR;	
	}

    char *buf_temp = NULL;
    int buf_size = 0;

    for(; cl; cl = cl->next) {
        if(ngx_buf_in_memory(cl->buf)) {
            post_body.append((char *)cl->buf->pos, cl->buf->last - cl->buf->pos);
            continue;
        } 

        /* buf in file */
        int nbytes = cl->buf->file_last - cl->buf->file_pos;
        if(nbytes <= 0) {
            continue;
        } 
        
        /* allocate lager temp buf */
        if(nbytes > buf_size) {
            if(buf_size > 0) delete [] buf_temp;

            buf_temp = new char[nbytes];
            buf_size = nbytes;
        }
        
        int buf_pos = 0;
        while(nbytes > 0) {
            int nread = ngx_read_file(cl->buf->file, (u_char *)buf_temp + buf_pos, 
                    cl->buf->file_last - cl->buf->file_pos, cl->buf->file_pos);

            if(nread < 0) { /* read temp file error */
                if(buf_size > 0) delete [] buf_temp;      

                return -1;
            }

            buf_pos += nread;
            nbytes -= nread; 
        }

        post_body.append(buf_temp, buf_size);
    } 

    if(buf_size > 0) delete [] buf_temp;

    return NGX_OK;
}
