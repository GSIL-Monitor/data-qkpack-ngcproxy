/*
 * Author: lihang <lihang01@youku.com>
 *
 * Create Date: 2015-08
 *
 */

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

#include "ngx_http_qkpack.h"
#include "ngx_upstream_util.h"
#include "ngx_upstream_interface.h"


typedef struct {
    ngx_http_complex_value_t            key;
} ngx_http_qkupstream_node_srv_conf_t;


typedef struct {
    /* the round robin data must be first */
    ngx_http_upstream_rr_peer_data_t    rrp;
    ngx_http_qkupstream_node_srv_conf_t  *conf;
    ngx_str_t                           key;
    ngx_uint_t                          tries;
    ngx_event_get_peer_pt               get_rr_peer;
	ngx_http_request_t					*r;
} ngx_http_qkupstream_node_peer_data_t;


static ngx_int_t ngx_http_qkupstream_node_init(ngx_conf_t *cf,
    ngx_http_upstream_srv_conf_t *us);
static ngx_int_t ngx_http_qkupstream_node_init_peer(ngx_http_request_t *r,
    ngx_http_upstream_srv_conf_t *us);
static ngx_int_t ngx_http_qkupstream_node_get_peer(ngx_peer_connection_t *pc,
    void *data);
	
static void *ngx_http_qkupstream_node_create_conf(ngx_conf_t *cf);
static char *ngx_http_qkupstream_node(ngx_conf_t *cf, ngx_command_t *cmd,
    void *conf);


static ngx_command_t  ngx_http_qkupstream_node_commands[] = 
{

    { ngx_string("qkupstream_node"),
      NGX_HTTP_UPS_CONF|NGX_CONF_TAKE12,
      ngx_http_qkupstream_node,
      NGX_HTTP_SRV_CONF_OFFSET,
      0,
      NULL },

      ngx_null_command
};


static ngx_http_module_t  ngx_http_qkupstream_node_module_ctx = 
{
    NULL,                                  /* preconfiguration */
    NULL,                                  /* postconfiguration */

    NULL,                                  /* create main configuration */
    NULL,                                  /* init main configuration */

    ngx_http_qkupstream_node_create_conf,    /* create server configuration */
    NULL,                                  /* merge server configuration */

    NULL,                                  /* create location configuration */
    NULL                                   /* merge location configuration */
};


ngx_module_t  ngx_http_qkupstream_node_module = {
    NGX_MODULE_V1,
    &ngx_http_qkupstream_node_module_ctx,    /* module context */
    ngx_http_qkupstream_node_commands,       /* module directives */
    NGX_HTTP_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};


static ngx_int_t
ngx_http_qkupstream_node_init(ngx_conf_t *cf, ngx_http_upstream_srv_conf_t *us)
{
    if (ngx_http_upstream_init_round_robin(cf, us) != NGX_OK) {
        return NGX_ERROR;
    }

    us->peer.init = ngx_http_qkupstream_node_init_peer;

    return NGX_OK;
}


static ngx_int_t
ngx_http_qkupstream_node_init_peer(ngx_http_request_t *r,
    ngx_http_upstream_srv_conf_t *us)
{
    ngx_http_qkupstream_node_srv_conf_t   *hcf;
    ngx_http_qkupstream_node_peer_data_t  *hp;

    hp = ngx_palloc(r->pool, sizeof(ngx_http_qkupstream_node_peer_data_t));
    if (hp == NULL) {
        return NGX_ERROR;
    }

    r->upstream->peer.data = &hp->rrp;
	
    if (ngx_http_upstream_init_round_robin_peer(r, us) != NGX_OK) {
        return NGX_ERROR;
    }

    r->upstream->peer.get = ngx_http_qkupstream_node_get_peer;

    hcf = ngx_http_conf_upstream_srv_conf(us, ngx_http_qkupstream_node_module);
    if (ngx_http_complex_value(r, &hcf->key, &hp->key) != NGX_OK) {
        return NGX_ERROR;
    }

    ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
    		"[qkupstream_node]  upstream  key:\"%V\"", &hp->key);

    hp->conf = hcf;
    hp->tries = 0;
    hp->get_rr_peer = ngx_http_upstream_get_round_robin_peer;

	hp->r = r;

	return NGX_OK;
}

