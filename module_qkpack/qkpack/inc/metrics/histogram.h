#ifndef __HISTOGRAM_H__
#define __HISTOGRAM_H__

#include <boost/cstdint.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/atomic.hpp>

#include "metrics/metric.h"
#include "metrics/sampling.h"
#include "metrics/sample.h"

namespace com { 
namespace youku { 
namespace data {
namespace qkpack {
namespace metrics {


class Histogram: public Metric, Sampling {
public:
    /**
     * Creates a new histogram based on the sample type.
     * @param sample_type the sample to use internally.
     * @see SamplingInterface for different types of samples.
     */
    Histogram(SampleType sample_type = kBiased);
    virtual ~Histogram();

    /**
     * @return the current snapshot based on the sample.
     */
    virtual SnapshotPtr GetSnapshot() const;

    /**
     * Adds a recorded value.
     * @param value The length of the value.
     */
    void Update(boost::int64_t value);

    /**
     * @return The number of values recorded until now.
     */
    boost::uint64_t GetCount() const;

    /**
     * Clears the underlying sample.
     */
    void Clear();

    /**< The Maximum sample size at any given time. */
    static const boost::uint64_t DEFAULT_SAMPLE_SIZE;
private:
    static const double DEFAULT_ALPHA;

    boost::scoped_ptr<Sample> sample_; /**< The underlying sample implementation. */
    boost::atomic<boost::uint64_t> count_; /**< The number of samples. */
};

typedef boost::shared_ptr<Histogram> HistogramPtr;

} /* namespace metrics */
} /* namespace qkpack */
} /* namespace data */
} /* namespace youku */
} /* namespace com */

#endif /* HISTOGRAM_H_ */

