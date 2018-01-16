#include "core/qkpack_handler.h"
#include "util/filekv_parser.h"
#include "util/string_util.h"
#include "qkpack_log.h"

using namespace com::youku::data::qkpack::core;
using namespace com::youku::data::qkpack::util;


int QkPackHandler::Init(const std::string &so_conf,const std::string &backup_server,
		int mset_max_count, int batchadd_max_count, bool metrics_status)
{
	int										rc;
	qkpack_conf_t							conf;

	if( !FilekvParser::Parse(const_cast<std::string&>(so_conf),conf.conf_str) ) {
		std::cout << "Parse cofig file error:" << so_conf << std::endl;
		return QKPACK_ERROR;
	}
	
	qkpack_parser_ = new QkPackParserImpl(mset_max_count,batchadd_max_count);
	
	rc = qkpack_parser_->ParseConf(&conf);
	if ( rc != QKPACK_OK ) {
		return QKPACK_ERROR;
	}
	
	redis_cluster_ = new RedisCluster(backup_server);
	qkpack_acl_ = new QkPackACL(conf.acl_url,conf.timeout);
	
	redis_proto_   = new RedisProto(conf.add_multikv_script,conf.add_zset_script,
			conf.incrby_script,conf.add_rset_script);

	qkpack_metrics_ = new QkPackMetrics(metrics_status);

	return QKPACK_OK;
}

//timer 1 min publish
int QkPackHandler::Publish() 
{
	qkpack_metrics_->Report();
	
	return QKPACK_OK;
}


int QkPackHandler::ReinitRequest(void *data)
{
	qkpack_request_t						*request = (qkpack_request_t*)data;

	QKPACK_LOG_ERROR("qkpack handler reinit request");
	QKPACK_LOG_ERROR("qkpack handler reinit request->now_tries=[%d],request->tries=[%d]", request->now_tries,request->tries);
	QKPACK_LOG_ERROR("qkpack handler reinit node_ip=[%s]", request->node_ip.c_str());

	//get backup_server
	if ( request->now_tries == (request->tries - 1) ) {

		//metrics 
		qkpack_metrics_->UserSceneMeter(request, QKPACK_METRICS_5XX_ERROR_METER);
				
		request->kvpair.moved_ask =  QKPACK_OK;
		request->code = QKPACK_RESPONSE_CODE_5XX_TIMEOUT;
		return CreateRequestError(request);
	}

	if ( request->kvpair.moved_ask == REDIS_ASK ) {
		request->kvpair.proto_str.insert(0,"*1\r\n$6\r\nASKING\r\n");
		request->kvpair.moved_ask =  REDIS_ASKING;

		QKPACK_LOG_ERROR("qkpack handler reinit proto_str=[%s]", request->kvpair.proto_str.c_str());
	}	
	
	return QKPACK_OK;
}


int QkPackHandler::Destroy(void *data) 
{
	qkpack_request_t						*request = (qkpack_request_t*)data;
	
	QKPACK_LOG_DEBUG("qkpack handler destroy");
	
	if ( request->code != QKPACK_RESPONSE_CODE_OK ) {
		QKPACK_LOG_ERROR("request error code=[%d]", request->code);
		QKPACK_LOG_ERROR("request error desc=[%s]", request->desc.c_str());
	}

	qkpack_metrics_->TimerStop(request);

	return QKPACK_OK;
}


