/*
 * Author: lihang <lihang01@youku.com>
 *
 * Create Date: 2015-08
 *
 */

#include "ngx_http_qkpack.h"
#include "ngx_upstream_handler.h"


static char *ngx_http_qkupstream_pass(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static void *ngx_http_qkupstream_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_qkupstream_merge_loc_conf(ngx_conf_t *cf,void *parent, void *child);
	
	
static ngx_conf_bitmask_t  ngx_http_qkupstream_next_upstream_masks[] = {
    { ngx_string("error"), NGX_HTTP_UPSTREAM_FT_ERROR },
    { ngx_string("timeout"), NGX_HTTP_UPSTREAM_FT_TIMEOUT },
    { ngx_string("invalid_response"), NGX_HTTP_UPSTREAM_FT_INVALID_HEADER },
    { ngx_string("not_found"), NGX_HTTP_UPSTREAM_FT_HTTP_404 },
    { ngx_string("off"), NGX_HTTP_UPSTREAM_FT_OFF },
    { ngx_null_string, 0 }
};


static ngx_command_t  ngx_http_qkupstream_commands[] = {
    
	{ngx_string("qkupstream_pass"),
      NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF|NGX_CONF_TAKE1,
      ngx_http_qkupstream_pass,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL },

    { ngx_string("qkupstream_connect_timeout"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_msec_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_qkupstream_loc_conf_t, upstream.connect_timeout),
      NULL },

    { ngx_string("qkupstream_send_timeout"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_msec_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_qkupstream_loc_conf_t, upstream.send_timeout),
      NULL },

    { ngx_string("qkupstream_buffer_size"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_size_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_qkupstream_loc_conf_t, upstream.buffer_size),
      NULL },

    { ngx_string("qkupstream_read_timeout"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_msec_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_qkupstream_loc_conf_t, upstream.read_timeout),
      NULL },

    { ngx_string("qkupstream_next_upstream"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_1MORE,
      ngx_conf_set_bitmask_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_qkupstream_loc_conf_t, upstream.next_upstream),
      &ngx_http_qkupstream_next_upstream_masks },
	  
	{ ngx_string("qkupstream_next_upstream_tries"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_num_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_qkupstream_loc_conf_t, upstream.upstream_tries),
      NULL },
/*
    { ngx_string("qkupstream_next_upstream_timeout"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_msec_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_qkupstream_loc_conf_t, upstream.next_upstream_timeout),
      NULL },
*/

      ngx_null_command
};

static ngx_http_module_t  ngx_http_qkupstream_module_ctx = {
    NULL,                                  /* preconfiguration */
    NULL,							       /* postconfiguration */

    NULL,                                  /* create main configuration */
    NULL,                                  /* init main configuration */

    NULL,                                  /* create server configuration */
    NULL,                                  /* merge server configuration */

    ngx_http_qkupstream_create_loc_conf,    /* create location configration */
    ngx_http_qkupstream_merge_loc_conf      /* merge location configration */
};


ngx_module_t  ngx_http_qkupstream_module = {
    NGX_MODULE_V1,
    &ngx_http_qkupstream_module_ctx,        /* module context */
    ngx_http_qkupstream_commands,           /* module directives */
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



static void *
ngx_http_qkupstream_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_qkupstream_loc_conf_t  		*conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_qkupstream_loc_conf_t));
    if (conf == NULL) {
        return NULL;
    }
	
    conf->upstream.upstream_tries = NGX_CONF_UNSET_UINT;
    conf->upstream.connect_timeout = NGX_CONF_UNSET_MSEC;
    conf->upstream.send_timeout = NGX_CONF_UNSET_MSEC;
    conf->upstream.read_timeout = NGX_CONF_UNSET_MSEC;
    //conf->upstream.next_upstream_timeout = NGX_CONF_UNSET_MSEC;
	
    conf->upstream.buffer_size = NGX_CONF_UNSET_SIZE;

	/* the hardcoded values */
    conf->upstream.cyclic_temp_file = 0;
    conf->upstream.buffering = 0;
    conf->upstream.ignore_client_abort = 1; 
    conf->upstream.send_lowat = 0;
    conf->upstream.bufs.num = 8;
	conf->upstream.bufs.size = ngx_pagesize;
    conf->upstream.busy_buffers_size =  2 * ngx_pagesize;
    conf->upstream.max_temp_file_size = 1024 * 1024 * 1024;
    conf->upstream.temp_file_write_size = 0;
    conf->upstream.intercept_errors = 1;
    conf->upstream.intercept_404 = 1;
    conf->upstream.pass_request_headers = 0;
    conf->upstream.pass_request_body = 0;

    return conf;
}


