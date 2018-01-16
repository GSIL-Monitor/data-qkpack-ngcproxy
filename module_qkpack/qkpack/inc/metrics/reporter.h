#ifndef __REPORTER_H__
#define __REPORTER_H__

#include "metrics/metric_registry.h"

namespace com { 
namespace youku { 
namespace data {
namespace qkpack {
namespace metrics {


/**
 * The interface for all the reporter sub classes.
 */
class Reporter {
public:

	
	virtual ~Reporter() {
    }

    /**
     * reports the metrics.
     */
	virtual void Report() = 0;
};

} /* namespace metrics */
} /* namespace qkpack */
} /* namespace data */
} /* namespace youku */
} /* namespace com */

#endif /* __REPORTER_H__ */

