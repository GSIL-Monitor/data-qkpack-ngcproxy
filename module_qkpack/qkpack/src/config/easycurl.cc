#include "config/easycurl.h"
#include "qkpack_log.h"

using namespace com::youku::data::qkpack::config;

int EasyCurl::writeCallback(void *ptr, size_t size, size_t nmemb, void *usrptr) 
{
	EasyCurl					*p = (EasyCurl*)usrptr;

	p->response_.append((char *)ptr, size * nmemb);
	return size * nmemb;
}

EasyCurl::EasyCurl(): curl_(NULL), code_(0) 
{
	int							res = curl_global_init(CURL_GLOBAL_ALL);
	
	if( res != 0 ){
		QKPACK_LOG_ERROR("curl_global_init error \n");
	}

	curl_ = curl_easy_init();
	if( curl_ == NULL ){
		
		QKPACK_LOG_ERROR("cur_easy_init error \n");
		return;
	}

	curl_easy_setopt(curl_, CURLOPT_NOSIGNAL, 1);  
	curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, writeCallback); 
	curl_easy_setopt(curl_, CURLOPT_WRITEDATA, this); 
	curl_easy_setopt(curl_, CURLOPT_ERRORBUFFER, error_);

	//default timeout
	curl_easy_setopt(curl_, CURLOPT_CONNECTTIMEOUT_MS, 10000);
	curl_easy_setopt(curl_, CURLOPT_TIMEOUT_MS, 10000);
}


EasyCurl::~EasyCurl() 
{
	if ( curl_ ) {
		
		curl_easy_cleanup(curl_); 
		curl_global_cleanup(); 
	}
}


int EasyCurl::Get(const std::string& url, int timeout) 
{
	if ( curl_ == NULL ) {
		return QKPACK_ERROR;
	}
	
	curl_easy_setopt(curl_, CURLOPT_HTTPGET, 1);
	
	if( timeout > 0 ) {
		
		curl_easy_setopt(curl_, CURLOPT_TIMEOUT_MS, timeout);
		curl_easy_setopt(curl_, CURLOPT_CONNECTTIMEOUT_MS, timeout);
	}
	
	curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());    

	response_.clear();
	code_ = curl_easy_perform(curl_);

	if ( code_ != CURLE_OK ) {
		
		QKPACK_LOG_ERROR("curl post is error code=[%d]",code_);
		return QKPACK_ERROR;
	}
	
	return QKPACK_OK;
}


int EasyCurl::Post(const std::string& url,const std::vector<std::string> headers,const std::string& data,int timeout) 
{
	struct	curl_slist						*slist = 0;
	
	if ( curl_ == NULL ) {
		return QKPACK_ERROR;
	}
	
	curl_easy_setopt(curl_, CURLOPT_POST, 1);
	
	if( timeout > 0 ) {
		curl_easy_setopt(curl_, CURLOPT_TIMEOUT_MS, timeout);
		curl_easy_setopt(curl_, CURLOPT_CONNECTTIMEOUT_MS, timeout);
	}

	curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
	
	if ( headers.size() ) {
		
		for ( size_t i = 0; i < headers.size(); ++i) {
			slist = curl_slist_append(slist, headers[i].c_str());
		}
		curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, slist);
	}
	
	if ( !data.empty() ) {
		curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, data.c_str());
		curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, data.size());
	}
	
	response_.clear();
	code_ = curl_easy_perform(curl_);
	
	if ( slist ) {
		curl_slist_free_all(slist);
	}
	
	if ( code_ != CURLE_OK ) {
		QKPACK_LOG_ERROR("curl post is error code=[%d]",code_);
		return QKPACK_ERROR;
	}
	
	return QKPACK_OK;
}

