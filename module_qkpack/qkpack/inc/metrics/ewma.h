#ifndef __EWMA_H__
#define __EWMA_H__

#include <boost/scoped_ptr.hpp>
#include <boost/chrono.hpp>
#include <boost/atomic.hpp>

namespace com { 
namespace youku { 
namespace data {
namespace qkpack {
namespace metrics {


/**
 * An exponentially-weighted moving average.
 * describe in detail  http://en.wikipedia.org/wiki/Moving_average#Exponential_moving_average
 * not thread-safe.
 */
class EWMA {
public:
    /**
     * Creates a new EWMA which is equivalent to the UNIX one minute load average and which expects
     * to be ticked every 5 seconds.
     * @return a one-minute EWMA
     */
    static EWMA OneMinuteEWMA() {
        return EWMA(M1_ALPHA, boost::chrono::seconds(INTERVAL_IN_SEC));
    }

    /**
     * Creates a new EWMA which is equivalent to the UNIX five minute load average and which expects
     * to be ticked every 5 seconds.
     * @return a five-minute EWMA
     */
    static EWMA FiveMinuteEWMA() {
        return EWMA(M5_ALPHA, boost::chrono::seconds(INTERVAL_IN_SEC));
    }

    /**
     * Creates a new EWMA which is equivalent to the UNIX fifteen minute load average and which expects
     * to be ticked every 5 seconds.
     * @return a five-minute EWMA
     */
    static EWMA FifteenMinuteEWMA() {
        return EWMA(M15_ALPHA, boost::chrono::seconds(INTERVAL_IN_SEC));
    }

    /**
     * Create a new EWMA with a specific smoothing constant.
     * @param alpha        the smoothing constant
     * @param interval     the expected tick interval
     */
    EWMA(double alpha, boost::chrono::nanoseconds interval);
    EWMA(const EWMA &other);
    ~EWMA();

    /**
     * Update the moving average with a new value.
     * @param n the new value
     */
    void Update(boost::uint64_t n);

    /**
     * Mark the passage of time and decay the current rate accordingly.
     */
    void Tick();

    /**
     * Returns the rate in the given units of time.
     * @param rate_unit the unit of time
     * @return the rate
     */
    double GetRate(boost::chrono::nanoseconds rate_unit =
            boost::chrono::seconds(1)) const;
private:

    static const int INTERVAL_IN_SEC;
    static const int ONE_MINUTE;
    static const int FIVE_MINUTES;
    static const int FIFTEEN_MINUTES;
    static const double M1_ALPHA;
    static const double M5_ALPHA;
    static const double M15_ALPHA;

    boost::atomic<bool> initialized_;
    boost::atomic<double> ewma_;
    boost::atomic<boost::uint64_t> uncounted_;
    const double alpha_;
    const boost::uint64_t interval_nanos_;
};

} /* namespace metrics */
} /* namespace qkpack */
} /* namespace data */
} /* namespace youku */
} /* namespace com */

#endif /* __EWMA_H__ */

