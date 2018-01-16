#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/unordered_map.hpp>
#include "metrics/metric_registry.h"

using namespace com::youku::data::qkpack::metrics;

class MetricRegistry::Impl {
public:
    Impl();
    ~Impl();

    bool AddGauge(const std::string& name, GaugePtr metric);
    bool RemoveMetric(const std::string& name);

    CounterPtr counter(const std::string& name);
    HistogramPtr histogram(const std::string& name);
    MeterPtr meter(const std::string& name);
    TimerPtr timer(const std::string& name);

    CounterMap GetCounters() const;
    HistogramMap GetHistograms() const;
    MeteredMap GetMeters() const;
    TimerMap GetTimers() const;
    GaugeMap GetGauges() const;

    size_t Count() const;
private:
    // Old C++98 style enum.
    enum MetricType {
        GaugeType = 0,
        CounterType,
        HistogramType,
        MeterType,
        TimerType,
        TotalTypes
    };

    // We should use a lock-free concurrent map implementation outside of boost.
    typedef boost::unordered_map<std::string, MetricPtr> MetricSet;
    MetricSet metric_set_[TotalTypes];
    typedef std::set<std::string> StringSet;
    StringSet metric_names_;

    template<typename MetricClass>
    bool IsInstanceOf(const MetricPtr& metric_ptr) const;

    bool AddMetric(MetricSet& metric_set,
            const std::string& name,
            MetricPtr metric);

    MetricPtr BuildMetric(MetricType metric_type) const;
    MetricPtr GetOrAdd(MetricType metric_type, const std::string& name);

    template<typename MetricClass>
    std::map<std::string, boost::shared_ptr<MetricClass> > GetMetrics(const MetricSet& metric_set) const;
};

MetricRegistry::Impl::Impl() {

}

MetricRegistry::Impl::~Impl() {

}

size_t MetricRegistry::Impl::Count() const {
    return metric_names_.size();
}

// RTTI is a performance overhead, should probably replace it in future.
template<typename MetricClass>
bool MetricRegistry::Impl::IsInstanceOf(const MetricPtr& metric_ptr) const {
    boost::shared_ptr<MetricClass> stored_metric(
            boost::dynamic_pointer_cast<MetricClass>(metric_ptr));
    return (stored_metric.get() != NULL);
}

MetricPtr MetricRegistry::Impl::BuildMetric(MetricType metric_type) const {
    MetricPtr metric_ptr;
    switch (metric_type) {
    case CounterType:
        return boost::make_shared<Counter>();
    case HistogramType:
        return boost::make_shared<Histogram>();
    case MeterType:
        return boost::make_shared<Meter>();
    case TimerType:
        return boost::make_shared<Timer>();
    default:
        throw std::invalid_argument("Unknown or invalid metric type.");
    };
}

bool MetricRegistry::Impl::AddMetric(MetricSet& metric_set,
        const std::string& name,
        MetricPtr new_metric) {
    StringSet::iterator s_itt(metric_names_.find(name));
    if (s_itt == metric_names_.end()) {
        metric_names_.insert(name);
        return metric_set.insert(std::make_pair(name, new_metric)).second;
    }
    throw std::invalid_argument(
            name + " already exists as a different metric.");
}

MetricPtr MetricRegistry::Impl::GetOrAdd(MetricType metric_type,
        const std::string& name) {
    MetricSet& metric_set(metric_set_[metric_type]);
    MetricSet::iterator itt(metric_set.find(name));
    if (itt != metric_set.end()) {
        return itt->second;
    } else {
        MetricPtr new_metric(BuildMetric(metric_type));
        AddMetric(metric_set, name, new_metric);
        return new_metric;
    }
}

bool MetricRegistry::Impl::AddGauge(const std::string& name, GaugePtr gauge) {
    return AddMetric(metric_set_[GaugeType], name, gauge);
}

CounterPtr MetricRegistry::Impl::counter(const std::string& name) {
    MetricPtr metric_ptr(GetOrAdd(CounterType, name));
    return boost::static_pointer_cast<Counter>(metric_ptr);
}

