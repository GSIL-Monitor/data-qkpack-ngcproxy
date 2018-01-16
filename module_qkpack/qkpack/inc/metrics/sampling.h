#ifndef __SAMPLING_H__
#define __SAMPLING_H__

#include "metrics/snapshot.h"

namespace com { 
namespace youku { 
namespace data {
namespace qkpack {
namespace metrics {


/**
 * The interface for all classes that sample values.
 */
class Sampling {
public:
    enum SampleType {
        kUniform, kBiased
    };
    virtual ~Sampling() {
    }

    /**
     * Returns the snapshot of values in the sample.
     * @return the snapshot of values in the sample.
     */
    virtual SnapshotPtr GetSnapshot() const = 0;
};

} /* namespace metrics */
} /* namespace qkpack */
} /* namespace data */
} /* namespace youku */
} /* namespace com */

#endif /* __SAMPLING_H__ */