//update nodes info
static ngx_int_t
ngx_http_upstream_update_nodes(ngx_http_qkupstream_node_peer_data_t *hp, ngx_http_request_t *r)
{
	
	ngx_int_t								rc;
	ngx_time_t								*tp;
	ngx_uint_t 								rand_num;
	ngx_uint_t								count;
	ngx_uint_t								max_try = 10;
	ngx_http_qkpack_ctx_t 					*ctx;
	
	ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
		   "[qkupstream_node] ngx_http_upstream_update_nodes ");
	
	ctx = ngx_http_get_module_ctx(r, ngx_http_qkupstream_module);
	if ( ctx == NULL ) {
		return NGX_ERROR;
	}

	//moved ask parser node  -MOVED 1024 127.0.0.1:7000
	if ( ctx->moved_ask != QKUPSTREAM_OK ) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0 ,
				"[qkupstream_node]  moved_ask moved_ask=[%d], upstream_ip=[%V]",ctx->moved_ask, &ctx->upstream_ip);
		
		ctx->tries = hp->tries;
		rc = ngx_http_qkupstream_process_reinit_request(r);
	    if ( rc != NGX_OK ) {
	       
			ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
					"[qkupstream] ngx_http_qkupstream_reinit_request node=[%V]",&hp->key);
	
	        return NGX_ERROR;
	    }
	
		hp->key.data = (u_char*)ctx->upstream_ip.data;
		hp->key.len = ctx->upstream_ip.len;

		//ctx->moved_ask = QKUPSTREAM_OK;
		return NGX_OK;
	}

	//disconnected rand node 
	ngx_str_t default_server = ngx_string("0.0.0.0:80");
	ngx_str_t backup_server = ngx_string("127.0.0.1:8010"); 
	
	tp = ngx_timeofday();
	srand(tp->msec);
	for(;;) {

		count++;
		if ( count >= max_try ) {
			break;
		}

		rand_num=rand() % hp->rrp.peers->number;
		if ( ngx_strncmp(hp->rrp.peers->peer[rand_num].name.data, hp->key.data, hp->key.len) != 0 &&
				ngx_strncmp(hp->rrp.peers->peer[rand_num].name.data, default_server.data, default_server.len) != 0 &&
				ngx_strncmp(hp->rrp.peers->peer[rand_num].name.data, backup_server.data, backup_server.len) !=0 ) {
			break;
		}
	}
	
	hp->key.data = hp->rrp.peers->peer[rand_num].name.data;
	hp->key.len = hp->rrp.peers->peer[rand_num].name.len;
	
	ctx->upstream_ip.data = hp->key.data;
	ctx->upstream_ip.len = hp->key.len;

	ngx_log_error(NGX_LOG_ERR, r->connection->log, 1,
			"[qkupstream_node] rand_node=[%V]",&hp->key);

	ctx->tries = hp->tries;
	rc = ngx_http_qkupstream_process_reinit_request(r);
    if ( rc != NGX_OK ) {
       
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
				"[qkupstream] ngx_http_qkupstream_reinit_request rand_node=[%V]",&hp->key);

        return NGX_ERROR;
    }

	hp->key.data = ctx->upstream_ip.data;
	hp->key.len = ctx->upstream_ip.len;

	return NGX_OK;
}


