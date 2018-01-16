#ifndef __QKPACK_METRICS_H__
#define __QKPACK_METRICS_H__

#include "qkpack_common.h"
#include "metrics/reporter.h"

extern "C" {
#include <yajl/yajl_parse.h>
#include <yajl/yajl_gen.h>
#include <yajl/yajl_tree.h>
}

using namespace com::youku::data::qkpack::metrics;


namespace com { 
namespace youku { 
namespace data {
namespace qkpack {
namespace core {
	

class QkPackMetrics : public Reporter 
{
public:
    QkPackMetrics(bool metrics_status,
			boost::chrono::milliseconds rate_unit = boost::chrono::seconds(1),
			boost::chrono::milliseconds duration_unit = boost::chrono::seconds(1));
	virtual ~QkPackMetrics();
	
public:
	/**
     * 
     * 构建metrics 信息
    */
	virtual void Report();
	
	/**
     * 
     * 用户场景timer
    */
	void UserSceneTimer(qkpack_request_t *request);
	
	/**
     * 
     * 全局场景timer
    */
	void GlobalSceneTimer(qkpack_request_t *request);

	/**
     * 
     * 用户场景meter
    */
	void UserSceneMeter(qkpack_request_t *request, std::string metrics_command);

	/**
     * 
     * 全局场景meter
    */
	void GlobalSceneMeter(qkpack_request_t *request, std::string metrics_command);

	/**
     * 
     * timer stop
    */
	void TimerStop(qkpack_request_t *request);
	
	/**
	 * 
	 * Metrics埋点(GET,SMEMBERS)Miss
	 * 
	*/
	void CheckUserSceneMetricsMissGetAndSmembers(qkpack_request_t *request);


	/**
	 * 
	 * Metrics埋点(MGET,ZRANGE)Miss
	 * 
	*/
	void CheckUserSceneMetricsMissMgeAndZRange(qkpack_request_t *request);

protected:
	/**
     * 
     * 用户场景metric registry
    */
	MetricRegistryPtr UserSceneMetricRegistry(const std::string &host,
			int	pid);

	/**
     * 
     * 全局场景metric registry
    */
	MetricRegistryPtr GlobalSceneMetricRegistry(const std::string &host,
			int pid);


	double ConvertDurationUnit(double duration_value) const;
	
    double ConvertRateUnit(double rate_value) const;
	
	const char* RateUnitInSecMeter();
	
	const char* RateUnitInSec();
	
	const char* DurationUnitInSec(); 

private:
	void PrintMeter(const MeteredMap::mapped_type& meter, yajl_gen &g); 
	void PrintTimer(const TimerMap::mapped_type& timer, yajl_gen &g); 
	
	bool metrics_status_;
	boost::chrono::milliseconds rate_unit_;
	boost::chrono::milliseconds duration_unit_;
	double rate_factor_;
    double duration_factor_;

	std::map<std::string,MetricRegistryPtr> map_registry_;
};

} /* namespace core */
} /* namespace qkpack */
} /* namespace data */
} /* namespace youku */
} /* namespace com */


#endif /* __QKPACK_METRICS_H__ */
