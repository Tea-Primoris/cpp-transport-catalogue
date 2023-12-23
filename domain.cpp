#include "domain.h"
#include "transport_catalogue.h"

namespace transport {
    const Bus& Catalogue::GetBus(std::string_view bus_name) const {
        return *busnumber_to_buses_.at(bus_name);
    }

    std::string_view Bus::GetNumber() const {
        return number_;
    }

    size_t Bus::GetStopsCount() const {
        return circle_route_ ? route_.size() : route_.size() * 2 - 1;
    }

    size_t Bus::CountUniqueStops() const {
        std::set<std::string_view> unique_stops;
        for (const Stop* stop: route_) {
            unique_stops.insert(stop->GetName());
        }
        return unique_stops.size();
    }

    double Bus::GetRouteGeoDistance() const {
        return route_distance_;
    }

    Bus::Bus(std::string_view number, std::deque<Stop *>&& stops, bool is_circle_route) : number_(number),
        route_(std::move(stops)),
        circle_route_(
            is_circle_route) {
        CalculateDistance();
    }

    void Bus::CalculateDistance() {
        route_distance_ = 0;
        for (auto iterator = std::next(route_.begin()); iterator != route_.end(); iterator = std::next(iterator)) {
            auto& stop1 = **iterator;
            auto& stop2 = **std::prev(iterator);
            route_distance_ += ComputeDistance(stop1.GetCoordinates(), stop2.GetCoordinates());
        }
        if (!circle_route_) {
            route_distance_ *= 2;
        }
    }

    std::deque<Stop *>& Bus::GetStops() {
        return route_;
    }

    const std::deque<Stop *>& Bus::GetStops() const {
        return route_;
    }

    const geo::Coordinates& Stop::GetCoordinates() const {
        return coordinates_;
    }

    std::string_view Stop::GetName() const {
        return name_;
    }

    void Stop::AddBus(Bus* bus) {
        buses_.insert(bus);
    }

    const std::set<Bus *, details::BusComparator>& Stop::GetBusses() const {
        return buses_;
    }
}
