#ifndef __TIMER_CONTEXT_H__
#define __TIMER_CONTEXT_H__

#include <boost/chrono.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include "metrics/types.h"

namespace com { 
namespace youku { 
namespace data {
namespace qkpack {
namespace metrics {

class Timer;

/**
 * Class that actually measures the wallclock time.
 */
class TimerContext {
public:

    /**
     * Creates a TimerContext.
     * @param timer The parent timer metric.
     */
    TimerContext(Timer& timer);

    ~TimerContext();

    /**
     * Resets the underlying clock.
     */
    void Reset();

    /**
     * Stops recording the elapsed time and updates the timer.
     * @return the elapsed time in nanoseconds
     */
    boost::chrono::nanoseconds Stop();
private:

    TimerContext& operator=(const TimerContext&);

    Clock::time_point start_time_; ///< The start time on instantitation */
    Timer& timer_;                 ///< The parent timer object. */
    bool active_;                  ///< Whether the timer is active or not */
};

typedef boost::shared_ptr<TimerContext> TimerContextPtr;

} /* namespace metrics */
} /* namespace qkpack */
} /* namespace data */
} /* namespace youku */
} /* namespace com */

#endif /* __TIMER_CONTEXT_H__ */

