#include "parser/qkpack_parser_impl.h"

#include "core/qkpack_metrics.h"
#include "util/string_util.h"
#include "core/redis_slot.h"
#include "qkpack_log.h"

using namespace com::youku::data::qkpack::parser;
using namespace com::youku::data::qkpack::core;
using namespace com::youku::data::qkpack::util;

QkPackParserImpl::QkPackParserImpl(int mset_max_count, int batchadd_max_count):
	mset_max_count_(mset_max_count),
	batchadd_max_count_(batchadd_max_count)
{
}

QkPackParserImpl::~QkPackParserImpl() 
{
}


/**
 * 
 * 过滤member null value
 * 
*/
void QkPackParserImpl::FilterGetByScoreMemberNullValue(qkpack_request_t *request)
{
		std::map<std::string,std::vector<kvpair_t> >				&map_kv = request->map_kv;
		std::map<std::string,std::vector<kvpair_t> >::iterator		it_map = map_kv.begin();
	
		for ( ; it_map != map_kv.end(); ) {
		
			std::vector<kvpair_t> &vector_kv = it_map->second;
			std::vector<kvpair_t>::iterator it = vector_kv.begin();
	
			for ( ; it != vector_kv.end(); ) {
			
				if ( (*it).value.compare("null") == 0 ) {
				
				//if ( FindGetByScoreMembersValue((*it).member,(*it).value, request) ) {
					it = vector_kv.erase(it++);
				}
				else {
					++it;
				}
			}
			if ( vector_kv.size() == 0 ) {
				map_kv.erase(it_map++);
			}else {
				++it_map;
			}
		}
		return;
}


/**
 * 
 * 设置子请求
 * 
*/
void QkPackParserImpl::SetSubRequest(qkpack_request_t *sub,const std::string &request_uri,const std::string &ak, const std::string &uri)
{
	sub->is_subrequest = 1;
	sub->request_uri = request_uri;
	sub->request_uri.append("/sub");
	sub->args = "is_subrequest=1";
	sub->ak = ak;
	sub->uri = uri;
}


/**
 * 
 * MGET子请求
 * 
*/
int QkPackParserImpl::CreateSubRequestMget(qkpack_request_t *request)
{
	int												rc;

	if ( request->phase == QKPACK_SUBREQUEST_START ) {
		QKPACK_LOG_DEBUG("create subrequest mget QKPACK_SUBREQUEST_START");

		request->phase = QKPACK_SUBREQUEST_MGET;
	}

	//发送command mget
	if ( request->phase == QKPACK_SUBREQUEST_MGET ) {
		QKPACK_LOG_DEBUG("create subrequest mget QKPACK_SUBREQUEST_MGET");

		std::map<int, std::vector<kvpair_t*> >			&map_slots = request->map_slots_kv;
		std::map<int, std::vector<kvpair_t*> >::iterator it;

		for ( it = map_slots.begin(); it != map_slots.end(); ++it ) {
			
			qkpack_request_t sub;
			SetSubRequest(&sub,QKPACK_URI_MGET,request->ak,request->uri);
			
			sub.args.append("&slotid=");
			sub.args.append(StringUtil::ToString(it->first));

			request->subrequest.push_back(sub);
		}

		request->phase = QKPACK_SUBREQUEST_DONE;
		return QKPACK_AGAIN;
	}

	//子请求发送完毕
	if ( request->phase == QKPACK_SUBREQUEST_DONE ) {
		QKPACK_LOG_DEBUG("create subrequest mget QKPACK_SUBREQUEST_DONE");
		
		//检查子请求状态
		rc = CheckSubRequestStatus(request);
		if ( rc != QKPACK_OK ) {
			QKPACK_LOG_ERROR("CheckSubRequestS is error");
			return QKPACK_ERROR;
		}
	
		QKPACK_TIME_MS_END(request);	

		return qkpack_yajl_json_->WriterJsonMget(request);
	}

	return QKPACK_ERROR;
}