int QkPackHandler::CreateRequest(void *data) 
{
	int										rc;
	bool									b;
	qkpack_request_t						*request = (qkpack_request_t*)data;
	qkpack_acl_t							*acl = NULL;

	QKPACK_TIME_BEGIN(request);
	
	//解析 request body
	rc = qkpack_parser_->ParseRequestBody(request);
	if ( rc != QKPACK_OK ) {
		QKPACK_LOG_ERROR("parse request body is error");
			
		//metrics 
		qkpack_metrics_->UserSceneMeter(request, QKPACK_METRICS_JSON_PARSER_ERROR_METER);	
			
		request->code = QKPACK_RESPONSE_CODE_PARSE_ERROR;
		return CreateRequestError(request);
	}
		
	QKPACK_TIME_END(request);	
	QKPACK_LOG_DEBUG("parse request body time=[%d],",request->cost);

	//子请求
	if ( request->is_subrequest ) {
		goto subrequest;
	}

	//acl权限认证
	acl = qkpack_acl_->Process(request);
	if ( acl == NULL ) {
		QKPACK_LOG_ERROR("acl module process is error");
			
		//metrics 
		qkpack_metrics_->UserSceneMeter(request, QKPACK_METRICS_ACL_ERROR_METER);
			
		request->code = QKPACK_RESPONSE_CODE_ILLEGAL_RIGHT;
		return CreateRequestError(request);
	}
			
	QKPACK_TIME_END(request);	
	QKPACK_LOG_DEBUG("acl module process time=[%d],",request->cost);

	
	//metrics 用户场景 只埋非子请求
	qkpack_metrics_->UserSceneTimer(request);
	
	//根据key得到集群节点ip
	rc = redis_cluster_->GetNode(request, acl->route);
	if ( rc != QKPACK_OK ) {
		QKPACK_LOG_ERROR("get redis cluster node is error");
		
		//metrics 
		qkpack_metrics_->UserSceneMeter(request, QKPACK_METRICS_CLUSTER_NODES_ERROR_METER);	

		request->code = QKPACK_RESPONSE_CODE_UNKNOWN;
		return CreateRequestError(request);
	}
	
	QKPACK_TIME_END(request);	
	QKPACK_LOG_DEBUG("redis cluster getnode=[%s]",request->node_ip.c_str());
	QKPACK_LOG_DEBUG("get redis cluster node time=[%d],",request->cost);

	//检查是否需要发起子请求
	b = qkpack_parser_->CheckSubRequest(request);
	if ( b ) {
		rc = qkpack_parser_->CreateSubRequestBody(request);
		if ( rc == QKPACK_ERROR ) {
				
			QKPACK_LOG_ERROR("create sub request body is error");
			return ProcessBodyError(request);
		}
		return rc;
	}

subrequest:	
	
	//根据key得到集群节点ip
	if ( request->node_ip.empty() ) {
		rc = redis_cluster_->GetNode(request);

		if ( rc != QKPACK_OK ) {
			QKPACK_LOG_ERROR("get redis cluster node is error");
			
			//metrics 
			qkpack_metrics_->UserSceneMeter(request, QKPACK_METRICS_CLUSTER_NODES_ERROR_METER);	

			request->code = QKPACK_RESPONSE_CODE_UNKNOWN;
			return CreateRequestError(request);
		}
		
		QKPACK_TIME_END(request);	
		QKPACK_LOG_DEBUG("redis cluster getnode=[%s]",request->node_ip.c_str());
		QKPACK_LOG_DEBUG("get redis cluster node time=[%d],",request->cost);
	}

	
	//构建redis协议
	rc = redis_proto_->RedisProtoEncode(request);
	if ( rc != REDIS_OK ) {
		QKPACK_LOG_ERROR("redis proto encode is error");
		
		//metrics 
		qkpack_metrics_->UserSceneMeter(request, QKPACK_METRICS_REDIS_PROTO_ERROR_METER);	

		request->code = QKPACK_RESPONSE_CODE_UNKNOWN;
		return CreateRequestError(request);
	}

	QKPACK_TIME_END(request);	
	QKPACK_LOG_DEBUG("redis proto encode time=[%d],",request->cost);

	//metrics全局埋点
	qkpack_metrics_->GlobalSceneTimer(request);
	
	return QKPACK_OK;
}


