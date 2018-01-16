#include "metrics/timer_context.h"
#include "metrics/timer.h"

using namespace com::youku::data::qkpack::metrics;

TimerContext::TimerContext(Timer& timer) :
        timer_(timer) {
    Reset();
}

TimerContext::~TimerContext() {
    Stop();
}

void TimerContext::Reset() {
    active_ = true;
    start_time_ = Clock::now();
}

boost::chrono::nanoseconds TimerContext::Stop() {
    if (active_) {
        boost::chrono::nanoseconds dur = Clock::now() - start_time_;
        timer_.Update(dur);
        active_ = false;
        return dur;
    }
    return boost::chrono::nanoseconds(0);
}