int QkPackParserImpl::ProcessMset(qkpack_request_t *request) 
{
	int												rc;
	std::vector<qkpack_request_t>					&vector_sub = request->vector_sub;	

	//发送EVALSHA mset
	if ( request->phase == QKPACK_SUBREQUEST_MSET ) {
		QKPACK_LOG_DEBUG("create subrequest mset QKPACK_SUBREQUEST_MSET");
		
		request->subrequest.clear();

		if ( !vector_sub.size() ) {
			std::map<int, std::vector<kvpair_t*> > &map_slots = request->map_slots_kv;
			std::map<int, std::vector<kvpair_t*> >::iterator it;
			
			for ( it = map_slots.begin(); it != map_slots.end(); ++it ) {
				
				qkpack_request_t sub;
				SetSubRequest(&sub,QKPACK_URI_MSET,request->ak,request->uri);

				sub.args.append("&slotid=");
				sub.args.append(StringUtil::ToString(it->first));

				vector_sub.push_back(sub);
			}
		}
		
		int sub_size = vector_sub.size();
		if ( sub_size == 0) {
			return QKPACK_ERROR;	
		}
		else if ( sub_size > mset_max_count_ ) {
			QKPACK_LOG_DEBUG("create subrequest mset QKPACK_SUBREQUEST_MSET sub_size=[%d]", mset_max_count_);
			
			request->subrequest.insert(request->subrequest.end(), vector_sub.begin(), vector_sub.begin() + mset_max_count_);	
			vector_sub.erase(vector_sub.begin(), vector_sub.begin() + mset_max_count_);
		}
		else {
			QKPACK_LOG_DEBUG("create subrequest mset QKPACK_SUBREQUEST_MSET sub_size=[%d]",sub_size);
			
			request->subrequest.insert(request->subrequest.end(), vector_sub.begin(), vector_sub.begin() + sub_size);	
			vector_sub.erase(vector_sub.begin(), vector_sub.begin() + sub_size);
		}

		request->phase = QKPACK_SUBREQUEST_DONE;
		return QKPACK_AGAIN;
	}
	
	//子请求发送完毕
	if ( request->phase == QKPACK_SUBREQUEST_DONE ) {
		QKPACK_LOG_DEBUG("create subrequest mset QKPACK_SUBREQUEST_DONE");
		
		//检查子请求状态
		rc = CheckSubRequestStatus(request);
		if ( rc != QKPACK_OK ) {
			QKPACK_LOG_ERROR("CheckSubRequestS is error");
			return QKPACK_ERROR;
		}
		QKPACK_TIME_MS_END(request);	

		if ( vector_sub.size() ) {
			request->phase = QKPACK_SUBREQUEST_MSET;
			return QKPACK_AGAIN;
		}
		
		return QKPACK_OK;
	}

	return QKPACK_ERROR;
}


/**
 * 
 * MSET子请求
 * 
*/
int QkPackParserImpl::CreateSubRequestMset(qkpack_request_t *request)
{
	int												rc;
	
	if ( request->phase == QKPACK_SUBREQUEST_START ) {
		QKPACK_LOG_DEBUG("create subrequest mset QKPACK_SUBREQUEST_START");

		request->phase = QKPACK_SUBREQUEST_MSET;
	}

	rc = ProcessMset(request);
	if ( rc == QKPACK_AGAIN &&  request->phase == QKPACK_SUBREQUEST_MSET ) {
		return ProcessMset(request);
	}
	else if ( rc == QKPACK_AGAIN && request->phase == QKPACK_SUBREQUEST_DONE ) {
		return QKPACK_AGAIN;
	}
	else if ( rc != QKPACK_OK ) {
		return QKPACK_ERROR;
	}
	
	return qkpack_yajl_json_->WriterJsonMset(request);
}


