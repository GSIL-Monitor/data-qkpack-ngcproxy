#ifndef __METERED_H__
#define __METERED_H__

#include <boost/shared_ptr.hpp>
#include <boost/cstdint.hpp>
#include <string>
#include <boost/chrono.hpp>
#include "metrics/metric.h"

namespace com { 
namespace youku { 
namespace data {
namespace qkpack {
namespace metrics {


/**
 * Interface for objects which maintains mean and exponentially-weighted rate.
 */
class Metered: public Metric {
public:
    virtual ~Metered() {
    }


	virtual boost::chrono::nanoseconds GetRateUnit() const = 0;
	/**
     * @returns the number of events that have been marked.
     */
    virtual boost::uint64_t GetCount() const = 0;
    /**
     * @return the fifteen-minute exponentially-weighted moving average rate at which events have
     *         occurred since the meter was created.
     */
    virtual double GetFifteenMinuteRate() = 0;
    /**
     * @return the fifteen-minute exponentially-weighted moving average rate at which events have
     *         occurred since the meter was created.
     */
    virtual double GetFiveMinuteRate() = 0;
    /**
     * @return the fifteen-minute exponentially-weighted moving average rate at which events have
     *         occurred since the meter was created.
     */
    virtual double GetOneMinuteRate() = 0;
    /**
     * @return the average rate at which events have occurred since the meter was created.
     */
    virtual double GetMeanRate() = 0;
};

typedef boost::shared_ptr<Metered> MeteredPtr;

} /* namespace metrics */
} /* namespace qkpack */
} /* namespace data */
} /* namespace youku */
} /* namespace com */

#endif /* __METERED_H__ */

