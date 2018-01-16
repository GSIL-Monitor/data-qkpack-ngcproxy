#include "ngx_qkpack_manager_module.h"

static void *ngx_qkpack_manager_create_conf(ngx_conf_t *cf);
static char *ngx_qkpack_manager_merge_conf(ngx_conf_t *cf, void *parent,
    void *child);	
static ngx_int_t ngx_qkpack_manager_prepare(ngx_cycle_t *cycle);
static ngx_int_t ngx_qkpack_manager_process_init(ngx_cycle_t *cycle);
static ngx_int_t ngx_qkpack_manager_loop(ngx_cycle_t *cycle);
static void ngx_qkpack_manager_process_exit(ngx_cycle_t *cycle);

//static void ngx_qkpack_manager_expire(ngx_event_t *event);

static void ngx_qkpack_manager_accept(ngx_event_t *ev);
//static void ngx_qkpack_manager_read(ngx_event_t *ev);
//static void ngx_qkpack_manager_write(ngx_event_t *ev);

static ngx_command_t ngx_qkpack_manager_commands[] = {

    { ngx_string("listen"),
      NGX_PROC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_num_slot,
      NGX_PROC_CONF_OFFSET,
      offsetof(ngx_qkpack_manager_conf_t, port),
      NULL },

    { ngx_string("interval"),
      NGX_PROC_CONF|NGX_CONF_FLAG,
      ngx_conf_set_msec_slot,
      NGX_PROC_CONF_OFFSET,
      offsetof(ngx_qkpack_manager_conf_t, interval),
      NULL },

      ngx_null_command
};


static ngx_proc_module_t ngx_qkpack_manager_module_ctx = {
    ngx_string("qkpack_manager"),            /* name                     */
    NULL,                                    /* create main configration */
    NULL,                                    /* init main configration   */
    ngx_qkpack_manager_create_conf,     /* create proc configration */
    ngx_qkpack_manager_merge_conf,      /* merge proc configration  */
    ngx_qkpack_manager_prepare,         /* prepare                  */
    ngx_qkpack_manager_process_init,    /* process init             */
    ngx_qkpack_manager_loop,            /* loop cycle               */
    ngx_qkpack_manager_process_exit     /* process exit             */
};


ngx_module_t ngx_qkpack_manager_module = {
    NGX_MODULE_V1,
    &ngx_qkpack_manager_module_ctx,
    ngx_qkpack_manager_commands,
    NGX_PROC_MODULE,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NGX_MODULE_V1_PADDING
};


static void *
ngx_qkpack_manager_create_conf(ngx_conf_t *cf)
{
    ngx_qkpack_manager_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_qkpack_manager_conf_t));
    if (conf == NULL) {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                           "[qkpack_manager] create proc conf error");
        return NULL;
    }

    conf->enable = 1;
    conf->port = NGX_CONF_UNSET_UINT;
    conf->interval = NGX_CONF_UNSET_MSEC;

    return conf;
}


static char *
ngx_qkpack_manager_merge_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_qkpack_manager_conf_t  *prev = parent;
    ngx_qkpack_manager_conf_t  *conf = child;

    ngx_conf_merge_value(conf->enable, prev->enable, 1);
    ngx_conf_merge_uint_value(conf->port, prev->port, 1);
    ngx_conf_merge_msec_value(conf->interval, prev->interval, 3000);

    return NGX_CONF_OK;
}


static ngx_int_t
ngx_qkpack_manager_prepare(ngx_cycle_t *cycle)
{
    ngx_qkpack_manager_conf_t *ptmcf;

    ptmcf = ngx_proc_get_conf(cycle->conf_ctx, ngx_qkpack_manager_module);
    if (!ptmcf->enable) {
        return NGX_DECLINED;
    }

    return NGX_OK;
}


