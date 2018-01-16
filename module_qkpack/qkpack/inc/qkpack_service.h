#ifndef __QKPACK_SERVICE_H__
#define __QKPACK_SERVICE_H__

#include "common.h"
#include "core/qkpack_handler.h"

using namespace com::youku::data::qkpack::core;

namespace com { 
namespace youku { 
namespace data {
namespace qkpack {


class QkPackService : public CPlugin
{
public:
	QkPackService(){}
	virtual ~QkPackService(){}
	
public:	
	/**
     * 
     * Initialize Work Process
    */
	int InitProcess(const std::string &so_conf);
	
	/**
     * 
     * Exit Work Process
    */
	void ExitProcess();
	
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

private:
	QkPackHandler *qkpack_handler_;
	
};
	

} /* namespace qkpack */
} /* namespace data */
} /* namespace youku */
} /* namespace com */

#endif /* __QKPACK_SERVICE_H__ */
