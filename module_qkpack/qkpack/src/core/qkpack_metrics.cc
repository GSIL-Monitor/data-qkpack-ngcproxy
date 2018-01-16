#include "core/qkpack_metrics.h"
#include "metrics/utils.h"
#include <boost/foreach.hpp>
#include "qkpack_log.h"
#include "util/string_util.h"
#include "config/easycurl.h"

using namespace com::youku::data::qkpack::util;
using namespace com::youku::data::qkpack::core;
using namespace com::youku::data::qkpack::config;

static const int64_t
	one_day			=	boost::chrono::milliseconds(boost::chrono::hours(24)).count(),
	one_hour		=	boost::chrono::milliseconds(boost::chrono::hours(1)).count(),
	one_minute		=	boost::chrono::milliseconds(boost::chrono::minutes(1)).count(),
	one_seconds		=	boost::chrono::milliseconds(boost::chrono::seconds(1)).count(),
	one_millisecond	=	boost::chrono::milliseconds(boost::chrono::milliseconds(1)).count();


QkPackMetrics::QkPackMetrics(bool metrics_status,
		boost::chrono::milliseconds rate_unit,
		boost::chrono::milliseconds duration_unit):
	metrics_status_(metrics_status),
	rate_unit_(rate_unit),
	duration_unit_(duration_unit) 
{		
	rate_factor_ = boost::chrono::milliseconds(1000).count() / rate_unit.count();
	duration_factor_ = static_cast<double>(1.0) / 
		boost::chrono::duration_cast<boost::chrono::nanoseconds>(duration_unit).count();
}


QkPackMetrics::~QkPackMetrics() 
{
}


MetricRegistryPtr QkPackMetrics::UserSceneMetricRegistry(const std::string &host,int pid)
{
	std::string												registry_name;
	std::map<std::string,MetricRegistryPtr>::iterator		it;

	registry_name.append(QKPACK_USERSCENE_SYS_ID);
	registry_name.append("_");

	registry_name.append(QKPACK_USERSCENE_SCE_ID);
	registry_name.append("_min_");
	
	registry_name.append(host);
	registry_name.append("::");
	
	registry_name.append(StringUtil::ToString(pid));
	
	it = map_registry_.find(registry_name);
	if ( it != map_registry_.end() ) {
		return it->second;
	}

	map_registry_[registry_name] =  MetricRegistryPtr(new MetricRegistry());
	return map_registry_[registry_name];
}


MetricRegistryPtr QkPackMetrics::GlobalSceneMetricRegistry(const std::string &host, int pid)
{
	std::string												registry_name;
	std::map<std::string,MetricRegistryPtr>::iterator		it;
	
	registry_name.append(QKPACK_GLOBALSCENE_G_SYS_ID);
	registry_name.append("_");

	registry_name.append(QKPACK_GLOBALSCENE_G_SCE_ID);
	registry_name.append("_min_");

	registry_name.append(host);
	registry_name.append("::");
	
	registry_name.append(StringUtil::ToString(pid));
	
	it = map_registry_.find(registry_name);
	if ( it != map_registry_.end() ) {
		return it->second;
	}

	map_registry_[registry_name] = MetricRegistryPtr(new MetricRegistry());
	return map_registry_[registry_name];
}

typedef struct {
	TimerContextPtr  user_timer;
	TimerContextPtr  global_timer;
}qkpack_timer;


void QkPackMetrics::UserSceneTimer(qkpack_request_t *request)
{
	if ( !metrics_status_ ) {
		return;
	}

	std::string 											timer_name;
	std::map<std::string,TimerContextPtr>::iterator			it;
	MetricRegistryPtr										user_scene_registry;	
	
	user_scene_registry = UserSceneMetricRegistry(request->host,request->pid);
	if ( user_scene_registry == NULL ) {
		return;
	}

	timer_name.append(request->kvpair.uri_id);
	timer_name.append(".");
	
	timer_name.append(request->ak);
	timer_name.append(".");

	timer_name.append(request->kvpair.namespaces);
	timer_name.append(".");

	timer_name.append(request->kvpair.metrics_command);
	
	timer_name.append(".timer");

	qkpack_timer	*timer;
	if ( request->handler == NULL ) {
		timer = new qkpack_timer;
		request->handler = timer;
	}
	else {
		timer = (qkpack_timer*) request->handler;
	}
	
	timer->user_timer = user_scene_registry->timer(timer_name)->timerContextPtr();
}