int QkPackHandler::ProcessBody(void *data)
{
	int										rc;
	qkpack_request_t						*request = (qkpack_request_t*)data;
	bool									b;

	//检查子请求 
	b = qkpack_parser_->CheckSubRequest(request);
	if ( b ) {
		QKPACK_LOG_DEBUG("subrequest body ");
				
		//发起子请求
		rc = qkpack_parser_->CreateSubRequestBody(request);
		if ( rc == QKPACK_OK ) {
			//metrics 埋点
			qkpack_metrics_->CheckUserSceneMetricsMissMgeAndZRange(request);
		}
		else if ( rc == QKPACK_ERROR ) {
			QKPACK_LOG_ERROR("create sub request body is error");
			return ProcessBodyError(request);
		}
		return rc;
	}

	//解析redis响应的信息
	rc = redis_proto_->RedisProtoDecode(request); 
	if ( rc != REDIS_OK ) {
		//moved,ask
		if ( rc == REDIS_MOVED || rc == REDIS_ASK || rc == REDIS_TRYAGAIN ) {
	
			QKPACK_LOG_ERROR("redis proto  rc=[%d]", rc);
			
			if ( request->kvpair.moved_ask == REDIS_ASKING_END ) {
				//asking 之后继续moved，节点有问题。可能是迁移的时候，目标节点挂掉，然后起来之后的状态错误导致的原因
				QKPACK_LOG_ERROR("redis cluster asking after moved, importing node down");
				
				request->code = QKPACK_RESPONSE_CODE_REDIS_STORAGE_ERROR;
				return ProcessBodyError(request);
			}

			request->kvpair.moved_ask = (rc == REDIS_TRYAGAIN ? REDIS_MOVED : rc);
			rc = redis_cluster_->ProcessMoved(request->response_buffer, request->cluster_name, request->node_ip, rc); 
			if ( rc != QKPACK_OK ) {
				QKPACK_LOG_ERROR("redis cluster process moved is error");
				
				request->code = QKPACK_RESPONSE_CODE_REDIS_STORAGE_ERROR;
				return ProcessBodyError(request);
			}
			
			QKPACK_LOG_ERROR("redis proto  moved_ask=[%d]", request->kvpair.moved_ask);
			return QKPACK_UPSTREAM_INVALID_HEADER;
		}
		//error	
		QKPACK_LOG_ERROR("redis proto decode is error");
		return ProcessBodyError(request);
	}
	//metrics 检查get,smembers miss
	qkpack_metrics_->CheckUserSceneMetricsMissGetAndSmembers(request); 

	QKPACK_TIME_END(request);	
	QKPACK_LOG_DEBUG("redis proto decode time=[%d]",request->cost);


	//解析 response body
	rc = qkpack_parser_->ParseResponseBody(request);
	if ( rc != QKPACK_OK ) {
		QKPACK_LOG_ERROR("qkpack parser response body is error");
		return ProcessBodyError(request);
	}
	
	QKPACK_TIME_END(request);	
	QKPACK_LOG_DEBUG("parse response body time=[%d]",request->cost);

	return QKPACK_OK;
}


int QkPackHandler::CreateRequestError(qkpack_request_t *request)
{
	int								rc;
	bool							b;

	if ( request->parent ) {
		request->parent->code = request->code;
		request->parent->desc = request->desc;
	}

	QKPACK_LOG_ERROR("create request error");
	QKPACK_LOG_ERROR("create request buffer=[%s]",request->request_buffer.c_str());
	
	//检查是否需要发起子请求
	b = qkpack_parser_->CheckSubRequest(request);
	if ( b ) {
		ProcessBodyError(request);
		return QKPACK_OK;
	}

	//get backup_server
	request->node_ip = redis_cluster_->GetBackUpNode();

	QKPACK_LOG_ERROR("create request error node_ip=[%s]",request->node_ip.c_str());

	//redis ping proto
	rc = redis_proto_->RedisProtoEncode(request);
	if ( rc != QKPACK_OK ) {
		return QKPACK_ERROR;
	}
	
	QKPACK_LOG_ERROR("create request error redis_proto=[%s]",request->kvpair.proto_str.c_str());

	return QKPACK_OK;
}


int QkPackHandler::ProcessBodyError(qkpack_request_t *request)
{
	QKPACK_LOG_ERROR("process body error");
	QKPACK_LOG_ERROR("process body=[%s]",request->response_buffer.c_str());
	
	if ( request->parent ) {
		request->parent->code = request->code;
		request->parent->desc = request->desc;
	}

	qkpack_parser_->ParseResponseError(request);
	return QKPACK_OK;
}


int QkPackHandler::Exit() 
{
	if ( qkpack_parser_  ) {
		delete qkpack_parser_;
		qkpack_parser_ = NULL;
	}
	
	if ( redis_cluster_  ) {
		delete redis_cluster_;
		redis_cluster_ = NULL;
	}
	
	if ( redis_proto_ ) {
		delete redis_proto_;
		redis_proto_ = NULL;
	}
	
	if ( qkpack_acl_ ) {
		delete qkpack_acl_;
		qkpack_acl_ = NULL;
	}

	if ( qkpack_metrics_ ) {
		delete qkpack_metrics_;
		qkpack_metrics_ = NULL;
	}

	return QKPACK_OK;
}