static char *
ngx_http_qkupstream_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_qkupstream_loc_conf_t 			*prev = parent;
    ngx_http_qkupstream_loc_conf_t 			*conf = child;

    ngx_conf_merge_msec_value(conf->upstream.connect_timeout,
                              prev->upstream.connect_timeout, 60000);

    ngx_conf_merge_msec_value(conf->upstream.send_timeout,
                              prev->upstream.send_timeout, 60000);

    ngx_conf_merge_msec_value(conf->upstream.read_timeout,
                              prev->upstream.read_timeout, 60000);
							  
    ngx_conf_merge_uint_value(conf->upstream.upstream_tries,
                              prev->upstream.upstream_tries, 0);
						
    //ngx_conf_merge_msec_value(conf->upstream.next_upstream_timeout,
    //                          prev->upstream.next_upstream_timeout, 0);

    ngx_conf_merge_size_value(conf->upstream.buffer_size,
                              prev->upstream.buffer_size,
                              8096);

    ngx_conf_merge_bitmask_value(conf->upstream.next_upstream,
                              prev->upstream.next_upstream,
                              (NGX_CONF_BITMASK_SET
                               |NGX_HTTP_UPSTREAM_FT_ERROR
                               |NGX_HTTP_UPSTREAM_FT_TIMEOUT));
						

    if (conf->upstream.next_upstream & NGX_HTTP_UPSTREAM_FT_OFF) {
        conf->upstream.next_upstream = NGX_CONF_BITMASK_SET
                                       |NGX_HTTP_UPSTREAM_FT_OFF;
    }

    if (conf->upstream.upstream == NULL) {
        conf->upstream.upstream = prev->upstream.upstream;
    }
	
    return NGX_CONF_OK;
}


static char *
ngx_http_qkupstream_pass(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{

	ngx_url_t                   		url;
	ngx_uint_t                  		n;
	ngx_str_t                  			*value;
	ngx_http_core_loc_conf_t   			*clcf;
    ngx_http_qkupstream_loc_conf_t 		*rlcf = conf;
    ngx_http_compile_complex_value_t    ccv;

    if (rlcf->upstream.upstream) {
        return "is duplicate";
    }

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);

    clcf->handler = ngx_http_qkupstream_handler;

    if (clcf->name.data[clcf->name.len - 1] == '/') {
        clcf->auto_redirect = 1;
    }

    value = cf->args->elts;

    n = ngx_http_script_variables_count(&value[1]);
    if (n) {
        rlcf->complex_target = ngx_palloc(cf->pool,
        		sizeof(ngx_http_complex_value_t));

        if (rlcf->complex_target == NULL) {
            return NGX_CONF_ERROR;
        }

        ngx_memzero(&ccv, sizeof(ngx_http_compile_complex_value_t));
        ccv.cf = cf;
        ccv.value = &value[1];
        ccv.complex_value = rlcf->complex_target;

        if (ngx_http_compile_complex_value(&ccv) != NGX_OK) {
            return NGX_CONF_ERROR;
        }

        return NGX_CONF_OK;
    }

    rlcf->complex_target = NULL;

    ngx_memzero(&url, sizeof(ngx_url_t));

    url.url = value[1];
    url.no_resolve = 1;

    rlcf->upstream.upstream = ngx_http_upstream_add(cf, &url, 0);
    if (rlcf->upstream.upstream == NULL) {
        return NGX_CONF_ERROR;
    }

    return NGX_CONF_OK;
}
