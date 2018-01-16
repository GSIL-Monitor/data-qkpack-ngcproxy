/*
 * Author: lihang <lihang01@youku.com>
 *
 * Create Date: 2015-08
 *
 */

#ifndef __NGX_QKPACK_MANAGER_MODULE_H__
#define __NGX_QKPACK_MANAGER_MODULE_H__

#include <ngx_event.h>
#include <ngx_core.h>
#include <ngx_config.h>

#define MAX_LINE 1024
#define READ_BUFFER 81920

typedef struct {
    ngx_flag_t                       enable;
    ngx_uint_t                       port;
    ngx_msec_t                       interval;;

    ngx_int_t 						 load_average;
    ngx_int_t   					 mem_size;
	
    ngx_socket_t                     fd;
    ngx_event_t                      expire_event;
    ngx_str_t                        shm_name;
} ngx_qkpack_manager_conf_t;





#endif /* __NGX_QKPACK_MANAGER_MODULE_H__ */
