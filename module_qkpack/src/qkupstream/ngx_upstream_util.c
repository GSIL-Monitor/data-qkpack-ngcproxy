/*
 * Author: lihang <lihang01@youku.com>
 *
 * Create Date: 2015-08
 *
 */

#include "ngx_upstream_util.h"

static ngx_http_upstream_server_t*
ngx_http_qkupstream_compare_server(ngx_http_upstream_srv_conf_t * us, ngx_url_t u);
static ngx_http_upstream_srv_conf_t *
ngx_http_qkupstream_find_upstream(ngx_http_request_t *r,  ngx_str_t *host);
static ngx_int_t 
ngx_http_qkupstream_exist_peer(ngx_http_upstream_rr_peers_t * peers, ngx_url_t u);

static void *
ngx_prealloc_and_delay(ngx_pool_t *pool, void *p, size_t old_size, size_t new_size);


/*
*add upstream server
*
*upstream_name: upstream_name
*upstream_ip: 127.0.0.1:7005	
*/
ngx_int_t 
ngx_http_qkupstream_add_server(ngx_http_request_t *r, ngx_str_t upstream_name,
		ngx_str_t upstream_ip)
{
	ngx_int_t 						 rc;
	ngx_http_upstream_srv_conf_t     *uscf;
	ngx_http_upstream_server_t       *us;
	ngx_url_t						 u;
	ngx_str_t                         id;

	if ( upstream_name.len == 0 || upstream_ip.len == 0 ) {
		return NGX_ERROR;
	}

	

	uscf = ngx_http_qkupstream_find_upstream(r, &upstream_name);
	if ( uscf == NULL ) {
		
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
						"[qkupstream]	ngx_http_upstream_add_server upstream not found upstream_name=[%V]", &upstream_name);
		
		return NGX_ERROR;
	}
	ngx_memzero(&u, sizeof (ngx_url_t));

	u.url.len = upstream_ip.len;
	u.url.data = ngx_pcalloc(uscf->servers->pool, u.url.len);
	ngx_memcpy(u.url.data, upstream_ip.data, u.url.len);
   
	u.default_port = 80;
	u.uri_part = 0;
	u.no_resolve = 1;

	if (ngx_http_qkupstream_compare_server(uscf, u) != NULL) {

		ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
						"[qkupstream]	ngx_http_qkupstream_add_server this server is exist");

		return NGX_ERROR;
	}

	if (uscf->servers == NULL || uscf->servers->nelts == 0) {

		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
						"[qkupstream]	ngx_http_qkupstream_add_server upstream has no server before!");
						
		return NGX_ERROR;

	}

	rc = ngx_parse_url(uscf->servers->pool, &u);
	if ( rc != NGX_OK && u.err ) {

		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
						"[qkupstream]	ngx_http_qkupstream_add_server url parser error upstream_name=[%v],upstream_ip=[%V]",
						&upstream_name, &upstream_ip);

		return NGX_ERROR;

	}

	us = ngx_array_push(uscf->servers);
	if (us == NULL) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
						"[qkupstream]	ngx_http_qkupstream_add_server us push uscf->servers failed");
		return NGX_ERROR;
	}

	ngx_memzero(us, sizeof (ngx_http_upstream_server_t));

	/*
	us->name = u.url;
	us->addrs = u.addrs;
	us->naddrs = u.naddrs;
	us->weight = 0;
	us->max_fails = 6;
	us->fail_timeout = 0;
	*/

	if (u.addrs && u.addrs[0].sockaddr) {
		
		us->host = u.host;
		ngx_str_null(&id);
		us->id = id;
		
		//us->name = u.url;
		us->addrs = u.addrs;
		us->naddrs = u.naddrs;
		us->weight = 0;
		us->max_fails = 6;
		us->fail_timeout = 0;
		
		us->addrs->name = u.addrs[0].name;	
		us->addrs->sockaddr = u.addrs[0].sockaddr;
		us->addrs->socklen = u.addrs[0].socklen;

	} else {
		//*err = "no host allowed";
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
				"[qkupstream]	ngx_http_qkupstream_add_server nohost allowed");
		
		return NGX_ERROR;
	}

	ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
						"[qkupstream]	ngx_http_qkupstream_add_server server=[%V] is ok",&u.url);
	
	return NGX_OK;
}