/**
 * 
 * ZFIXEDSETADD子请求
 * 
*/
int QkPackParserImpl::CreateSubRequestZfixedsetAdd(qkpack_request_t *request)
{
	int												rc;
	std::vector<qkpack_request_t>					&vector_sub = request->vector_sub; 
	
	if ( request->phase == QKPACK_SUBREQUEST_START ) {
		QKPACK_LOG_DEBUG("create subrequest zfixedsetadd QKPACK_SUBREQUEST_START");

		request->phase = QKPACK_SUBREQUEST_ZFIXEDSET_ADD;
	}

	//发送EVALSHA ZFIXEDSET_Add
	if ( request->phase == QKPACK_SUBREQUEST_ZFIXEDSET_ADD ) {
		QKPACK_LOG_DEBUG("create subrequest zfixedsetadd QKPACK_SUBREQUEST_ZFIXEDSET_ADD");

		request->subrequest.clear();
	
		if ( !vector_sub.size() ) {

			std::map<std::string, std::vector<kvpair_t> >				&map_kv = request->map_kv;
			std::map<std::string, std::vector<kvpair_t> >::iterator		it;

			for ( it = map_kv.begin(); it != map_kv.end(); ++it ) {
				//发起子请求	
				qkpack_request_t sub;
				SetSubRequest(&sub,QKPACK_URI_ZFIXEDSET_ADD,request->ak,request->uri);
			
				sub.args.append("&key=");
				sub.args.append(it->first);

				sub.args.append("&slotid=");
				
				std::string key = request->kvpair.namespaces + it->first;
				int slot_id = key_hash_slot((char*)key.c_str(), key.length());
				sub.args.append(StringUtil::ToString(slot_id));
		
				vector_sub.push_back(sub);
			}
		}

		int sub_size = vector_sub.size();
		if ( sub_size == 0) {
			return QKPACK_ERROR;	
		}
		else if ( sub_size > batchadd_max_count_ ) {
			QKPACK_LOG_DEBUG("create subrequest  QKPACK_SUBREQUEST_ZFIXEDSET_ADD sub_size=[%d]", batchadd_max_count_);
			
			request->subrequest.insert(request->subrequest.end(), vector_sub.begin(), vector_sub.begin() + batchadd_max_count_);	
			vector_sub.erase(vector_sub.begin(), vector_sub.begin() + batchadd_max_count_);
		}
		else {
			QKPACK_LOG_DEBUG("create subrequest  QKPACK_SUBREQUEST_ZFIXEDSET_ADD sub_size=[%d]",sub_size);
			
			request->subrequest.insert(request->subrequest.end(), vector_sub.begin(), vector_sub.begin() + sub_size);	
			vector_sub.erase(vector_sub.begin(), vector_sub.begin() + sub_size);
		}
		
		request->phase = QKPACK_SUBREQUEST_MSET;
		return QKPACK_AGAIN;
	}
	
	//发送EVALSHA mset
	if ( request->phase == QKPACK_SUBREQUEST_MSET ) {
		QKPACK_LOG_DEBUG("create subrequest zfixedsetadd QKPACK_SUBREQUEST_MSET");
		
		//检查子请求状态
		rc = CheckSubRequestStatus(request);
		if ( rc != QKPACK_OK ) {
			QKPACK_LOG_ERROR("CheckSubRequestStatus is error");
			return QKPACK_ERROR;
		}

		request->subrequest.clear();
		
		if ( request->vector_sub.size() ) {
			request->phase = QKPACK_SUBREQUEST_ZFIXEDSET_BATCHADD;
			return QKPACK_AGAIN;
		}
	}
	
	return CreateSubRequestMset(request);
}


int QkPackParserImpl::ProcessZfixedsetBatchAdd(qkpack_request_t *request)
{
	//发送EVALSHA ZFIXEDSET_add
	if ( request->phase == QKPACK_SUBREQUEST_ZFIXEDSET_BATCHADD ) {
		QKPACK_LOG_DEBUG("create subrequest zfixedsetbatchadd QKPACK_SUBREQUEST_ZFIXEDSET_BATCHADD");

		request->phase = QKPACK_SUBREQUEST_ZFIXEDSET_ADD;
	}
	return CreateSubRequestZfixedsetAdd(request);
}


/**
 * 
 * ZFIXEDSET_BATCHADD子请求
 * 
*/
int QkPackParserImpl::CreateSubRequestZfixedsetBatchAdd(qkpack_request_t *request)
{
	int												rc;
	
	if ( request->phase == QKPACK_SUBREQUEST_START ) {
		QKPACK_LOG_DEBUG("create subrequest zfixedsetbatchadd QKPACK_SUBREQUEST_START");

		request->phase = QKPACK_SUBREQUEST_ZFIXEDSET_BATCHADD;
	}

	rc = ProcessZfixedsetBatchAdd(request);
	if ( rc == QKPACK_AGAIN && request->phase == QKPACK_SUBREQUEST_ZFIXEDSET_BATCHADD ) {
		return ProcessZfixedsetBatchAdd(request);
	}
	else if ( rc == QKPACK_AGAIN && request->phase == QKPACK_SUBREQUEST_MSET ) {
		return QKPACK_AGAIN;
	}

	return rc;	
}