void QkPackMetrics::GlobalSceneTimer(qkpack_request_t *request)
{
	if ( !metrics_status_ ) {
		return;
	}

	std::string 											timer_name;
	std::string												metrics_command;
	std::map<std::string,TimerContextPtr>::iterator			it;
	MetricRegistryPtr										global_scene_registry;	
	
	if ( request->kvpair.script_name.empty() ) {
		return;	
	}
	
	//metrics 批量接口
	metrics_command = request->kvpair.script_name;

	if ( request->parent ) {
		global_scene_registry = GlobalSceneMetricRegistry(request->parent->host,request->parent->pid);
	}
	else {
		global_scene_registry = GlobalSceneMetricRegistry(request->host,request->pid);
	}
	
	if ( global_scene_registry == NULL ) {
		return;
	}

	timer_name.append("G_");

	timer_name.append(metrics_command);

	timer_name.append(".timer");

	qkpack_timer	*timer;
	if ( request->handler == NULL ) {
		timer = new qkpack_timer;
		request->handler = timer;
	}
	else {
		timer = (qkpack_timer*) request->handler;
	}
	
	timer->global_timer = global_scene_registry->timer(timer_name)->timerContextPtr();
}



void QkPackMetrics::UserSceneMeter(qkpack_request_t *request, std::string metrics_command)
{
	if ( !metrics_status_ ) {
		return;
	}

	std::string 											meter_name;
	std::map<std::string,MeterPtr>::iterator				it;
	MetricRegistryPtr										user_scene_registry;	
	
	user_scene_registry = UserSceneMetricRegistry(request->host,request->pid);
	if ( user_scene_registry == NULL ) {
		return;
	}

	meter_name.append(request->kvpair.uri_id);
	meter_name.append(".");

	meter_name.append(request->ak);
	meter_name.append(".");

	meter_name.append(request->kvpair.namespaces);
	meter_name.append(".");

	meter_name.append(metrics_command);

	meter_name.append(".meter");

	user_scene_registry->meter(meter_name)->Mark();
}



void QkPackMetrics::GlobalSceneMeter(qkpack_request_t *request, std::string metrics_command)
{
	if ( !metrics_status_ ) {
		return;
	}

	std::string 											meter_name;
	std::map<std::string,MeterPtr>::iterator				it;
	MetricRegistryPtr										global_scene_registry;	

	if ( request->parent ) {
		global_scene_registry = GlobalSceneMetricRegistry(request->parent->host,request->parent->pid);
	}
	else {
		global_scene_registry = GlobalSceneMetricRegistry(request->host,request->pid);
	}

	if ( global_scene_registry == NULL ) {
		return;
	}

	meter_name.append(request->kvpair.uri_id);

	meter_name.append(metrics_command);

	meter_name.append(".meter");

	global_scene_registry->meter(meter_name)->Mark();
}


void QkPackMetrics::TimerStop(qkpack_request_t *request) 
{
	if ( !metrics_status_ ) {
		return;
	}

	 if ( request->handler ) {
		qkpack_timer *timer = (qkpack_timer*)request->handler;
		TimerContextPtr user_timer = timer->user_timer;
		TimerContextPtr global_timer = timer->global_timer;
		
		if ( user_timer ) {
			user_timer->Stop();
		}

		if ( global_timer ) {
			global_timer->Stop();
		}

		delete timer;
		timer = NULL;
	 }
}