/*
*add upstream server peer
*
*upstream_name: upstream_name
*upstream_ip: 127.0.0.1:7005	
*/
ngx_int_t 
ngx_http_qkupstream_add_peer(ngx_http_request_t *r, ngx_str_t upstream_name,
		ngx_str_t upstream_ip)
{
	ngx_uint_t                              n;
	ngx_http_upstream_srv_conf_t     		*uscf;
	ngx_http_upstream_server_t       		*us;
	ngx_http_upstream_rr_peer_t             peer; 
    ngx_http_upstream_rr_peers_t          	*peers;
	ngx_url_t						  		u;
	size_t                                  old_size, new_size;

	if ( upstream_name.len == 0 || upstream_ip.len == 0 ) {
		return NGX_ERROR;
	}
	
	uscf = ngx_http_qkupstream_find_upstream(r, &upstream_name);
	if ( uscf == NULL ) {
		
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
						"[qkupstream]	ngx_http_qkupstream_add_peer upstream not found upstream_name=[%V]", &upstream_name);
		
		return NGX_ERROR;
	}
	ngx_memzero(&u, sizeof (ngx_url_t));

	u.url.len = upstream_ip.len;
	u.url.data = upstream_ip.data;

	//u.url.data = ngx_pcalloc(uscf->servers->pool, u.url.len+1);
    	//ngx_memcpy(u.url.data, upstream_ip.data, u.url.len);

    	u.default_port = 80;
	
	us = ngx_http_qkupstream_compare_server(uscf, u);
	if ( us == NULL) {

		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
						"[qkupstream]	ngx_http_qkupstream_add_peer server not found upstream_name=[%v],upstream_ip=[%V]",
						&upstream_name, &upstream_ip);

		return NGX_ERROR;
	}
	
	peers = uscf->peer.data;
	ngx_memzero(&peer, sizeof (ngx_http_upstream_rr_peer_t));


	if (ngx_http_qkupstream_exist_peer(peers, u)) {
		
		ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
					"[qkupstream]	ngx_http_qkupstream_add_peer the peer is exist");
		
		return NGX_ERROR;
	}

	n = peers != NULL ? (peers->number - 1) : 0;
	
	old_size = n * sizeof(ngx_http_upstream_rr_peer_t) 
		+ sizeof(ngx_http_upstream_rr_peers_t);
	new_size = old_size + sizeof(ngx_http_upstream_rr_peer_t);

	peers = ngx_prealloc_and_delay(ngx_cycle->pool, uscf->peer.data, old_size, new_size);
	if (peers == NULL) {
		
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
					"[qkupstream]	ngx_http_qkupstream_add_peer peers pcalloc fail");
		
		return NGX_ERROR;
	}
	
	peer.weight = us->weight; 
	peer.effective_weight = us->weight;
	peer.current_weight= 0;
	peer.max_fails = us->max_fails;
	peer.fail_timeout = us->fail_timeout;
	peer.host = us->host;
	peer.id = us->id;
	peer.sockaddr = us->addrs->sockaddr;
	peer.socklen = us->addrs->socklen;
	peer.name = us->addrs->name;
	peer.down = us->down;
	peer.fails = 0;
	//peer.server = us->name;

	peers->peer[peers->number++] = peer;
	peers->total_weight += peer.weight;
	peers->single = (peers->number == 1);
	peers->weighted = (peers->total_weight != peers->number);

	uscf->peer.data = peers;

	ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
						"[qkupstream]	ngx_http_qkupstream_add_peer server=[%V] is ok",&u.url);
		
	return NGX_OK;
}


/*
*get upstream server peer
*
*upstream_name: upstream_name
*upstream_ip: 127.0.0.1:7005	
*/
ngx_http_upstream_rr_peer_t *
ngx_http_qkupstream_get_peers(ngx_http_request_t *r, ngx_str_t upstream_name,
		ngx_str_t upstream_ip)
{
    ngx_uint_t                            	i;
    ngx_http_upstream_rr_peers_t         	*peers;
    ngx_http_upstream_srv_conf_t         	*us;
	ngx_url_t						  		u;
	size_t                                  len;

	if ( upstream_name.len == 0 || upstream_ip.len == 0 ) {
		 return NULL;
	}
	
	us = ngx_http_qkupstream_find_upstream(r, &upstream_name);
	if ( us == NULL ) {
		
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
						"[qkupstream]	ngx_http_qkupstream_get_peers upstream not found upstream_name=[%V]", &upstream_name);
		
		 return NULL;
	}

	u.url.len = upstream_ip.len;
	u.url.data = upstream_ip.data;

	//u.url.data = ngx_pcalloc(us->servers->pool, u.url.len+1);
    	//ngx_memcpy(u.url.data, upstream_ip.data, u.url.len);

    u.default_port = 80;
	
    peers = us->peer.data;

    if (peers == NULL) {
	
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
						"[qkupstream]	ngx_http_qkupstream_get_peers peer not found");
						
         return NULL;
    }

   for (i = 0; i < peers->number; i++) {
       
        len = peers->peer[i].name.len;
        if (len == u.url.len
            && ngx_memcmp(u.url.data, peers->peer[i].name.data, u.url.len) == 0) {
            return &peers->peer[i];
        }
    }

    return NULL;
}



