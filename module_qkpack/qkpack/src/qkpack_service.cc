#include "qkpack_service.h"
#include "qkpack_log.h"
#include "util/filekv_parser.h"
#include "util/file_util.h"
#include "util/string_util.h"

using namespace com::youku::data::qkpack;
using namespace com::youku::data::qkpack::util;

int QkPackService::InitProcess(const std::string &so_conf) 
{
	std::string											backup_server;
	std::string 										conf_dir;
	std::string 										log4cpp_path;
	std::string 										acl_conf_path;
	int													mset_max_count;
	int													batchadd_max_count;
	bool												metrics_status = true;

	std::map<std::string, std::string>					conf_kv;
	std::map<std::string, std::string>::const_iterator	it;
	
	std::cout  << so_conf << std::endl;

	if( !FilekvParser::Parse(const_cast<std::string&>(so_conf), conf_kv) ) {
		std::cout << "Parse cofig file error:" << so_conf << std::endl;
		return QKPACK_ERROR;
	}

	conf_dir =  FileUtil::GetParentDir(so_conf) + "/";
	it = conf_kv.find(LOG4CPP_CONF);
	if ( it->second.empty() ) {
		std::cout << "Can't find LOG4CPP_CONF in config" << std::endl;
		return QKPACK_ERROR;
	}
	
	//log4cpp_path = FileUtil::GenerateAbsolutePath(conf_dir, it->second);
	//LOG_CONFIG(log4cpp_path);
	
	it = conf_kv.find("qkpack_acl_conf");
	if ( it->second.empty() ) {
		std::cout << "Can't find qkpack_acl.conf in config" << std::endl;
		return QKPACK_ERROR;
	}
	acl_conf_path = FileUtil::GenerateAbsolutePath(conf_dir,it->second);

	
	it = conf_kv.find("backup_server");
	if ( it->second.empty() ) {
		std::cout << "Can't find backup_server in config" << std::endl;
		return QKPACK_ERROR;
	}
	backup_server = it->second;

	it = conf_kv.find("metrics_status");
	if ( it->second.compare("off") == 0 ) {
		metrics_status =  false;
	}


	it = conf_kv.find("mset_max_count");
	if ( it->second.empty() ) {
		std::cout << "Can't find mset_max_count in config" << std::endl;
		return QKPACK_ERROR;
	}
	StringUtil::StrToInt32(it->second.c_str(),mset_max_count);

	it = conf_kv.find("batchadd_max_count");
	if ( it->second.empty() ) {
		std::cout << "Can't find batchadd_max_count in config" << std::endl;
		return QKPACK_ERROR;
	}
	StringUtil::StrToInt32(it->second.c_str(),batchadd_max_count);

	qkpack_handler_ = new QkPackHandler();
	if( qkpack_handler_ == NULL ){
		return QKPACK_ERROR;
	}

	if( qkpack_handler_->Init(acl_conf_path,backup_server,mset_max_count,batchadd_max_count, metrics_status) != QKPACK_OK ) {
		return QKPACK_ERROR;
	}

	return QKPACK_OK;
}


void QkPackService::ExitProcess() 
{
	if ( qkpack_handler_ ) {
		
		qkpack_handler_->Exit();

		delete qkpack_handler_;
		qkpack_handler_ = NULL;
	}
}


int QkPackService::CreateRequest(void *data)
{
	return qkpack_handler_->CreateRequest(data);
}


int QkPackService::ProcessBody(void *data)
{
	return qkpack_handler_->ProcessBody(data);
}

int QkPackService::ReinitRequest(void *data)
{
	return qkpack_handler_->ReinitRequest(data);
}


int QkPackService::Destroy(void *data) 
{
	return qkpack_handler_->Destroy(data);
}


int QkPackService::Publish()
{
	return qkpack_handler_->Publish();
}


extern "C" {
	
	void* create_instance() {
		return new QkPackService(); 
	}
}

