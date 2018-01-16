#ifndef __SNAPSHOT_H__
#define __SNAPSHOT_H__

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <vector>

namespace com { 
namespace youku { 
namespace data {
namespace qkpack {
namespace metrics {


typedef std::vector<boost::int64_t> ValueVector;

/**
 * A statistical snapshot of a {@link Sample}.
 */
class Snapshot {
public:
    /**
     * Create a new {@link Snapshot} with the given values.
     * @param values    an unordered set of values in the reservoir
     */
    Snapshot(const ValueVector& values);
    ~Snapshot();

    /**
     * Returns the number of values in the snapshot.
     * @return the number of values
     */
    size_t Size() const;

    /**
     * Returns the lowest value in the snapshot.
     * @return the lowest value
     */
    ValueVector::value_type GetMin() const;

    /**
     * Returns the highest value in the snapshot.
     * @return the highest value
     */
    ValueVector::value_type GetMax() const;

    /**
     * Returns the arithmetic mean of the values in the snapshot.
     * @return the arithmetic mean
     */
    double GetMean() const;

    /**
     * Returns the standard deviation of the values in the snapshot.
     * @return the standard deviation value
     */
    double GetStdDev() const;

    /**
     * Returns all the values in the snapshot.
     * @return All the values in the snapshot.
     */
    const ValueVector& GetValues() const;

    /**
     * Returns the value at the given quantile.
     * @param quantile    a given quantile, in {@code [0..1]}
     * @return the value in the distribution at {@code quantile}
     */
    double GetValue(double quantile) const;

    /**
     * Returns the median value in the distribution.
     * @return the median value
     */
    double GetMedian() const;

    /**
     * Returns the value at the 75th percentile in the distribution.
     * @return the value at the 75th percentile
     */
    double Get75thPercentile() const;

    /**
     * Returns the value at the 95th percentile in the distribution.
     * @return the value at the 95th percentile
     */
    double Get95thPercentile() const;

    /**
     * Returns the value at the 98th percentile in the distribution.
     * @return the value at the 98th percentile
     */
    double Get98thPercentile() const;

    /**
     * Returns the value at the 99th percentile in the distribution.
     * @return the value at the 99th percentile
     */
    double Get99thPercentile() const;

    /**
     * Returns the value at the 999th percentile in the distribution.
     * @return the value at the 999th percentile
     */
    double Get999thPercentile() const;
private:
    ValueVector values_;
};

typedef boost::shared_ptr<Snapshot> SnapshotPtr;

} /* namespace metrics */
} /* namespace qkpack */
} /* namespace data */
} /* namespace youku */
} /* namespace com */


#endif /* __SNAPSHOT_H__ */

