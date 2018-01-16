#ifndef	_LOG_H
#define _LOG_H

#include <iostream>
#include "util/log4cpp_wrapper.h"

namespace com { 
namespace youku { 
namespace data {
namespace qkpack {
namespace util {

#define LOG_CONFIG_AFTER_CONFIGURE() do {       \
        try {                                                           \
            Log4cppWrapper::InitAfterLogr();              \
        } catch(std::exception &e) {                                    \
            std::cerr << "Error!!! Failed to configure logger"          \
                      << e.what() << std::endl;                         \
            exit(-1);                                                   \
        }                                                               \
    }while(0)

#define LOG_CONFIG(filename) do {                                       \
    try {                                                               \
        Log4cppWrapper::Init(filename);                       \
    } catch(std::exception &e) {                                        \
        std::cerr << "Error!!! Failed to configure logger"              \
                  << e.what() << std::endl;                             \
        exit(-1);                                                       \
    }                                                                   \
    }while(0)



#define LOG_SHUTDOWN() Log4cppWrapper::Shutdown()
#define LOG_SHUTDOWN_BEFORE() Log4cppWrapper::ShutdownBeforeLogr()
enum SHARELIB_LOG_LEVEL{
  LOG_LEVEL_DEBUG,
  LOG_LEVEL_INFO,
  LOG_LEVEL_NOTICE,
  LOG_LEVEL_WARN,
  LOG_LEVEL_ERROR
};

#define NEW_FORMAT(format)                                              \
    char buffer[256];                                                   \
    bzero(buffer, 256);                                                 \
    sprintf(buffer, "(%s:%s:%d,%lu)",  __FILE__,  __FUNCTION__ , __LINE__ ,pthread_self() ); \
    std::string newformat = std::string("%s ") + format

#define LOG(log_level, logger, format, ...) do {                        \
        switch(log_level) {                                             \
        case LOG_LEVEL_DEBUG:                                           \
        {                                                               \
            NEW_FORMAT(format);                                         \
            (logger)->Debug(newformat.c_str(), buffer, ##__VA_ARGS__);  \
            break;                                                      \
        }                                                               \
        case LOG_LEVEL_INFO:                                            \
        {                                                               \
            NEW_FORMAT(format);                                         \
            (logger)->Info(newformat.c_str(), buffer, ##__VA_ARGS__);   \
            break;                                                      \
        }                                                               \
        case LOG_LEVEL_NOTICE:                                          \
        {                                                               \
            NEW_FORMAT(format);                                         \
            (logger)->Notice(newformat.c_str(), buffer, ##__VA_ARGS__); \
            break;                                                      \
        }                                                               \
        case LOG_LEVEL_WARN:                                            \
        {                                                               \
            NEW_FORMAT(format);                                         \
            (logger)->Warn(newformat.c_str(), buffer, ##__VA_ARGS__);   \
            break;                                                      \
        }                                                               \
        case LOG_LEVEL_ERROR:                                           \
        {                                                               \
            NEW_FORMAT(format);                                         \
            (logger)->Error(newformat.c_str(), buffer, ##__VA_ARGS__);  \
            break;                                                      \
        }                                                               \
        default:                                                        \
        {                                                               \
            NEW_FORMAT(format);                                         \
            (logger)->Debug(newformat.c_str(), buffer, ##__VA_ARGS__);  \
            break;                                                      \
        }                                                               \
        }                                                               \
    }while(0)




#define LOG_INITIAL(log_level, logger, format, ...) do {                \
        switch(log_level) {                                             \
        case LOG_LEVEL_DEBUG:                                           \
            (logger)->Debug(format, ##__VA_ARGS__);                     \
            break;                                                      \
        case LOG_LEVEL_INFO:                                            \
            (logger)->Info(format, ##__VA_ARGS__);                      \
            break;                                                      \
        case LOG_LEVEL_NOTICE:                                          \
            (logger)->Notice(format, ##__VA_ARGS__);                    \
            break;                                                      \
        case LOG_LEVEL_WARN:                                            \
            (logger)->Warn(format, ##__VA_ARGS__);                      \
            break;                                                      \
        case LOG_LEVEL_ERROR:                                           \
            (logger)->Error(format, ##__VA_ARGS__);                     \
            break;                                                      \
        default:                                                        \
            (logger)->Debug(format, ##__VA_ARGS__);                     \
            break;                                                      \
        }                                                               \
    }while(0)


#endif 

} /* namespace util */
} /* namespace qkpack */
} /* namespace data */
} /* namespace youku */
} /* namespace com */

