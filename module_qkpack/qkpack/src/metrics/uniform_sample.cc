#include "metrics/utils.h"
#include "metrics/uniform_sample.h"

using namespace com::youku::data::qkpack::metrics;

const boost::uint64_t UniformSample::DEFAULT_SAMPLE_SIZE = 1028;
UniformSample::UniformSample(boost::uint32_t reservoir_size) :
        reservoir_size_(reservoir_size), count_(0), values_(reservoir_size, 0) {
    rng_.seed(get_millis_from_epoch());
}

UniformSample::~UniformSample() {
}

void UniformSample::Clear() {
    for (size_t i = 0; i < reservoir_size_; ++i) {
        values_[i] = 0;
    }
    count_ = 0;
}

boost::uint64_t UniformSample::Size() const {
    boost::uint64_t size = values_.size();
    boost::uint64_t count = count_;
    return std::min(count, size);
}

boost::uint64_t UniformSample::GetRandom(boost::uint64_t count) const {
    boost::random::uniform_int_distribution<> uniform(0, count - 1);
    return uniform(rng_);
}

void UniformSample::Update(boost::int64_t value) {
    boost::uint64_t count = ++count_;
    size_t size = values_.size();
    if (count <= size) {
        values_[count - 1] = value;
    } else {
        boost::uint64_t rand = GetRandom(count);
        if (rand < size) {
            values_[rand] = value;
        }
    }
}

SnapshotPtr UniformSample::GetSnapshot() const {
    Int64Vector::const_iterator begin_itr(values_.begin());
    Int64Vector::const_iterator end_itr(values_.begin());
    std::advance(end_itr, Size());
    return SnapshotPtr(new Snapshot(ValueVector(begin_itr, end_itr)));
}

