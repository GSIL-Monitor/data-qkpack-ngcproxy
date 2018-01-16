#ifndef __EASYCURL_H__
#define __EASYCURL_H__

#include "qkpack_common.h"
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

namespace com { 
namespace youku { 
namespace data {
namespace qkpack {
namespace config {


class EasyCurl 
{
public:
	EasyCurl();
	virtual ~EasyCurl();
	
public:
	/**
	 * 
	 * Request Get
	 * 
	*/
	int Get(const std::string& url, int timeout = 0);
	
	
	/**
	 * 
	 * Request Post
	 * 
	*/
	int Post(const std::string& url,const std::vector<std::string> headers,const std::string& data,int timeout = 0);

	
	/**
	 * 
	 * GetResponse
	 * 
	*/
	const std::string& GetResponse() const { return response_; }
	
	
	/**
	 * 
	 * Error
	 * 
	*/
	const char *  Error() const { return error_; };

private:
	// no copyable
	EasyCurl(const EasyCurl&);              
	EasyCurl& operator=(const EasyCurl&);

	static int writeCallback(void *ptr, size_t size, size_t nmemb, void *usrptr);

	CURL* curl_;
	int  code_;
	char error_[CURL_ERROR_SIZE];      //errmsg, null terminate
	std::string response_;
};

} /* namespace config */
} /* namespace qkpack */
} /* namespace data */
} /* namespace youku */
} /* namespace com */

#endif //__EASYCURL_H__
