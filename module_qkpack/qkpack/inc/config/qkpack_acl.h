#ifndef __QKPACK_ACL_H__
#define __QKPACK_ACL_H__

#include "common.h"
#include "parser/qkpack_yajl_json.h"

using namespace com::youku::data::qkpack::parser;

namespace com { 
namespace youku { 
namespace data {
namespace qkpack {
namespace config {


class QkPackACL 
{
public:
	QkPackACL(std::string acl_url,int timeout=5);
	virtual ~QkPackACL();

public:
	/**
     * 
     * Process ACL Module
    */
	qkpack_acl_t* Process(qkpack_request_t *request);

protected:
	/**
     * 
     * Set NameSpance
    */
	int SetNameSpace(qkpack_request_t *request);
	
	/**
     * 
     * Set ClusterName
    */
	int SetClusterName(qkpack_request_t *request);
	
	/**
     * 
     * Set ACL Info
    */
	qkpack_acl_t* SetACL(const std::string &ak, const std::string &uri);
	
	/**
     * 
     * Get ZsetLimit
    */
	int GetZsetLimit(qkpack_request_t *request, const std::vector<limit_policies_t>	&vector_policies);
	
	/**
     * 
     * Check interface timeout
    */
	int CheckInterfaceTimeout(qkpack_request_t *request, const std::vector<timeouts_t> &vector_timeouts);

	/**
     * 
     * Check interface timeout
    */
	int CheckInterfaceLimitsLength(qkpack_request_t *request, const std::vector<limits_length_t> &vector_limits_len);

	/**
     * 
     * Get NodeList 
    */
	int GetNodeList(qkpack_request_t *request,qkpack_acl_t *acl);
	
	/**
     * 
     * Get Curl Response 
    */
	int GetCurlResponse(qkpack_acl_t *acl,const std::string &ak, const std::string &uri);
	
private:
	std::string	acl_url_;
	int timeout_;

	std::map<std::string, qkpack_acl_t*> map_acl_;
	QkPackYajlJSON *qkpack_yajl_json_;
};

} /* namespace config */
} /* namespace qkpack */
} /* namespace data */
} /* namespace youku */
} /* namespace com */

#endif //__QKPACK_ACL_H__