static ngx_int_t
ngx_qkpack_manager_process_init(ngx_cycle_t *cycle)
{
    int                             reuseaddr;
    ngx_event_t                    *rev;//*wev, *expire;
    ngx_socket_t                    fd;
    ngx_connection_t               *c;
    struct sockaddr_in              sin;
    ngx_qkpack_manager_conf_t *conf;

	if ( ngx_exiting ) {
        ngx_log_error(NGX_LOG_ERR, cycle->log, 0, "[qkpack_manager] ngx_qkpack_manager_process_init exiting");
		return NGX_OK;
	}

    conf = ngx_proc_get_conf(cycle->conf_ctx, ngx_qkpack_manager_module);

    fd = ngx_socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        ngx_log_error(NGX_LOG_ERR, cycle->log, 0, "[qkpack_manager] ngx_qkpack_manager_process_init ngx_socket error");
        return NGX_ERROR;
    }

    reuseaddr = 1;

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
                   (const void *) &reuseaddr, sizeof(int))
        == -1)
    {
        ngx_log_error(NGX_LOG_EMERG, cycle->log, ngx_socket_errno,
                      "[qkpack_manager] ngx_qkpack_manager_process_init setsockopt(SO_REUSEADDR) failed");

        ngx_close_socket(fd);
        return NGX_ERROR;
    }

    if (ngx_nonblocking(fd) == -1) {
        ngx_log_error(NGX_LOG_EMERG, cycle->log, ngx_socket_errno,
                      "[qkpack_manager] ngx_qkpack_manager_process_init ngx_nonblocking failed");

        ngx_close_socket(fd);
        return NGX_ERROR;
    }

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(conf->port);

    if (bind(fd, (struct sockaddr *) &sin, sizeof(sin)) == -1) {
        ngx_log_error(NGX_LOG_ERR, cycle->log, 0, "[qkpack_manager] ngx_qkpack_manager_process_init bind error");
        return NGX_ERROR;
    }

    if (listen(fd, 20) == -1) {
        ngx_log_error(NGX_LOG_ERR, cycle->log, 0, "[qkpack_manager] ngx_qkpack_manager_process_init listen error");
        return NGX_ERROR;
    }

    c = ngx_get_connection(fd, cycle->log);
    if (c == NULL) {
        ngx_log_error(NGX_LOG_ERR, cycle->log, 0, "[qkpack_manager] ngx_qkpack_manager_process_init ngx_get_connection no connection");
        return NGX_ERROR;
    }

    c->log = cycle->log;
    rev = c->read;
    rev->log = c->log;
    rev->accept = 1;
    rev->handler = ngx_qkpack_manager_accept;

    if (ngx_add_event(rev, NGX_READ_EVENT, 0) == NGX_ERROR) {
        return NGX_ERROR;
    }

    conf->fd = fd;
    
    /*
	c->log = cycle->log;
	wev = c->write;
	wev->log = c->log;
	wev->ready = 1;
	wev->handler = ngx_qkpack_manager_write;

	if (ngx_add_event(wev, NGX_WRITE_EVENT, 0) == NGX_ERROR) {
		return NGX_ERROR;
	}

  conf->fd = fd;
	
    expire = &conf->expire_event;
    expire->handler = ngx_qkpack_manager_expire;
    expire->log = cycle->log;
    expire->data = conf;
    expire->timer_set = 0;
    
    //ngx_add_timer(expire, conf->interval);
*/

    return NGX_OK;
}

/*
static void
ngx_qkpack_manager_expire(ngx_event_t *event)
{
	ngx_qkpack_manager_conf_t *conf;
	conf = event->data;
	
	ngx_log_error(NGX_LOG_EMERG, event->log, 0,"[qkpack_manager] ngx_qkpack_manager_expire");

	//ngx_add_timer(event, conf->interval);
}
*/

static ngx_int_t
ngx_qkpack_manager_loop(ngx_cycle_t *cycle)
{
    return NGX_OK;
}


