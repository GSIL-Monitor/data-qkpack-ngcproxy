#include "metrics/timer.h"

using namespace com::youku::data::qkpack::metrics;

Timer::Timer() :
        histogram_(Sampling::kBiased) {
}

Timer::~Timer() {
}


boost::chrono::nanoseconds Timer::GetRateUnit() const {
	return meter_.GetRateUnit();
}

boost::uint64_t Timer::GetCount() const {
    return histogram_.GetCount();
}

double Timer::GetFifteenMinuteRate() {
    return meter_.GetFifteenMinuteRate();
}

double Timer::GetFiveMinuteRate() {
    return meter_.GetFiveMinuteRate();
}

double Timer::GetOneMinuteRate() {
    return meter_.GetOneMinuteRate();
}

double Timer::GetMeanRate() {
    return meter_.GetMeanRate();
}

void Timer::Clear() {
    histogram_.Clear();
}

void Timer::Update(boost::chrono::nanoseconds duration) {
    boost::int64_t count = duration.count();
    if (count >= 0) {
        histogram_.Update(count);
        meter_.Mark();
    }
}

SnapshotPtr Timer::GetSnapshot() const {
    return histogram_.GetSnapshot();
}

void Timer::Time(boost::function<void()> func) {
    TimerContext timer_context(*this);
    func();
}

