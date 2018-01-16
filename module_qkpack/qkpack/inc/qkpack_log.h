#ifndef __QKPACK_LOG_H__
#define __QKPACK_LOG_H__

#include "common.h"
#include "util/log.h"

using namespace com::youku::data::qkpack::util;

#define  LOG4CPP_CONF		"log4cpp_conf"
#define  LOGGER_NAME		"qkpack"

#define QKPACK_LOG_DEBUG(format, args...)
	//LOG(LOG_LEVEL_DEBUG, Log4cppWrapper::GetLog(LOGGER_NAME),format, ##args)
	//ngx_log_error(NGX_LOG_DEBUG,  (ngx_log_t *)log, 0, format, ##args)

#define QKPACK_LOG_ERROR(format, args...)
	//LOG(LOG_LEVEL_ERROR, Log4cppWrapper::GetLog(LOGGER_NAME), format, ##args) 
    //ngx_log_error(NGX_LOG_ERR,  (ngx_log_t *)log, 0, format, ##args)

#define QKPACK_TIME_BEGIN(request)		\
	gettimeofday(&request->tv_begin, NULL);


#define QKPACK_TIME_END(request)		\
	gettimeofday(&request->tv_end, NULL); \
	request->cost = 1000000*(request->tv_end.tv_sec-request->tv_begin.tv_sec)+  \
			(request->tv_end.tv_usec-request->tv_begin.tv_usec);

#define QKPACK_TIME_MS_END(request)		\
	gettimeofday(&request->tv_end, NULL); \
	request->cost = 1000*(request->tv_end.tv_sec-request->tv_begin.tv_sec)+ \
			(request->tv_end.tv_usec-request->tv_begin.tv_usec)/1000;


#endif  // __QKPACK_LOG_H__ 
