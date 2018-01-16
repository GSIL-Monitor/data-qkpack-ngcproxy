#ifndef __QKPACK_HANDLER_H__
#define __QKPACK_HANDLER_H__

#include "common.h"
#include "core/redis_cluster.h"
#include "core/qkpack_metrics.h"
#include "parser/qkpack_parser_impl.h"
#include "protocol/redis_proto.h"
#include "config/qkpack_acl.h"
#include "metrics/metrics.h"

using namespace com::youku::data::qkpack::parser;
using namespace com::youku::data::qkpack::protocol;
using namespace com::youku::data::qkpack::config;
using namespace com::youku::data::qkpack::metrics;


namespace com { 
namespace youku { 
namespace data {
namespace qkpack {
namespace core {


class QkPackHandler 
{
public:
    QkPackHandler(){};
    virtual ~QkPackHandler(){};

public:
	/**
     * 
     * Initialize Work Process
    */
	int Init(const std::string &so_conf,const std::string &backup_server,
			int mset_max_count, int batchadd_max_count, bool metrics_status);
	
	/**
     * 
     * Exit Work Process
    */
    int Exit();
	
	/**
     * 
     * Create Request Body
    */
	int CreateRequest(void *data);
	
	/**
     * 
     * Process Response Body
    */
	int ProcessBody(void *data);
	
	/**
     * 
     * Process Reinit Request
    */
	int ReinitRequest(void *data);


	/**
     * 
     * Destroy
    */
    int Destroy(void *data);
	
	/**
     * 
     * 1 min Publish
    */
	int Publish();

protected:
	/**
     * 
     * Create Request Error
    */
	int CreateRequestError(qkpack_request_t *request);
	
	/**
     * 
     * Process Response Body Error
    */
	int ProcessBodyError(qkpack_request_t *request);

private:
	QkPackParserImpl     *qkpack_parser_;
	RedisCluster     *redis_cluster_;
	RedisProto	     *redis_proto_;
	QkPackACL	     *qkpack_acl_;
	QkPackMetrics	 *qkpack_metrics_;

};

} /* namespace core */
} /* namespace qkpack */
} /* namespace data */
} /* namespace youku */
} /* namespace com */

#endif /* __QKPACK_HANDLER_H__ */