/**
 * 
 * ZFIXEDSET_GETBYSCORE子请求
 * 
*/
int QkPackParserImpl::CreateSubRequestZfixedsetGetByScore(qkpack_request_t *request)
{
	int												rc;
	
	if ( request->phase == QKPACK_SUBREQUEST_START ) {
		QKPACK_LOG_DEBUG("create subrequest zfixedsetgetbyscore QKPACK_SUBREQUEST_START");

		request->phase = QKPACK_SUBREQUEST_ZFIXEDSET_GETBYSCORE;
	}
	//发送zrangebyscore,ZREVRANGEBYSCORE 命令
	if ( request->phase == QKPACK_SUBREQUEST_ZFIXEDSET_GETBYSCORE ) {
		QKPACK_LOG_DEBUG("create subrequest zfixedsetgetbyscore QKPACK_SUBREQUEST_ZFIXEDSET_GETBYSCORE");
		
		request->subrequest.clear();

		std::map<std::string, zset_query_t>				&map_query = request->map_query;
		std::map<std::string, zset_query_t>::iterator	it;

		for( it = map_query.begin(); it != map_query.end(); ++it ) {

			//发起子请求	
			qkpack_request_t sub;
			if ( request->kvpair.command_type == REDIS_ZRANGEBYSCORE || 
					request->kvpair.command_type == REDIS_BATCHZRANGEBYSCORE ){
				SetSubRequest(&sub,QKPACK_URI_ZFIXEDSET_GETBYSCORE,request->ak,request->uri);
			}
			else if (  request->kvpair.command_type == REDIS_ZRANGE ) {
				SetSubRequest(&sub, QKPACK_URI_ZFIXEDSET_GETBYRANK,request->ak,request->uri);
			}


			sub.args.append("&key=");
			sub.args.append(it->first);

			sub.args.append("&slotid=");
			
			std::string key = request->kvpair.namespaces + it->first;
			int slot_id = key_hash_slot((char*)key.c_str(), key.length());
			sub.args.append(StringUtil::ToString(slot_id));
			
			request->subrequest.push_back(sub);
		}

		request->phase = QKPACK_SUBREQUEST_MGET;
		return QKPACK_AGAIN;
	}
	
	//发送mget命令
	if ( request->phase == QKPACK_SUBREQUEST_MGET ) {
		QKPACK_LOG_DEBUG("create subrequest zfixedsetgetbyscore	QKPACK_SUBREQUEST_MGET");

		//检查子请求状态
		rc = CheckSubRequestStatus(request);
		if ( rc != QKPACK_OK ) {
			QKPACK_LOG_ERROR("CheckSubRequestS is error");
			return QKPACK_ERROR;
		}

		request->subrequest.clear();
		
		std::map<std::string, std::vector<kvpair_t> >				&map_kv = request->map_kv;
		std::map<std::string, std::vector<kvpair_t> >::iterator		it;
		std::string													kv_key;
		std::string													&namespaces = request->kvpair.namespaces;
		int															slot_id;

		for( it = map_kv.begin(); it != map_kv.end(); ++it ) {
			
			std::vector<kvpair_t>	&vector_kv = it->second;
			
			for( size_t i = 0; i< vector_kv.size(); ++i ) {
				kv_key = namespaces + vector_kv[i].key;
				slot_id = key_hash_slot((char*)kv_key.c_str(), kv_key.length());
				
				request->map_slots_kv[slot_id].push_back(&vector_kv[i]);
			}
		}
		
		if ( request->map_slots_kv.size() ) {
			return CreateSubRequestMget(request);
		}

		request->phase = QKPACK_SUBREQUEST_DONE;
	}

	//结束子请求
	if ( request->phase == QKPACK_SUBREQUEST_DONE ) {
		
		//检查子请求状态
		rc = CheckSubRequestStatus(request);
		if ( rc != QKPACK_OK ) {
			QKPACK_LOG_ERROR("CheckSubRequestS is error");
			return QKPACK_ERROR;
		}
		
		FilterGetByScoreMemberNullValue(request);
		
		QKPACK_TIME_MS_END(request);

		if ( request->kvpair.command_type == REDIS_BATCHZRANGEBYSCORE ) {
			return qkpack_yajl_json_->WriterJsonZfixedsetBatchGetByScore(request);
		}
		else {
			return qkpack_yajl_json_->WriterJsonZfixedsetGetByScore(request);
		}
	}

	return QKPACK_ERROR;
}


