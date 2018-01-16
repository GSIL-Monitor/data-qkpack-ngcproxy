#include "metrics/uniform_sample.h"
#include "metrics/exp_decay_sample.h"
#include "metrics/histogram.h"

using namespace com::youku::data::qkpack::metrics;

Histogram::Histogram(SampleType sample_type) {
    if (sample_type == kUniform) {
        sample_.reset(new UniformSample());
    } else if (sample_type == kBiased) {
        sample_.reset(new ExpDecaySample());
    } else {
        throw std::invalid_argument("invalid sample_type.");
    }
    Clear();
}

Histogram::~Histogram() {
}

void Histogram::Clear() {
    count_ = 0;
    sample_->Clear();
}

boost::uint64_t Histogram::GetCount() const {
    return count_;
}

SnapshotPtr Histogram::GetSnapshot() const {
    return sample_->GetSnapshot();
}

void Histogram::Update(boost::int64_t value) {
    ++count_;
    sample_->Update(value);
}

