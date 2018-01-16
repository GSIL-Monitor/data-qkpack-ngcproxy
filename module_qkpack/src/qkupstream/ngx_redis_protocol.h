/*
 * Author: lihang <lihang01@youku.com>
 *
 * Create Date: 2015-08
 *
 */

#ifndef NGX_REDIS_PROTOCOL_H
#define NGX_REDIS_PROTOCOL_H

#include <ngx_config.h>
#include <ngx_core.h>


ngx_int_t ngx_redis_proto_decode(char *rdata, ssize_t rdata_len);


#endif //NGX_REDIS_PROTOCOL_H