static ngx_int_t
ngx_http_qkupstream_node_get_peer(ngx_peer_connection_t *pc, void *data)
{
	
	ngx_int_t					  			rc;
    time_t                        			now;
  	ngx_http_qkpack_ctx_t 					*ctx;
	ngx_http_qkupstream_node_peer_data_t    *hp = data;
    ngx_http_upstream_rr_peer_t  			*peer = NULL;
	ngx_http_request_t						*r = hp->r;
	
	ngx_log_error(NGX_LOG_DEBUG, pc->log, 0,
                "[qkupstream_node] ngx_http_qkupstream_node_get_peer get  peer, try: %ui ", pc->tries);
	
	ctx = ngx_http_get_module_ctx(r, ngx_http_qkupstream_module);
	if ( ctx == NULL ) {
		return NGX_ERROR;
	}
	
    if (hp->tries > 50) {
        return hp->get_rr_peer(pc, &hp->rrp);
    }

    now = ngx_time();
    pc->cached = 0;
    pc->connection = NULL;

	//try update route info
	if ( hp->tries > 0 ) {
		
		rc = ngx_http_upstream_update_nodes(hp,r);
		if ( rc != NGX_OK ) {
			ngx_log_error(NGX_LOG_ERR, pc->log, 0 ,
					"[qkupstream_node] update route info is error");
			
			return NGX_ERROR;
		}
	}
	
	//peer = ngx_http_qkupstream_upstream_peer_get(r);
	peer = ngx_http_qkupstream_get_peers(r, ctx->upstream_name, ctx->upstream_ip);
	if ( peer == NULL || peer->name.data == NULL || peer->name.len == 0 ) {
		
		ngx_log_error(NGX_LOG_DEBUG, pc->log, 0,
				   "[qkupstream_node] peer is null add upstream_name=[%V], upstream_ip=[%V]",
				   &ctx->upstream_name, &ctx->upstream_ip);

		peer = ngx_http_qkupstream_get_peers(r, ctx->upstream_name, ctx->upstream_ip);
		if ( peer == NULL ) {

			ngx_http_qkupstream_add_server(r, ctx->upstream_name, ctx->upstream_ip);
			ngx_http_qkupstream_add_peer(r,ctx->upstream_name, ctx->upstream_ip);
	
			peer = ngx_http_qkupstream_get_peers(r, ctx->upstream_name, ctx->upstream_ip);
			if ( peer == NULL ) {
				return NGX_ERROR;
			}
		}
		
		//init _upstream_peer_map 
		//ngx_http_qkupstream_upstream_peer_set(r,peer);
	}

	if ( peer->sockaddr == NULL || peer->socklen == 0 ) {
		ngx_log_error(NGX_LOG_ERR, pc->log, 0,
				"[qkupstream_node] peer->socklen:%ui ", peer->socklen);
			
	}

	ngx_log_error(NGX_LOG_DEBUG, pc->log, 0,
				   "[qkupstream_node] tries:%ui ", hp->tries);
    hp->tries++;			   
    pc->sockaddr = peer->sockaddr;
    pc->socklen = peer->socklen;
    pc->name = &peer->name;
	
	
	ngx_log_error(NGX_LOG_DEBUG, pc->log, 0,
                       "[qkupstream_node] qknode=[%V] last", &peer->name);
	
    if (now - peer->checked > peer->fail_timeout) {
        peer->checked = now;
    }

    return NGX_OK;
}


static void *
ngx_http_qkupstream_node_create_conf(ngx_conf_t *cf)
{
    ngx_http_qkupstream_node_srv_conf_t  *conf;

    conf = ngx_palloc(cf->pool, sizeof(ngx_http_qkupstream_node_srv_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    return conf;
}


static char *
ngx_http_qkupstream_node(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{

	ngx_str_t                         		*value;
	ngx_http_upstream_srv_conf_t      		*uscf;
	ngx_http_compile_complex_value_t  		 ccv;

	value = cf->args->elts;
    ngx_http_qkupstream_node_srv_conf_t  *hcf = conf;

    ngx_memzero(&ccv, sizeof(ngx_http_compile_complex_value_t));

    ccv.cf = cf;
    ccv.value = &value[1];
    ccv.complex_value = &hcf->key;

    if (ngx_http_compile_complex_value(&ccv) != NGX_OK) {
        return NGX_CONF_ERROR;
    }

    uscf = ngx_http_conf_get_module_srv_conf(cf, ngx_http_upstream_module);
    if (uscf->peer.init_upstream) {
        ngx_conf_log_error(NGX_LOG_WARN, cf, 0,
                           "[qkupstream_node]  load balancing method redefined");
    }

    uscf->flags = NGX_HTTP_UPSTREAM_CREATE
                  |NGX_HTTP_UPSTREAM_WEIGHT
                  |NGX_HTTP_UPSTREAM_MAX_FAILS
                  |NGX_HTTP_UPSTREAM_FAIL_TIMEOUT
                  |NGX_HTTP_UPSTREAM_DOWN;

    if (cf->args->nelts == 2) {
        uscf->peer.init_upstream = ngx_http_qkupstream_node_init;

    } else {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                           "[qkupstream_node] invalid parameter \"%V\"", &value[2]);
        return NGX_CONF_ERROR;
    }

    return NGX_CONF_OK;
}