static void
ngx_qkpack_manager_process_exit(ngx_cycle_t *cycle)
{
    ngx_qkpack_manager_conf_t *conf;

    conf = ngx_proc_get_conf(cycle->conf_ctx, ngx_qkpack_manager_module);

    if (conf->fd) {
        ngx_close_socket(conf->fd);
    }

    if (conf->expire_event.timer_set) {
        //ngx_del_timer(&conf->expire_event);
    }
}



static void
ngx_qkpack_manager_accept(ngx_event_t *ev)
{
	u_char                sa[NGX_SOCKADDRLEN];
	socklen_t             socklen;
	ngx_connection_t     *lc;//*c;
	//ngx_event_t                    *rev;
	ngx_socket_t          s;

	socklen = NGX_SOCKADDRLEN;    
	lc = ev->data;

	//ngx_log_error(NGX_LOG_EMERG, ev->log, 0,"[qkpack_manager] ngx_qkpack_manager_accept");

	s = accept(lc->fd, (struct sockaddr *) sa, &socklen);
	if (s == -1) {
		return;
	}

	if (ngx_nonblocking(s) == -1) {
		ngx_close_socket(s);
	}

	ngx_str_t             output = ngx_string("test");
	ngx_write_fd(s, output.data, output.len);

	ngx_close_socket(s);
	
	/*
	c = ngx_get_connection(s, ev->log);
	if (c == NULL) {
		ngx_close_socket(s);
		return;
	}

	c->log = ev->log;
	rev = c->read;
	rev->log = c->log;
	rev->accept = 1;
	rev->handler = ngx_qkpack_manager_read;

	if (ngx_add_event(rev, NGX_READ_EVENT, 0) == NGX_ERROR) {
		ngx_close_socket(s);
		return;
	}
	*/
}

/*
static void
ngx_qkpack_manager_read(ngx_event_t *ev)
{
    ngx_connection_t      *lc;
    lc = ev->data;
	
	//ngx_log_error(NGX_LOG_EMERG, ev->log, 0,"[qkpack_manager] ngx_qkpack_manager_read");
	
	int done = 0;
	ssize_t count = 0;

	char buf[READ_BUFFER];
	ssize_t readline = 0;

	bzero(buf, MAX_LINE);

	while(1) {
		count = read(lc->fd,buf,MAX_LINE);
		if(count == -1){
			if(errno == EAGAIN){
				done = 1;
				break;
			}
			else if (errno == ECONNRESET){
				 ngx_close_socket(lc->fd);
				 lc->fd = -1;
				 ngx_log_error(NGX_LOG_EMERG, ev->log, 0,"[qkpack_manager] ngx_qkpack_manager_read counterpart send out RST\n");
				 break;
			}
			else if(errno == EINTR){
				continue;
			}
			else{
				ngx_close_socket(lc->fd);
				lc->fd = -1;
				ngx_log_error(NGX_LOG_EMERG, ev->log, 0,"[qkpack_manager] ngx_qkpack_manager_read unrecovable error\n");
				break;
			}

		}
		else if(count == 0){
			ngx_close_socket(lc->fd);
			lc->fd = -1;
			ngx_log_error(NGX_LOG_EMERG, ev->log, 0,"[qkpack_manager] ngx_qkpack_manager_read ounterpart has shut off\n");
			break;
		}
		if(count >0)
			readline += count;
	}
	
	if(!done) {
		return;
	}
	
	//ngx_log_error(NGX_LOG_EMERG, ev->log, 0,"[qkpack_manager] ngx_qkpack_manager_read buf=[%s]\n",buf);

	ngx_write_fd(lc->fd,buf,readline);
	//ngx_shm_string_parser(ev,lc->fd,buf,readline);
}


static void
ngx_qkpack_manager_write(ngx_event_t *ev)
{
    ngx_str_t             output = ngx_string("test");
	ngx_connection_t      *lc;
    lc = ev->data;
   
   //ngx_log_error(NGX_LOG_EMERG, ev->log, 0,"[qkpack_manager] ngx_qkpack_manager_write\n");
   
   //todo
    ngx_write_fd(lc->fd, output.data, output.len);

    ngx_close_socket(lc->fd);
}
*/