static ngx_int_t 
ngx_http_qkupstream_exist_peer(ngx_http_upstream_rr_peers_t * peers, ngx_url_t u)
{
    ngx_uint_t                          i;
    size_t                              len;
    ngx_http_upstream_rr_peer_t         peer;
	
	if ( peers ==NULL ) {
		return 0;
	}

    for (i = 0; i < peers->number; i++) {
        peer = peers->peer[i];

        len = peer.name.len;
        if (len == u.url.len
            && ngx_memcmp(u.url.data, peer.name.data, u.url.len) == 0) {
            return 1;;
        }
    }
    
    return 0;
}


static ngx_http_upstream_srv_conf_t *
ngx_http_qkupstream_find_upstream(ngx_http_request_t *r,  ngx_str_t *host)
{
    ngx_uint_t                            i;
    ngx_http_upstream_srv_conf_t        **uscfp, *uscf;
    ngx_http_upstream_main_conf_t        *umcf;

    umcf = ngx_http_get_module_main_conf(r, ngx_http_upstream_module);
    uscfp = umcf->upstreams.elts;

    for (i = 0; i < umcf->upstreams.nelts; i++) {

        uscf = uscfp[i];

        if (uscf->host.len == host->len
            && ngx_memcmp(uscf->host.data, host->data, host->len) == 0) {
            return uscf;
        }
    }
	
    return NULL;
}

static ngx_http_upstream_server_t*
ngx_http_qkupstream_compare_server(ngx_http_upstream_srv_conf_t * us, ngx_url_t u)
{
    ngx_uint_t                       i, j;
    size_t                           len;
    ngx_http_upstream_server_t      *server = NULL;

    if (us->servers == NULL || us->servers->nelts == 0) {
        return NULL;
    }

    server = us->servers->elts;

    for (i = 0; i < us->servers->nelts; ++i) {
        for(j = 0; j < server[i].naddrs; ++j) {

            len = server[i].addrs[j].name.len;
            if (len == u.url.len 
                 && ngx_memcmp(u.url.data, server[i].addrs[j].name.data, u.url.len) == 0) {

                return  &server[i];
            } 
         }
    }

    return NULL;
}



char *
ngx_http_qkupstream_set_complex_value_slot(ngx_conf_t *cf, ngx_command_t *cmd,
    void *conf)
{
    char                             *p = conf;
    ngx_str_t                        *value;
    ngx_http_complex_value_t         **field;
    ngx_http_compile_complex_value_t  ccv;

    field = (ngx_http_complex_value_t **) (p + cmd->offset);

    if (*field) {
        return "is duplicate";
    }

    *field = ngx_palloc(cf->pool, sizeof(ngx_http_complex_value_t));
    if (*field == NULL) {
        return NGX_CONF_ERROR;
    }

    value = cf->args->elts;

    if (value[1].len == 0) {
        ngx_memzero(*field, sizeof(ngx_http_complex_value_t));
        return NGX_OK;
    }

    ngx_memzero(&ccv, sizeof(ngx_http_compile_complex_value_t));

    ccv.cf = cf;
    ccv.value = &value[1];
    ccv.complex_value = *field;

    if (ngx_http_compile_complex_value(&ccv) != NGX_OK) {
        return NGX_CONF_ERROR;
    }
	
    return NGX_CONF_OK;
}



ngx_http_upstream_srv_conf_t *
ngx_http_qkupstream_upstream_add(ngx_http_request_t *r, ngx_url_t *url)
{

	ngx_uint_t                      i;
    ngx_http_upstream_main_conf_t   *umcf;
    ngx_http_upstream_srv_conf_t    **uscfp;


    umcf = ngx_http_get_module_main_conf(r, ngx_http_upstream_module);

    uscfp = umcf->upstreams.elts;

    for (i = 0; i < umcf->upstreams.nelts; i++) {

        if (uscfp[i]->host.len != url->host.len
            || ngx_strncasecmp(uscfp[i]->host.data, url->host.data,
               url->host.len) != 0)
        {
			ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
					"[qkupstream] ngx_http_qkupstream_upstream_add  upstream_add: host not match");
			
            continue;
        }

        if (uscfp[i]->port != url->port) {
           
		    ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
		    		"[qkupstream] ngx_http_qkupstream_upstream_add  upstream_add: port not match");
		   
            continue;
        }

        if (uscfp[i]->default_port
            && url->default_port
            && uscfp[i]->default_port != url->default_port)
        {
			ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
					"[qkupstream] ngx_http_qkupstream_upstream_add  upstream_add: default_port not match");
            continue;
        }

        return uscfp[i];
    }

	ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
			"[qkupstream] ngx_http_qkupstream_upstream_add  no upstream found");
	
    return NULL;
}


