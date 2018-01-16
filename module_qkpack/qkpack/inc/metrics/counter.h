#ifndef __COUNTER_H__
#define __COUNTER_H__

#include <boost/cstdint.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/atomic.hpp>
#include "metrics/metric.h"

namespace com { 
namespace youku { 
namespace data {
namespace qkpack {
namespace metrics {


class Counter: public Metric {
public:

    /**
     * Constructor
     * @param n Initialize the counter with a value of \c n.
     */
    Counter(boost::int64_t n = 0) :
            count_(n) {
    }

    virtual ~Counter() {
    }

    /**
     * @return the current value of the counter.
     */
    boost::int64_t GetCount() const {
        return count_;
    }

    /**
     * @param n reset the counter to the value \c n.
     */
    void SetCount(boost::int64_t n) {
        count_ = n;
    }

    /**
     * @param n increment the counter by \c n
     */
    void Increment(boost::int64_t n = 1) {
        count_ += n;
    }

    /**
     * @param n decrement the counter by \c n
     */
    void Decrement(boost::int64_t n = 1) {
        count_ -= n;
    }

    /**
     * Clears the counter, same as calling <code> setCount(0) </code>;
     */
    void Clear() {
        SetCount(0);
    }
private:
    boost::atomic<boost::int64_t> count_;
};

typedef boost::shared_ptr<Counter> CounterPtr;

} /* namespace metrics */
} /* namespace qkpack */
} /* namespace data */
} /* namespace youku */
} /* namespace com */

#endif /* __COUNTER_H__ */