HistogramPtr MetricRegistry::Impl::histogram(const std::string& name) {
    MetricPtr metric_ptr(GetOrAdd(HistogramType, name));
    return boost::static_pointer_cast<Histogram>(metric_ptr);
}

MeterPtr MetricRegistry::Impl::meter(const std::string& name) {
    MetricPtr metric_ptr(GetOrAdd(MeterType, name));
    return boost::static_pointer_cast<Meter>(metric_ptr);
}

TimerPtr MetricRegistry::Impl::timer(const std::string& name) {
    MetricPtr metric_ptr(GetOrAdd(TimerType, name));
    return boost::static_pointer_cast<Timer>(metric_ptr);
}

template<typename MetricClass>
std::map<std::string, boost::shared_ptr<MetricClass> >
MetricRegistry::Impl::GetMetrics(const MetricSet& metric_set) const {
    std::map<std::string, boost::shared_ptr<MetricClass> > ret_set;
    BOOST_FOREACH (const MetricSet::value_type& kv, metric_set) {
        ret_set[kv.first] = boost::static_pointer_cast<MetricClass>(kv.second);
    }
    return ret_set;
}

CounterMap MetricRegistry::Impl::GetCounters() const {
    return GetMetrics<Counter>(metric_set_[CounterType]);
}

HistogramMap MetricRegistry::Impl::GetHistograms() const {
    return GetMetrics<Histogram>(metric_set_[HistogramType]);
}

MeteredMap MetricRegistry::Impl::GetMeters() const {
    return GetMetrics<Metered>(metric_set_[MeterType]);
}

TimerMap MetricRegistry::Impl::GetTimers() const {
    return GetMetrics<Timer>(metric_set_[TimerType]);
}

GaugeMap MetricRegistry::Impl::GetGauges() const {
    return GetMetrics<Gauge>(metric_set_[GaugeType]);
}

bool MetricRegistry::Impl::RemoveMetric(const std::string& name) {
    StringSet::iterator s_itt(metric_names_.find(name));
    if (s_itt != metric_names_.end()) {
        for (size_t i = 0; i < TotalTypes; ++i) {
            if (metric_set_[i].erase(name) > 0) {
                break;
            }
        }
        metric_names_.erase(s_itt);
        return true;
    }
    return false;
}

// <=================Implementation end============>

MetricRegistryPtr MetricRegistry::DEFAULT_REGISTRY() {
    static MetricRegistryPtr g_metric_registry(new MetricRegistry());
    return g_metric_registry;
}

MetricRegistry::MetricRegistry() :
        impl_(new MetricRegistry::Impl()) {
}

MetricRegistry::~MetricRegistry() {
}

CounterPtr MetricRegistry::counter(const std::string& name) {
    return impl_->counter(name);
}

HistogramPtr MetricRegistry::histogram(const std::string& name) {
    return impl_->histogram(name);
}

MeterPtr MetricRegistry::meter(const std::string& name) {
    return impl_->meter(name);
}

TimerPtr MetricRegistry::timer(const std::string& name) {
    return impl_->timer(name);
}

CounterMap MetricRegistry::GetCounters() const {
    return impl_->GetCounters();
}

HistogramMap MetricRegistry::GetHistograms() const {
    return impl_->GetHistograms();
}

MeteredMap MetricRegistry::GetMeters() const {
    return impl_->GetMeters();
}

TimerMap MetricRegistry::GetTimers() const {
    return impl_->GetTimers();
}

GaugeMap MetricRegistry::GetGauges() const {
    return impl_->GetGauges();
}

size_t MetricRegistry::Count() const {
    return impl_->Count();
}

bool MetricRegistry::AddGauge(const std::string& name, GaugePtr metric) {
    return impl_->AddGauge(name, metric);
}

bool MetricRegistry::RemoveMetric(const std::string& name) {
    return impl_->RemoveMetric(name);
}

