/*
 * Author: lihang <lihang01@youku.com>
 *
 * Create Date: 2015-08
 *
 */

#include "ngx_redis_protocol.h"

static size_t
ngx_get_num_size(uint64_t i)
{
	size_t          n = 0;

	do {
		i = i / 10;
		n++;
	} while (i > 0);

	return n;
}

static ngx_int_t 
ngx_parse_bulk(char *rdata, ssize_t rdata_len) 
{
	//$3\r\nabc\r\n
	int 							len = 1;
	int								bulklen;
	char							*rest;
	
	bulklen = strtol(rdata, &rest, 0);
	if ( bulklen == -1 ) {
		return NGX_OK;
	}
	
	len += ngx_get_num_size(bulklen);
	rest += 2, len += 2;
	rest += (bulklen + 2), len += (bulklen + 2);
	
	if ( len > rdata_len ) {
		return NGX_AGAIN;
	}
	
	return NGX_OK;
}

static ngx_int_t 
ngx_parse_multibulk(char *rdata, ssize_t rdata_len) 
{
	//*3\r\n$3\r\nSET\r\n$5\r\nmykey\r\n$7\r\nmyvalue\r\n
	int 							len = 1;
	int 							multibulkSize,i,bulklen;
	char							*rest;

	multibulkSize = strtol(rdata, &rest, 0);

	len += 2;
	rest += 2;
	i = 0;
	while (i < multibulkSize)       /* For each element in multibulk data */
	{
		if ( len > rdata_len ) {
			return NGX_AGAIN;
		}
		
		rest++,len++;                           /* Step through $ */
		bulklen = strtol(rest, &rest, 0); /* Get an element length */                      
		if (bulklen == -1)                /* If the element length is -1 then element is NULL */
		{
		  i++;
		  continue;
		}
		
		len += ngx_get_num_size(bulklen);
		rest += 2, len += 2;   									/* Step through "\r\n" */
		rest += (bulklen + 2), len += (bulklen + 2);            /* Jump to the end of data bypassing "\r\n" */
		
		i++;
	}
	
	return NGX_OK;
}



ngx_int_t 
ngx_redis_proto_decode(char *rdata, ssize_t len)
{
	switch (*rdata)
	{
		case '-' : 
		case '+' : 
		case ':' : 
			return NGX_OK;
		case '$' : 
			return ngx_parse_bulk(++rdata, len);
		case '*' : 
			return ngx_parse_multibulk((char*)++rdata, len);
		default  : 
			return NGX_ERROR;
	}
}


