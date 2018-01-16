#ifndef __METER_H__
#define __METER_H__

#include <boost/chrono.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/atomic.hpp>
#include "metrics/metered.h"

namespace com { 
namespace youku { 
namespace data {
namespace qkpack {
namespace metrics {


/**
 * A meter metric which measures mean throughput and one-, five-, and fifteen-minute
 * exponentially-weighted moving average throughputs.
 */
class Meter: public Metered {
public:
    /**
     * Creates a meter with the specified rate unit.
     * @param rate_unit The rate unit in nano seconds.
     */
    Meter(boost::chrono::nanoseconds rate_unit = boost::chrono::seconds(1));

    virtual ~Meter();

	virtual boost::chrono::nanoseconds GetRateUnit() const;

	
	/**
     * @returns the number of events that have been marked.
     */
    virtual boost::uint64_t GetCount() const;

    /**
     * @return the fifteen-minute exponentially-weighted moving average rate at which events have
     *         occurred since the meter was created.
     */
    virtual double GetFifteenMinuteRate();

    /**
     * @return the five-minute exponentially-weighted moving average rate at which events have
     *         occurred since the meter was created.
     */
    virtual double GetFiveMinuteRate();

    /**
     * @return the one-minute exponentially-weighted moving average rate at which events have
     *         occurred since the meter was created.
     */
    virtual double GetOneMinuteRate();

    /**
     * @return the mean rate at which events have occurred since the meter was created.
     */
    virtual double GetMeanRate();

    /**
     * Mark the occurrence of a given number of events.
     * @param n the number of events with the default being 1.
     */
    void Mark(boost::uint64_t n = 1);

private:
    class Impl;
    boost::scoped_ptr<Impl> impl_;
};

typedef boost::shared_ptr<Meter> MeterPtr;

} /* namespace metrics */
} /* namespace qkpack */
} /* namespace data */
} /* namespace youku */
} /* namespace com */

#endif /* __METER_H__ */