/**
 * 
 * ZFIXEDSET_BATCHGETSCORE
 * 
*/
int QkPackParserImpl::CreateSubRequestZfixedsetBatchGetByScore(qkpack_request_t *request)
{
	if ( request->phase == QKPACK_SUBREQUEST_START ) {
		QKPACK_LOG_DEBUG("create subrequest zfixedsetbatchgetbyscore	QKPACK_SUBREQUEST_START");

		request->phase = QKPACK_SUBREQUEST_ZFIXEDSET_BATCHGETBYSCORE;
	}
	
	//发送EVALSHA ZFIXEDSET_GETBYSCORE
	if ( request->phase == QKPACK_SUBREQUEST_ZFIXEDSET_BATCHGETBYSCORE ) {
		QKPACK_LOG_DEBUG("create subrequest zfixedsetbatchgetbyscore QKPACK_SUBREQUEST_ZFIXEDSET_BATCHGETBYSCORE");
		request->phase = QKPACK_SUBREQUEST_ZFIXEDSET_GETBYSCORE;
	}	

	return CreateSubRequestZfixedsetGetByScore(request);
}


/**
 * 
 * 创建子请求
 * 
*/
int QkPackParserImpl::CreateSubRequestBody(qkpack_request_t *request)
{
	// request解析错误码，直接构建json返回错误信息
	if ( request->code != QKPACK_RESPONSE_CODE_OK ) {
		return QKPACK_ERROR;
	}

	switch(request->kvpair.command_type)
	{
		case REDIS_MGET:	
			return CreateSubRequestMget(request);
		case REDIS_MSET:
			return CreateSubRequestMset(request);
		case REDIS_ZADD:
			return CreateSubRequestZfixedsetAdd(request);
		case REDIS_ZBATCHADD:
			return CreateSubRequestZfixedsetBatchAdd(request);
		case REDIS_ZRANGEBYSCORE:
		case REDIS_ZRANGE:
			return CreateSubRequestZfixedsetGetByScore(request);
		case REDIS_BATCHZRANGEBYSCORE:
			return CreateSubRequestZfixedsetBatchGetByScore(request);
		default:
			return QKPACK_ERROR;
	}
	
	return QKPACK_ERROR;
}



/**
 * 
 * 检查子请求状态
 * 
*/
int QkPackParserImpl::CheckSubRequestStatus(qkpack_request_t *request)
{
/*
	for ( size_t i = 0; i < request->subrequest.size(); ++i ) {
		qkpack_request_t &sub = request->subrequest[i];

		if ( sub.status != QKPACK_HTTP_OK ) {
			return QKPACK_ERROR;
		}
	}
*/
	if ( request->code != QKPACK_RESPONSE_CODE_OK ) {
		return QKPACK_ERROR;
	}
	
	return QKPACK_OK;
}



/**
 * 
 * 检查是否是子请求
 * 
*/
bool QkPackParserImpl::CheckSubRequest(qkpack_request_t *request) 
{
	QKPACK_LOG_DEBUG("[qkpack_so] QkPackParserImpl::CheckSubRequest");
	
	//如果有subrequest参数，证明是nginx自己发起的子请求，不需要创建子请求。否则，就创建需要创建子请求
	if ( request->is_subrequest ) {
		return false;
	}

	switch(request->kvpair.command_type)
	{
		case REDIS_MGET:
		case REDIS_MSET:
		case REDIS_ZADD:
		case REDIS_ZBATCHADD:
		case REDIS_ZRANGEBYSCORE:
		case REDIS_ZRANGE:
		case REDIS_BATCHZRANGEBYSCORE:
			return true;
	}

	return false;
}