/**
 * 
 * Metrics埋点(MGET,ZRANGE)Miss
 * 
*/
void QkPackMetrics::CheckUserSceneMetricsMissMgeAndZRange(qkpack_request_t *request)
{
	if ( !metrics_status_ ) {
		return;
	}

	bool									miss = true;
	std::string								dim_name;
	int										command_type = request->kvpair.command_type;

	if ( command_type == REDIS_ZRANGEBYSCORE || command_type == REDIS_ZRANGE )  {
	
		std::map<std::string, std::vector<kvpair_t> >			&map_kv = request->map_kv;
		std::map<std::string, std::vector<kvpair_t> >::iterator	it;

		for( it = map_kv.begin(); it != map_kv.end(); ++it ) {
			std::vector<kvpair_t>	&kvpair = it->second;

			for( size_t i = 0; i < kvpair.size(); ++i ) {
				
				if ( kvpair[i].value.compare("null") != 0 ) {
						miss = false;
						break;
				}
			}
			
			if ( !miss ) {
				return;
			}
		}
		
		dim_name = command_type == REDIS_ZRANGEBYSCORE ? 
			QKPACK_METRICS_ZFIXEDSETGETBYSCOREMISS : QKPACK_METRICS_ZFIXEDSETGETBYRANKMISS;
	}
	else if ( request->kvpair.command_type == REDIS_MGET ) {

		std::vector<kvpair_t>::iterator	 it = request->vector_kv.begin();
		for ( ; it != request->vector_kv.end(); ++it ) {
			
			if ( (*it).value.compare("null") != 0 ) {
					miss = false;
					break;
			}
		}

		if ( !miss ) {
			return;
		}

		dim_name = QKPACK_METRICS_MGETMISS;	
	}

	if ( dim_name.empty() ) {
		return;
	}

	UserSceneMeter(request, dim_name);
}


/**
 * 
 * Metrics埋点(GET,SMEMBERS)Miss
 * 
*/
void QkPackMetrics::CheckUserSceneMetricsMissGetAndSmembers(qkpack_request_t *request) 
{
	if ( !metrics_status_ ) {
		return;
	}

	std::string								dim_name;
	
	if  ( request->kvpair.command_type == REDIS_GET ) {

		if ( request->kvpair.value.compare("null") ) {
			return;
		}
		dim_name = QKPACK_METRICS_GETMISS;
	}
	else if ( request->kvpair.command_type == REDIS_SMEMBERS ) {
		
		if ( request->vector_mbs.size() ) {
			return;
		}
		dim_name = QKPACK_METRICS_SMEMBERSMISS;
		
	}

	if ( dim_name.empty() ) {
		return;
	}

	UserSceneMeter(request, dim_name);
}



