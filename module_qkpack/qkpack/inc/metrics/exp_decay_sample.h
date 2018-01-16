#ifndef __EXP_DECAY_SAMPLE_H__
#define __EXP_DECAY_SAMPLE_H__

#include <vector>
#include <boost/atomic.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_real_distribution.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/scoped_ptr.hpp>
#include "metrics/types.h"
#include "metrics/sample.h"

namespace com { 
namespace youku { 
namespace data {
namespace qkpack {
namespace metrics {

/**
 * An exponentially-decaying random reservoir of {@code long}s. Uses Cormode et al's
 * forward-decaying priority reservoir sampling method to produce a statistically representative
 * sampling reservoir, exponentially biased towards newer entries.
 */
class ExpDecaySample: public Sample {
public:

    /**
     * Creates a new {@link ExpDecaySample} of the given size and alpha factor.
     * @param size  the number of samples to keep in the sampling reservoir
     * @param alpha the exponential decay factor; the higher this is, the more biased the reservoir
     *              will be towards newer values
     */
    ExpDecaySample(boost::uint32_t size = 1024, double alpha = DEFAULT_ALPHA);
    virtual ~ExpDecaySample();

    virtual void Clear();

    /**
     * Returns the number of values recorded.
     * @return the number of values recorded
     */
    virtual boost::uint64_t Size() const;

    /**
     * Adds a new recorded value to the reservoir.
     * @param value a new recorded value
     */
    virtual void Update(boost::int64_t value);

    /**
     * Adds an old value with a fixed timestamp to the reservoir.
     * @param value     the value to be added
     * @param timestamp the epoch timestamp of {@code value} in seconds
     */
    virtual void Update(boost::int64_t value,
            const Clock::time_point& timestamp);

    /**
     * Returns a snapshot of the reservoir's values.
     * @return a snapshot of the reservoir's values
     */
    virtual SnapshotPtr GetSnapshot() const;

private:
    static const double DEFAULT_ALPHA;
    static const Clock::duration RESCALE_THRESHOLD;

    void RescaleIfNeeded(const Clock::time_point& when);
    void Rescale(const Clock::time_point& old_start_time);

    const double alpha_;
    const boost::uint64_t reservoir_size_;
    boost::atomic<boost::uint64_t> count_;

    Clock::time_point start_time_;
    Clock::time_point next_scale_time_;

    typedef std::map<double, boost::int64_t> Double2Int64Map;
    Double2Int64Map values_;
    mutable boost::mt11213b rng_;
};

} /* namespace metrics */
} /* namespace qkpack */
} /* namespace data */
} /* namespace youku */
} /* namespace com */

#endif /* __EXP_DECAY_SAMPLE_H__ */

