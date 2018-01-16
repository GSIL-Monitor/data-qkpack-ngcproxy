#include "metrics/meter.h"
#include "metrics/ewma.h"
#include "metrics/types.h"
#include "metrics/utils.h"

using namespace com::youku::data::qkpack::metrics;

static const uint64_t TICK_INTERVAL =
        Clock::duration(boost::chrono::seconds(5)).count();

class Meter::Impl {
public:
    Impl(boost::chrono::nanoseconds rate_unit);
    ~Impl();

	boost::chrono::nanoseconds GetRateUnit() const;
    boost::uint64_t GetCount() const;
    double GetFifteenMinuteRate();
    double GetFiveMinuteRate();
    double GetOneMinuteRate();
    double GetMeanRate();
    void Mark(boost::uint64_t n);

private:
    const boost::chrono::nanoseconds rate_unit_;
    boost::atomic<boost::uint64_t> count_;
    const Clock::time_point start_time_;
    boost::atomic<boost::uint64_t> last_tick_;
    EWMA m1_rate_;
    EWMA m5_rate_;
    EWMA m15_rate_;

    void Tick();
    void TickIfNecessary();
};

Meter::Impl::Impl(boost::chrono::nanoseconds rate_unit) :
                rate_unit_(rate_unit),
                count_(0),
                start_time_(Clock::now()),
                last_tick_(
                        boost::chrono::duration_cast<boost::chrono::nanoseconds>(
                                start_time_.time_since_epoch()).count()),
                m1_rate_(EWMA::OneMinuteEWMA()),
                m5_rate_(EWMA::FiveMinuteEWMA()),
                m15_rate_(EWMA::FifteenMinuteEWMA()) {
}

Meter::Impl::~Impl() {
}

boost::chrono::nanoseconds Meter::Impl::GetRateUnit() const {
	return rate_unit_;
}


boost::uint64_t Meter::Impl::GetCount() const {
    return count_;
}

double Meter::Impl::GetFifteenMinuteRate() {
    TickIfNecessary();
    return m15_rate_.GetRate();
}

double Meter::Impl::GetFiveMinuteRate() {
    TickIfNecessary();
    return m5_rate_.GetRate();
}

double Meter::Impl::GetOneMinuteRate() {
    TickIfNecessary();
    return m1_rate_.GetRate();
}

double Meter::Impl::GetMeanRate() {
    boost::uint64_t c = count_;
    if (c > 0) {
        boost::chrono::nanoseconds elapsed = boost::chrono::duration_cast<
                boost::chrono::nanoseconds>(Clock::now() - start_time_);
        return static_cast<double>(c * rate_unit_.count()) / elapsed.count();
    }
    return 0.0;
}

void Meter::Impl::Mark(boost::uint64_t n) {
    TickIfNecessary();
    count_ += n;
    m1_rate_.Update(n);
    m5_rate_.Update(n);
    m15_rate_.Update(n);
}

void Meter::Impl::Tick() {
    m1_rate_.Tick();
    m5_rate_.Tick();
    m15_rate_.Tick();
}

void Meter::Impl::TickIfNecessary() {
    boost::uint64_t old_tick = last_tick_;
    boost::uint64_t cur_tick =
            boost::chrono::duration_cast<boost::chrono::nanoseconds>(
                    Clock::now().time_since_epoch()).count();
    boost::uint64_t age = cur_tick - old_tick;
    if (age > TICK_INTERVAL) {
        boost::uint64_t new_tick = cur_tick - age % TICK_INTERVAL;
        if (last_tick_.compare_exchange_strong(old_tick, new_tick)) {
            boost::uint64_t required_ticks = age / TICK_INTERVAL;
            for (boost::uint64_t i = 0; i < required_ticks; i++) {
                Tick();
            }
        }
    }
}

Meter::Meter(boost::chrono::nanoseconds rate_unit) :
        impl_(new Meter::Impl(rate_unit)) {
}

Meter::~Meter() {

}


boost::chrono::nanoseconds Meter::GetRateUnit() const {
	return impl_->GetRateUnit();
}

boost::uint64_t Meter::GetCount() const {
    return impl_->GetCount();
}

double Meter::GetFifteenMinuteRate() {
    return impl_->GetFifteenMinuteRate();
}

double Meter::GetFiveMinuteRate() {
    return impl_->GetFiveMinuteRate();
}

double Meter::GetOneMinuteRate() {
    return impl_->GetOneMinuteRate();
}

double Meter::GetMeanRate() {
    return impl_->GetMeanRate();
}

void Meter::Mark(boost::uint64_t n) {
    impl_->Mark(n);
}