void QkPackMetrics::Report()
{
	if ( !metrics_status_ ) {
		return;
	}

	/*
	[{
        "sysName":"23_1533_min_10.100.23.195",
        "meters":[
            {
                "name":"1502.PBntwBtBl8.A1.zfixedsetGetByScoreMiss.meter",
                "count":34,
                "m15_rate":0.000704600257957628,
                "m1_rate":2.3078927890808079e-23,
                "m5_rate":0.0000012850722312153454,
                "mean_rate":0.0016760488045125888,
                "units":"events/second"
            }
        ],
        "timers":[
            {
                "name":"1502.PBntwBtBl8.A1.incrBy.timer",
                "count":488,
                "max":0,
                "mean":0.0005691203463114754,
                "min":0,
                "p50":0.0002616745,
                "p75":0.00029114925,
                "p95":0.00034554845000000004,
                "p98":0.0004745816799999999,
                "p99":0.0005644768200000001,
                "p999":0.14631544700000002,
                "stddev":0.006611449136952798,
                "m15_rate":0.03668855431573927,
                "m1_rate":0.009391558374931084,
                "m5_rate":0.03702271661734353,
                "mean_rate":0.02271881570776875,
                "duration_units":"seconds",
                "rate_units":"calls/second"
            }
        ]
    }]
	*/
	std::map<std::string,MetricRegistryPtr>::iterator it = map_registry_.begin();
	
	yajl_gen                  g;
	size_t					  len;
	const unsigned char		  *buf;
			    
	g = yajl_gen_alloc(NULL);
	if (g == NULL) {
		return;
	}
	
	yajl_gen_config(g, yajl_gen_beautify, 1);

	yajl_gen_array_open(g);

	for( ; it != map_registry_.end(); ++it ) {
		
		yajl_gen_map_open(g);

		yajl_gen_string(g, (const unsigned char *) QKPACK_METRICS_SYSNAME, strlen(QKPACK_METRICS_SYSNAME));
		yajl_gen_string(g, (const unsigned char *) it->first.c_str(), it->first.length());

		MetricRegistryPtr &registry = it->second;
		MeteredMap meter_map(registry->GetMeters());
		TimerMap timer_map(registry->GetTimers());

		if ( meter_map.size() ) {
			//meters array

			yajl_gen_string(g, (const unsigned char *) QKPACK_METRICS_METERS, strlen(QKPACK_METRICS_METERS));
			yajl_gen_array_open(g);

    	    BOOST_FOREACH(const MeteredMap::value_type& entry, meter_map) {

				yajl_gen_map_open(g);
			
				yajl_gen_string(g, (const unsigned char *) QKPACK_METRICS_METERS_NAME, strlen(QKPACK_METRICS_METERS_NAME));
				yajl_gen_string(g, (const unsigned char *) entry.first.c_str(), entry.first.length());
		
    	        PrintMeter(entry.second,g);
				
				yajl_gen_map_close(g);
    	    }

			yajl_gen_array_close(g);
		}

		if ( timer_map.size() ) {
			//meters array
			
			yajl_gen_string(g, (const unsigned char *) QKPACK_METRICS_TIMERS, strlen(QKPACK_METRICS_TIMERS));
			yajl_gen_array_open(g);

    	    BOOST_FOREACH(const TimerMap::value_type& entry, timer_map) {
				
				yajl_gen_map_open(g);
				
				yajl_gen_string(g, (const unsigned char *) QKPACK_METRICS_TIMERS_NAME, strlen(QKPACK_METRICS_TIMERS_NAME));
				yajl_gen_string(g, (const unsigned char *) entry.first.c_str(), entry.first.length());
				
				PrintTimer(entry.second,g);
				
				yajl_gen_map_close(g);
    	    }
			yajl_gen_array_close(g);
		}

		yajl_gen_map_close(g);
	}
	
	yajl_gen_array_close(g);

	yajl_gen_status status = yajl_gen_get_buf(g, &buf, &len);
	if(status != yajl_gen_status_ok) {
		yajl_gen_free(g);
		return;
	}
	 
	std::string data =  std::string((const char*)buf,len);
	yajl_gen_free(g);

	if ( !len ) {
		return;
	}

	QKPACK_LOG_DEBUG("%s",data.c_str());

	EasyCurl	m_curl;

	std::vector<std::string>			headers;
	headers.push_back("Content-Type: application/json");
	
	int code = m_curl.Post("http://dpm.1verge.net/httpServer/reception/json", headers, data);

	if (code != CURLE_OK) {
		QKPACK_LOG_ERROR("qkpack metrics post is error code=[%d]",code);
	}

}


