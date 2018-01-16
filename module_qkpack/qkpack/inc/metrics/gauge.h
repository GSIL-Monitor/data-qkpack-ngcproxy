#ifndef GAUGE_H_
#define GAUGE_H_

#include <boost/shared_ptr.hpp>
#include <boost/cstdint.hpp>
#include "metrics/metric.h"

namespace com { 
namespace youku { 
namespace data {
namespace qkpack {
namespace metrics {


/**
 * A gauge metric is an instantaneous reading of a particular value. Used typically
 * to instrument a queue size, backlog etc.
 *
 */
class Gauge: public Metric {
public:
    virtual ~Gauge() {
    }

    /**
     * @return the current value of the guage.
     */
    virtual boost::int64_t GetValue() = 0;
};

typedef boost::shared_ptr<Gauge> GaugePtr;

} /* namespace metrics */
} /* namespace qkpack */
} /* namespace data */
} /* namespace youku */
} /* namespace com */

#endif /* GAUGE_H_ */

