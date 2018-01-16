#ifndef __TIMER_H__
#define __TIMER_H__

#include <string>
#include <boost/chrono.hpp>
#include <boost/cstdint.hpp>
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include "metrics/metered.h"
#include "metrics/metric.h"
#include "metrics/sampling.h"
#include "metrics/timer_context.h"
#include "metrics/meter.h"
#include "metrics/histogram.h"

namespace com { 
namespace youku { 
namespace data {
namespace qkpack {
namespace metrics {

/**
 * A timer metric which aggregates timing durations and provides duration statistics, plus
 * throughput statistics via {@link Meter} and {@link Histogram}.
 */
class Timer: public Metered, Sampling {
public:
    /**
     * Creates a new {@link Timer} using an {@link ExpDecaySample}.
     */
    Timer();
    virtual ~Timer();

	virtual boost::chrono::nanoseconds GetRateUnit() const;
    /**
     * @returns the number of events that have been measured.
     */
    virtual boost::uint64_t GetCount() const;

    /**
     * @return the fifteen-minute exponentially-weighted moving average rate at which events have
     *         occurred since the timer was created.
     */
    virtual double GetFifteenMinuteRate();

    /**
     * @return the five-minute exponentially-weighted moving average rate at which events have
     *         occurred since the timer was created.
     */
    virtual double GetFiveMinuteRate();

    /**
     * @return the one-minute exponentially-weighted moving average rate at which events have
     *         occurred since the timer was created.
     */
    virtual double GetOneMinuteRate();

    /**
     * @return the average rate at which events have occurred since the meter was created.
     */
    virtual double GetMeanRate();

    /**
     * @return the current snapshot based on the sample.
     */
    virtual SnapshotPtr GetSnapshot() const;

    /**
     * Clears the underlying metrics.
     */
    void Clear();

    /**
     * Adds a recorded duration.
     * @param duration the length of the duration in nanos.
     */
    void Update(boost::chrono::nanoseconds duration);

    /**
     * Creates a new TimerContext instance that measures the duration and updates the
     * duration before the instance goes out of scope.
     * @return The TimerContext object.
     * @note The TimerContextPtr should not be shared.
     */
    TimerContextPtr timerContextPtr() {
        return boost::shared_ptr<TimerContext>(new TimerContext(*this));
    }

    /**
     * Times the duration of a function that will be executed internally and updates the duration.
     * @param The fn to be timed.
     */
    void Time(boost::function<void()> fn);

private:
    Meter meter_; /**< The underlying meter object */
    Histogram histogram_; /**< The underlying histogram object */
};

typedef boost::shared_ptr<Timer> TimerPtr;

} /* namespace metrics */
} /* namespace qkpack */
} /* namespace data */
} /* namespace youku */
} /* namespace com */

#endif /* __TIMER_H__ */