void QkPackMetrics::PrintMeter(const MeteredMap::mapped_type& meter, yajl_gen  &g) 
{

	yajl_gen_string(g, (const unsigned char *) QKPACK_METRICS_METERS_COUNT, strlen(QKPACK_METRICS_METERS_COUNT));
	yajl_gen_integer(g, meter->GetCount());

	yajl_gen_string(g, (const unsigned char *) QKPACK_METRICS_METERS_M1_RATE, strlen(QKPACK_METRICS_METERS_M1_RATE));
	yajl_gen_double(g, ConvertRateUnit(meter->GetOneMinuteRate()));

	yajl_gen_string(g, (const unsigned char *) QKPACK_METRICS_METERS_M15_RATE, strlen(QKPACK_METRICS_METERS_M15_RATE));
	yajl_gen_double(g, ConvertRateUnit(meter->GetFifteenMinuteRate()));

	yajl_gen_string(g, (const unsigned char *) QKPACK_METRICS_METERS_M5_RATE, strlen(QKPACK_METRICS_METERS_M5_RATE));
	yajl_gen_double(g, ConvertRateUnit(meter->GetFiveMinuteRate()));

	yajl_gen_string(g, (const unsigned char *) QKPACK_METRICS_METERS_MEAN_RATE, strlen(QKPACK_METRICS_METERS_MEAN_RATE));
	yajl_gen_double(g, ConvertRateUnit(meter->GetMeanRate()));
	
	const char* units = RateUnitInSecMeter();
	yajl_gen_string(g, (const unsigned char *) QKPACK_METRICS_METERS_UNITS, strlen(QKPACK_METRICS_METERS_UNITS));
	yajl_gen_string(g, (const unsigned char *) units, strlen(units));

}

void QkPackMetrics::PrintTimer(const TimerMap::mapped_type& timer, yajl_gen &g) 
{
	SnapshotPtr snapshot = timer->GetSnapshot();

	yajl_gen_string(g, (const unsigned char *) QKPACK_METRICS_TIMERS_COUNT, strlen(QKPACK_METRICS_TIMERS_COUNT));
	yajl_gen_integer(g, timer->GetCount());

	yajl_gen_string(g, (const unsigned char *) QKPACK_METRICS_TIMERS_MAX, strlen(QKPACK_METRICS_TIMERS_MAX));
	yajl_gen_double(g, ConvertDurationUnit(snapshot->GetMax()));

	yajl_gen_string(g, (const unsigned char *) QKPACK_METRICS_TIMERS_MEAN, strlen(QKPACK_METRICS_TIMERS_MEAN));
	yajl_gen_double(g, ConvertDurationUnit(snapshot->GetMean()));

	yajl_gen_string(g, (const unsigned char *) QKPACK_METRICS_TIMERS_MIN, strlen(QKPACK_METRICS_TIMERS_MIN));
	yajl_gen_double(g, ConvertDurationUnit(snapshot->GetMin()));
	
	yajl_gen_string(g, (const unsigned char *) QKPACK_METRICS_TIMERS_P50, strlen(QKPACK_METRICS_TIMERS_P50));
	yajl_gen_double(g, ConvertDurationUnit(snapshot->GetMedian()));
	
	yajl_gen_string(g, (const unsigned char *) QKPACK_METRICS_TIMERS_P75, strlen(QKPACK_METRICS_TIMERS_P75));
	yajl_gen_double(g, ConvertDurationUnit(snapshot->Get75thPercentile()));

	yajl_gen_string(g, (const unsigned char *) QKPACK_METRICS_TIMERS_P95, strlen(QKPACK_METRICS_TIMERS_P95));
	yajl_gen_double(g, ConvertDurationUnit(snapshot->Get95thPercentile()));
	
	yajl_gen_string(g, (const unsigned char *) QKPACK_METRICS_TIMERS_P98, strlen(QKPACK_METRICS_TIMERS_P98));
	yajl_gen_double(g, ConvertDurationUnit(snapshot->Get98thPercentile()));
	
	yajl_gen_string(g, (const unsigned char *) QKPACK_METRICS_TIMERS_P99, strlen(QKPACK_METRICS_TIMERS_P99));
	yajl_gen_double(g, ConvertDurationUnit(snapshot->Get99thPercentile()));
	
	yajl_gen_string(g, (const unsigned char *) QKPACK_METRICS_TIMERS_P999, strlen(QKPACK_METRICS_TIMERS_P999));
	yajl_gen_double(g, ConvertDurationUnit(snapshot->Get999thPercentile()));
	
	yajl_gen_string(g, (const unsigned char *) QKPACK_METRICS_TIMERS_STDDEV, strlen(QKPACK_METRICS_TIMERS_STDDEV));
	yajl_gen_double(g, ConvertDurationUnit(snapshot->GetStdDev()));
	
	yajl_gen_string(g, (const unsigned char *) QKPACK_METRICS_TIMERS_MEAN_RATE, strlen(QKPACK_METRICS_TIMERS_MEAN_RATE));
	yajl_gen_double(g, ConvertRateUnit(timer->GetMeanRate()));
	
	yajl_gen_string(g, (const unsigned char *) QKPACK_METRICS_TIMERS_M1_RATE, strlen(QKPACK_METRICS_TIMERS_M1_RATE));
	yajl_gen_double(g,  ConvertRateUnit(timer->GetOneMinuteRate()));

	yajl_gen_string(g, (const unsigned char *) QKPACK_METRICS_TIMERS_M5_RATE, strlen(QKPACK_METRICS_TIMERS_M5_RATE));
	yajl_gen_double(g,  ConvertRateUnit(timer->GetFiveMinuteRate()));

	yajl_gen_string(g, (const unsigned char *) QKPACK_METRICS_TIMERS_M15_RATE, strlen(QKPACK_METRICS_TIMERS_M15_RATE));
	yajl_gen_double(g,  ConvertRateUnit(timer->GetFifteenMinuteRate()));
	

	const char *duration = DurationUnitInSec();
	yajl_gen_string(g, (const unsigned char *) QKPACK_METRICS_TIMERS_DURATION_UNITS, strlen(QKPACK_METRICS_TIMERS_DURATION_UNITS));
	yajl_gen_string(g, (const unsigned char *) duration, strlen(duration));
	
	const char *rate = RateUnitInSec();
	yajl_gen_string(g, (const unsigned char *) QKPACK_METRICS_TIMERS_RATE_UNITS, strlen(QKPACK_METRICS_TIMERS_RATE_UNITS));
	yajl_gen_string(g, (const unsigned char *) rate, strlen(rate));
}


