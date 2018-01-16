#include <boost/foreach.hpp>
#include "metrics/exp_decay_sample.h"
#include "metrics/utils.h"


using namespace com::youku::data::qkpack::metrics;

const double ExpDecaySample::DEFAULT_ALPHA = 0.015;
const Clock::duration ExpDecaySample::RESCALE_THRESHOLD(
        boost::chrono::hours(1));

ExpDecaySample::ExpDecaySample(boost::uint32_t reservoir_size, double alpha) :
        alpha_(alpha), reservoir_size_(reservoir_size), count_(0) {
    Clear();
    rng_.seed(get_millis_from_epoch());
}

ExpDecaySample::~ExpDecaySample() {
}

void ExpDecaySample::Clear() {
    values_.clear();
    count_ = 0;
    start_time_ = Clock::now();
    next_scale_time_ = start_time_ + RESCALE_THRESHOLD;
}

boost::uint64_t ExpDecaySample::Size() const {
    return std::min(reservoir_size_, count_.load());
}

void ExpDecaySample::Update(boost::int64_t value) {
    Update(value, Clock::now());
}

void ExpDecaySample::Update(boost::int64_t value,
        const Clock::time_point& timestamp) {
    RescaleIfNeeded(timestamp);
    boost::random::uniform_real_distribution<> dist(0, 1);
    boost::chrono::seconds dur = boost::chrono::duration_cast<
            boost::chrono::seconds>(timestamp - start_time_);
    double priority = 0.0;
    do {
        priority = std::exp(alpha_ * dur.count()) / dist(rng_);
    } while (std::isnan(priority));

    boost::uint64_t count = ++count_;
    if (count <= reservoir_size_) {
        values_[priority] = value;
    } else {
        Double2Int64Map::iterator first_itt(values_.begin());
        double first = first_itt->first;
        if (first < priority
                && values_.insert(std::make_pair(priority, value)).second) {
            values_.erase(first_itt);
        }
    }
}

void ExpDecaySample::RescaleIfNeeded(const Clock::time_point& now) {
    if (next_scale_time_ < now) {
        Clock::time_point prevStartTime = start_time_;
        next_scale_time_ = now + RESCALE_THRESHOLD;
        prevStartTime = start_time_;
        start_time_ = now;
        Rescale(prevStartTime);
    }
}

void ExpDecaySample::Rescale(const Clock::time_point& prevStartTime) {
    Double2Int64Map old_values;
    std::swap(values_, old_values);
    BOOST_FOREACH (const Double2Int64Map::value_type& kv, old_values) {
        boost::chrono::seconds dur = boost::chrono::duration_cast<
                boost::chrono::seconds>(start_time_ - prevStartTime);
        double key = kv.first * std::exp(-alpha_ * dur.count());
        values_[key] = kv.second;
    }
    count_ = values_.size();
}

SnapshotPtr ExpDecaySample::GetSnapshot() const {
    ValueVector vals;
    vals.reserve(values_.size());
    BOOST_FOREACH (const Double2Int64Map::value_type& kv, values_) {
        vals.push_back(kv.second);
    }
    return SnapshotPtr(new Snapshot(vals));
}

