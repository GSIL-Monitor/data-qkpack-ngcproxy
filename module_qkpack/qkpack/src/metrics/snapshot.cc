#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <boost/foreach.hpp>
#include "metrics/snapshot.h"

using namespace com::youku::data::qkpack::metrics;

static const double MEDIAN_Q = 0.5;
static const double P75_Q = 0.75;
static const double P95_Q = 0.95;
static const double P98_Q = 0.98;
static const double P99_Q = 0.99;
static const double P999_Q = 0.999;

Snapshot::Snapshot(const ValueVector& values) :
        values_(values) {
    std::sort(values_.begin(), values_.end());
}

Snapshot::~Snapshot() {
}

std::size_t Snapshot::Size() const {
    return values_.size();
}

double Snapshot::GetValue(double quantile) const {
    if (quantile < 0.0 || quantile > 1.0) {
        throw std::invalid_argument("quantile is not in [0..1]");
    }

    if (values_.empty()) {
        return 0.0;
    }

    const double pos = quantile * (values_.size() + 1);

    if (pos < 1) {
        return values_.front();
    }

    if (pos >= values_.size()) {
        return values_.back();
    }

    const size_t pos_index = static_cast<size_t>(pos);
    double lower = values_[pos_index - 1];
    double upper = values_[pos_index];
    return lower + (pos - std::floor(pos)) * (upper - lower);
}

double Snapshot::GetMedian() const {
    return GetValue(MEDIAN_Q);
}

double Snapshot::Get75thPercentile() const {
    return GetValue(P75_Q);
}

double Snapshot::Get95thPercentile() const {
    return GetValue(P95_Q);
}

double Snapshot::Get98thPercentile() const {
    return GetValue(P98_Q);
}

double Snapshot::Get99thPercentile() const {
    return GetValue(P99_Q);
}

double Snapshot::Get999thPercentile() const {
    return GetValue(P999_Q);
}

ValueVector::value_type Snapshot::GetMin() const {
    return (values_.empty() ? 0.0 : values_.front());
}

ValueVector::value_type Snapshot::GetMax() const {
    return (values_.empty() ? 0.0 : values_.back());
}

double Snapshot::GetMean() const {
    if (values_.empty()) {
        return 0.0;
    }

    ValueVector::value_type mean(0);
    BOOST_FOREACH(ValueVector::value_type d, values_) {
        mean += d;
    }
    return static_cast<double>(mean) / values_.size();
}

double Snapshot::GetStdDev() const {
    const size_t values_size(values_.size());
    if (values_size <= 1) {
        return 0.0;
    }

    double mean_value = GetMean();
    double sum = 0;

    BOOST_FOREACH(ValueVector::value_type value, values_) {
        double diff = static_cast<double>(value) - mean_value;
        sum += diff * diff;
    }

    double variance = sum / (values_size - 1);
    return std::sqrt(variance);
}

const ValueVector& Snapshot::GetValues() const {
    return values_;
}