ngx_http_variable_value_t *
ngx_http_qkupstream_get_upstream_ip_name(ngx_http_request_t *r)
{
	ngx_uint_t  				hash;
	ngx_str_t 					var = ngx_string("node_ip");
	ngx_http_variable_value_t   *vv;

	hash = ngx_hash_key (var.data, var.len);
	vv = ngx_http_get_variable(r, &var, hash);
	if ( vv == NULL || vv->not_found ) {

		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
				"[qkupstream] node_ip variable is not found");

		return NULL;
	}
	
	return vv;
}

ngx_http_variable_value_t *
ngx_http_qkupstream_get_upstream_name(ngx_http_request_t *r)
{
	ngx_uint_t  				hash;
	ngx_str_t 					var = ngx_string("backserver");
	ngx_http_variable_value_t   *vv;

	hash = ngx_hash_key (var.data, var.len);
	vv = ngx_http_get_variable(r, &var, hash);
	if ( vv == NULL || vv->not_found ) {

		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
				"[qkupstream] backserver variable is not found");

		return NULL;
	}
	
	return vv;
}


static void
ngx_http_qkupstream_add_delay_delete(ngx_event_t *event)
{
    ngx_uint_t                     i;
    ngx_connection_t              *c;
    ngx_delay_event_t             *delay_event;
    ngx_http_request_t            *r=NULL;
    ngx_http_log_ctx_t            *ctx=NULL;
    void                          *peers=NULL;

    delay_event = event->data;

    c = ngx_cycle->connections;
    for (i = 0; i < ngx_cycle->connection_n; i++) {

        if (c[i].fd == (ngx_socket_t) - 1) {
            continue;
        } else {

            if (c[i].log->data != NULL) {
                ctx = c[i].log->data;
                r = ctx->current_request;
            }
        }

        if (r) {
            if (r->start_sec < delay_event->start_sec) {
                ngx_add_timer(&delay_event->delay_delete_ev, NGX_DELAY_DELETE);
                return;
            }

            if (r->start_sec == delay_event->start_sec) {

                if (r->start_msec <= delay_event->start_msec) {
                    ngx_add_timer(&delay_event->delay_delete_ev, NGX_DELAY_DELETE);
                    return;
                }
            }
        }
    }

    peers = delay_event->data;

    if (peers != NULL) {

        ngx_free(peers);
        peers = NULL;
    }

 
    ngx_free(delay_event);

    delay_event = NULL;

    return;
}


static void
ngx_http_qkupstream_event_init(void *peers)
{
    ngx_time_t                                  *tp;
    ngx_delay_event_t                           *delay_event;


    delay_event = ngx_calloc(sizeof(*delay_event), ngx_cycle->log);
    if (delay_event == NULL) {
        ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0,
                "hngx_http_qkupstream_event_init: calloc failed");
        return;
    }

    tp = ngx_timeofday();
    delay_event->start_sec = tp->sec;
    delay_event->start_msec = tp->msec;

    delay_event->delay_delete_ev.handler = ngx_http_qkupstream_add_delay_delete;
    delay_event->delay_delete_ev.log = ngx_cycle->log;
    delay_event->delay_delete_ev.data = delay_event;
    delay_event->delay_delete_ev.timer_set = 0;


    delay_event->data = peers;
    ngx_add_timer(&delay_event->delay_delete_ev, NGX_DELAY_DELETE);

    return;
}


static ngx_int_t
ngx_pfree_and_delay(ngx_pool_t *pool, void *p)    
{
	ngx_pool_large_t  *l;

	for (l = pool->large; l; l = l->next) {
		if (p == l->alloc) {
			ngx_log_debug1(NGX_LOG_DEBUG_ALLOC, pool->log, 0,
					"delay free: %p", l->alloc);

			ngx_http_qkupstream_event_init(l->alloc);
			
			return NGX_OK;
		}
	}

	return NGX_DECLINED;
}



static void *
ngx_prealloc_and_delay(ngx_pool_t *pool, void *p, size_t old_size, size_t new_size)
{
    void                *new;
    ngx_pool_t          *node;

    if (p == NULL) {
        return ngx_palloc(pool, new_size);
    }    

    if (new_size == 0) { 
        if ((u_char *) p + old_size == pool->d.last) {
           pool->d.last = p; 

        } else {
           ngx_pfree(pool, p);  
        }

        return NULL;
    }    

    if (old_size <= pool->max) {
        for (node = pool; node; node = node->d.next) {
            if ((u_char *)p + old_size == node->d.last
                && (u_char *)p + new_size <= node->d.end) {
                node->d.last = (u_char *)p + new_size;
                return p;
            }
        }
    }    

    if (new_size <= old_size) {
       return p;
    }

    new = ngx_palloc(pool, new_size);
    if (new == NULL) {
        return NULL;
    }
    
    ngx_memcpy(new, p, old_size);

    ngx_pfree_and_delay(pool, p);

    return new;
}
