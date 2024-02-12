#include "transport_catalogue.h"

#include <algorithm>

namespace transport {
    void Catalogue::SetDistance(std::string_view from_stop, std::string_view to_stop, int distance) {
        std::pair<const Stop * const, const Stop * const> from_to_pair(&GetStop(from_stop), &GetStop(to_stop));
        distances_.emplace(from_to_pair, distance);
    }

    void Catalogue::AddStop(Stop&& stop) {
        stops_.push_back(std::make_shared<Stop>(std::move(stop)));
        std::weak_ptr added_stop = stops_.back();
        stopnames_to_stops_[added_stop.lock()->name] = std::move(added_stop);
    }

    bool Catalogue::HasStop(std::string_view stop_name) const {
        return stopnames_to_stops_.find(stop_name) != stopnames_to_stops_.end();
    }

    const Stop& Catalogue::GetStop(std::string_view stop_name) const {
        return *stopnames_to_stops_.at(stop_name).lock();
    }

    std::weak_ptr<Stop> Catalogue::GetStopWeak(std::string_view stop_name) const {
        return stopnames_to_stops_.at(stop_name).lock();
    }

    void Catalogue::AddBus(Bus&& bus) {
        busses_.push_back(std::make_shared<Bus>(std::move(bus)));
        std::weak_ptr added_bus = busses_.back();
        for (const std::weak_ptr<Stop>& stop : added_bus.lock()->stops) {
            stop.lock()->passing_busses.emplace(added_bus.lock().get());
        }
        busnumber_to_bus_[added_bus.lock()->number] = std::move(added_bus);
    }

    bool Catalogue::HasBus(const std::string_view bus_number) const {
        return busnumber_to_bus_.find(bus_number) != busnumber_to_bus_.end();
    }

    void Catalogue::AddBus(const std::string_view bus_number, const std::vector<std::string>& stops,
                           const bool is_circular) {
        Bus new_bus;
        new_bus.number = bus_number;
        for (const std::string_view stop_name : stops) {
            new_bus.stops.push_back(GetStopWeak(stop_name));
        }
        new_bus.is_circular = is_circular;
        AddBus(std::move(new_bus));
    }

    const Bus& Catalogue::GetBus(const std::string_view bus_number) const {
        return *busnumber_to_bus_.at(bus_number).lock();
    }

    std::optional<int> Catalogue::GetDistanceBetweenStops(const Stop& from_stop, const Stop& to_stop) const {
        std::pair<const Stop *, const Stop *> stops_pair;
        stops_pair.first = &from_stop;
        stops_pair.second = &to_stop;

        if (const auto distance_iterator = distances_.find(stops_pair); distance_iterator == distances_.end()) {
            std::pair<const Stop *, const Stop *> reverse_stops_pair;
            reverse_stops_pair.first = stops_pair.second;
            reverse_stops_pair.second = stops_pair.first;
            if (const auto reverse_stops_distance = distances_.find(reverse_stops_pair);
                reverse_stops_distance != distances_.end()) {
                return reverse_stops_distance->second;
            }
            return std::nullopt;
        }
        else {
            return distance_iterator->second;
        }
    }

    const std::vector<std::shared_ptr<Stop>>& Catalogue::GetAllStops() const {
        return stops_;
    }

    const std::vector<std::shared_ptr<Bus>>& Catalogue::GetAllBusses() const {
        return busses_;
    }
}