double QkPackMetrics::ConvertDurationUnit(double duration) const
{
    return duration * duration_factor_;
}


double QkPackMetrics::ConvertRateUnit(double rate) const
{
    return rate * rate_factor_;
}


const char* QkPackMetrics::RateUnitInSecMeter() {
		
	int64_t unit_count = rate_unit_.count();

	if (unit_count >= one_day ) {
		return "events/day";
	} else if (unit_count >= one_hour) {
		return "events/hour";
	} else if (unit_count >= one_minute) {
		return "events/minute";
	} else if (unit_count >= one_seconds) {
		return "events/second";
	} else if (unit_count >= one_millisecond) {
		return "events/millisecond";
	}

	return "events/microsecond";
}


const char* QkPackMetrics::RateUnitInSec() {
		
	int64_t unit_count = rate_unit_.count();

	if (unit_count >= one_day ) {
		return "calls/day";
	} else if (unit_count >= one_hour) {
		return "calls/hour";
	} else if (unit_count >= one_minute) {
		return "calls/minute";
	} else if (unit_count >= one_seconds) {
		return "calls/second";
	} else if (unit_count >= one_millisecond) {
		return "calls/millisecond";
	}

	return "calls/microsecond";
}


const char* QkPackMetrics::DurationUnitInSec() 
{	
	int64_t unit_count = duration_unit_.count();

	if (unit_count >= one_day ) {
		return "day";
	} else if (unit_count >= one_hour) {
		return "hour";
	} else if (unit_count >= one_minute) {
		return "minute";
	} else if (unit_count >= one_seconds) {
		return "seconds";
	} else if (unit_count >= one_millisecond) {
		return "millisecond";
	}

	return "microsecond";
}


