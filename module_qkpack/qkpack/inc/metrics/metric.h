#ifndef __METRIC_H__
#define __METRIC_H__

#include <boost/shared_ptr.hpp>

namespace com { 
namespace youku { 
namespace data {
namespace qkpack {
namespace metrics {


/**
 * The base class for all metrics types.
 */
class Metric {
public:
    virtual ~Metric() = 0;
};

inline Metric::~Metric() {
}

typedef boost::shared_ptr<Metric> MetricPtr;

} /* namespace metrics */
} /* namespace qkpack */
} /* namespace data */
} /* namespace youku */
} /* namespace com */

#endif /* __METRIC_H__ */

