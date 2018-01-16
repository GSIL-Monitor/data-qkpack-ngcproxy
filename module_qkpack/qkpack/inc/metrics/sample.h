#ifndef __SAMPLE_H__
#define __SAMPLE_H__

#include "metrics/snapshot.h"

namespace com { 
namespace youku { 
namespace data {
namespace qkpack {
namespace metrics {


/**
 * A statistically representative sample of a data stream.
 */
class Sample {
public:
    virtual ~Sample() {
    }

    /**
     * Clears the values in the sample.
     */
    virtual void Clear() = 0;

    /**
     * Returns the number of values recorded.
     * @return the number of values recorded
     */
    virtual boost::uint64_t Size() const = 0;

    /**
     * Adds a new recorded value to the sample.
     * @param value a new recorded value
     */
    virtual void Update(boost::int64_t value) = 0;

    /**
     * Returns a snapshot of the sample's values.
     * @return a snapshot of the sample's values
     */
    virtual SnapshotPtr GetSnapshot() const = 0;
};

} /* namespace metrics */
} /* namespace qkpack */
} /* namespace data */
} /* namespace youku */
} /* namespace com */

#endif /* __SAMPLE_H__ */

