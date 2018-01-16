#ifndef __QKPACK_SUBREQUEST_H__
#define __QKPACK_SUBREQUEST_H__

#include "common.h"
#include "parser/qkpack_yajl_json_impl.h"

namespace com { 
namespace youku { 
namespace data {
namespace qkpack {
namespace parser {


class QkPackSubRequest
{
public:
    QkPackSubRequest();
	virtual ~QkPackSubRequest();
	
public:

	/**
	 * 
	 * 检查是否是子请求
	 * 
	*/
	bool CheckSubRequest(qkpack_request_t *request);
	
	/**
	 * 
	 * 创建子请求
	 * 
	*/
	int CreateSubRequestBody(qkpack_request_t *request);
	
protected:

	/**
	 * 
	 * 检查子请求状态
	 * 
	*/
	int CheckSubRequestStatus(qkpack_request_t *request);
	
	
	/**
	 * 
	 * MSET子请求
	 * 
	*/
	int CreateSubRequestMset(qkpack_request_t *request);
	
	
	/**
	 * 
	 * MGET子请求
	 * 
	*/
	int CreateSubRequestMget(qkpack_request_t *request);
	
	
	/**
	 * 
	 * ZFIXEDSETADD子请求
	 * 
	*/
	int CreateSubRequestZfixedsetAdd(qkpack_request_t *request);
	
	
	/**
	 * 
	 * ZFIXEDSET_BATCHADD子请求
	 * 
	*/
	int CreateSubRequestZfixedsetBatchAdd(qkpack_request_t *request);
	
	
	/**
	 * 
	 * ZFIXEDSET_GETBYSCORE子请求
	 * 
	*/
	int CreateSubRequestZfixedsetGetByScore(qkpack_request_t *request);
	
	
	/**
	 * 
	 * 过滤member null value
	 * 
	*/
	void FilterGetByScoreMemberNullValue(qkpack_request_t *request);
	
	
	/**
	 * 
	 * 查找GetByScoreMembers的Value
	 * 
	*/
	bool FindGetByScoreMembersValue(const std::string &key, std::string &value, std::vector<kvpair_t> &kvpair);
	
	
	/**
	 * 
	 * ZFIXEDSET_BATCHGETSCORE
	 * 
	*/
	int CreateSubRequestZfixedsetBatchGetByScore(qkpack_request_t *request);
	
	
	/**
	 * 
	 * 设置子请求
	 * 
	*/
	void SetSubRequest(qkpack_request_t *sub,const std::string &request_uri,const std::string &ak, const std::string &uri);
	
	
	/**
	 * 
	 * Metrics埋点(MGET,ZRANGE)Miss
	 * 
	*/
	void MetricsMiss(qkpack_request_t *request);

private:
	QkPackYajlJSONImpl *qkpack_yajl_json_;
};

} /* namespace parser */
} /* namespace qkpack */
} /* namespace data */
} /* namespace youku */
} /* namespace com */

#endif /* __QKPACK_SUBREQUEST_H__ */
