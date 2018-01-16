#ifndef __REDIS_CLUSTER_H__
#define __REDIS_CLUSTER_H__

#include "common.h"
#include "qkpack_common.h"
#include "core/redis_node.h"
#include "core/redis_slot.h"

namespace com { 
namespace youku { 
namespace data {
namespace qkpack {
namespace core {


class  RedisCluster 
{
public:
	RedisCluster(std::string backup_server);
	virtual ~RedisCluster();

public:
	/**
     * 
     * 获得节点(key->slotid->node_ip)
    */
	int GetNode(qkpack_request_t *request, const route_t &route);
	
	int GetNode(qkpack_request_t	*request);
	/**
     * 
     * 随机节点(rand()%16383)
    */
	int RandNode(qkpack_request_t *request);
	
	/**
     * 
     * 处理moved,ask
    */
	int ProcessMoved(const std::string &in,const std::string &cluster_name, std::string &node_ip, int status);
	
	/**
     * 
     * 异步初始化cluster nodes
    */
	int InitAsyncClusterNode(const std::string &node_ip, const std::string &cluster_name);
	
	/**
     * 
     * 初始化cluster nodes
    */
	int InitClusterNode(const std::string &node_ip, const std::string &cluster_name);

	
	/**
     * 
     * 获取cluster name
    */
	std::string GetClusterName();

	
	/**
     * 
     * 更新cluster nodes
    */
	int UpdateClusterNodes(const std::string& in, std::string cluster_name);

	
	/**
     * 
     * 获取备用节点
    */
	std::string GetBackUpNode()
	{
		return backup_server_;
	}

protected:

	/**
     * 
     * 设置cluster name
    */
	void SetClusterName(std::string cluster_name);
	

	int UpdateRouteNodeList(const std::string &iplist,const std::string &cluster_name);

	/**
     * 
     * 设置map slot
    */
	int SetMemNode(RedisNode *node,std::map<int,std::string> &slot_map);
	
	RedisNode* GetNode(const std::string& line);
	
	void AddSlotRange(RedisNode* node, char* slots);
	

private:
	std::map<std::string, std::map<int,std::string> > map_cluster_;
	std::string cluster_name_;

	std::string backup_server_;
};

} /* namespace core */
} /* namespace qkpack */
} /* namespace data */
} /* namespace youku */
} /* namespace com */


#endif //__REDIS_CLUSTER_H__
