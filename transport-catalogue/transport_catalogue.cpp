#include "transport_catalogue.h"

namespace transport {
    void Catalogue::AddStop(transport::Stop &&stop) {
        stops_.push_back(std::move(stop));
        Stop &added_stop = stops_.back();
        stopnames_to_stops_[added_stop.GetName()] = &added_stop;
    }

    void Catalogue::AddStop(std::string_view name, const Coordinates coordinates) {
        AddStop(std::move(Stop{name, coordinates.lat, coordinates.lng}));
    }

    void Catalogue::AddBus(std::string_view number, const std::vector<std::string> &stops, bool is_circle_route) {
        std::deque<Stop *> stops_deque;
        for (std::string_view stop: stops) {
            stops_deque.push_back(stopnames_to_stops_.at(stop));
        }
        AddBus(Bus{number, std::move(stops_deque), is_circle_route});
    }

    void Catalogue::AddBus(Bus &&bus) {
        buses_.push_back(std::move(bus));
        Bus &added_bus = buses_.back();
        for (Stop *stop: added_bus.GetStops()) {
            stop->AddBus(&added_bus);
        }
        busnumber_to_buses_[added_bus.GetNumber()] = &added_bus;
    }

    const Stop &Catalogue::GetStop(std::string_view stop_name) const {
        return *stopnames_to_stops_.at(stop_name);
    }

    Stop &Catalogue::GetStop(std::string_view stop_name) {
        return *stopnames_to_stops_.at(stop_name);
    }

    bool Catalogue::HasStop(std::string_view stop_name) const {
        return stopnames_to_stops_.find(stop_name) != stopnames_to_stops_.end();
    }

    const Bus &Catalogue::GetBus(std::string_view bus_name) const {
        return *busnumber_to_buses_.at(bus_name);
    }

    bool Catalogue::HasBus(std::string_view bus_name) const {
        return busnumber_to_buses_.find(bus_name) != busnumber_to_buses_.end();
    }

    int Catalogue::GetBusRouteDistance(std::string_view bus_name) const {
        const Bus *bus = busnumber_to_buses_.at(bus_name);
        int distance = 0;
        const auto &route = bus->GetStops();
        bool is_circle_route = bus->IsCircle();
        for (auto iterator = next(route.begin()); iterator != route.end(); iterator = next(iterator)) {
            distance += GetDistanceBetweenStops(**(iterator - 1), **iterator);
            if (!is_circle_route) {
                distance += GetDistanceBetweenStops(**iterator, **(iterator - 1));
            }
        }

        return distance;
    }

    double Catalogue::GetBusRouteGeoDistance(std::string_view bus_name) const {
        const Bus *bus = busnumber_to_buses_.at(bus_name);
        return bus->GetRouteGeoDistance();
    }

    void Catalogue::AddDistance(const Stop &from_stop, const Stop &to_stop, int length) {
        std::pair<const transport::Stop *, const transport::Stop *> stop_pair;
        stop_pair.first = &from_stop;
        stop_pair.second = &to_stop;
        distances_[stop_pair] = length;
    }

    int Catalogue::GetDistanceBetweenStops(const Stop &from_stop, const Stop &to_stop) const {
        std::pair<const transport::Stop *, const transport::Stop *> stops_pair;
        stops_pair.first = &from_stop;
        stops_pair.second = &to_stop;

        auto distance_it = distances_.find(stops_pair);
        if (distance_it == distances_.end()) {
            std::pair<const Stop *, const Stop *> reverse_stops_pair;
            reverse_stops_pair.first = stops_pair.second;
            reverse_stops_pair.second = stops_pair.first;
            return distances_.at(reverse_stops_pair);
        } else {
            return distance_it->second;
        }
    }

    BusInfo Catalogue::GetBusInfo(std::string_view bus_name) const {
        const Bus &bus = GetBus(bus_name);
        BusInfo bus_info{};
        bus_info.stops_on_route = bus.GetStopsCount();
        bus_info.unique_stops = bus.CountUniqueStops();
        bus_info.route_length = GetBusRouteDistance(bus_name);
        bus_info.curvature = bus_info.route_length / GetBusRouteGeoDistance(bus_name);
        return bus_info;
    }

    const Coordinates &Stop::GetCoordinates() const {
        return coordinates_;
    }

    std::string_view Stop::GetName() const {
        return name_;
    }

    void Stop::AddBus(Bus *bus) {
        buses_.insert(bus);
    }

    const std::set<Bus *, details::BusComparator> &Stop::GetBusses() const {
        return buses_;
    }

    std::string_view Bus::GetNumber() const {
        return number_;
    }

    size_t Bus::GetStopsCount() const {
        return circle_route_ ? route_.size() : route_.size() * 2 - 1;
    }

    size_t Bus::CountUniqueStops() const {
        std::set<std::string_view> unique_stops;
        for (const Stop *stop: route_) {
            unique_stops.insert(stop->GetName());
        }
        return unique_stops.size();
    }

    double Bus::GetRouteGeoDistance() const {
        return route_distance_;
    }

    Bus::Bus(std::string_view number, std::deque<Stop *> &&stops, bool is_circle_route) : number_(number),
                                                                                          route_(std::move(stops)),
                                                                                          circle_route_(
                                                                                                  is_circle_route) {
        CalculateDistance();
    }

    void Bus::CalculateDistance() {
        route_distance_ = 0;
        for (auto iterator = std::next(route_.begin()); iterator != route_.end(); iterator = std::next(iterator)) {
            auto &stop1 = **iterator;
            auto &stop2 = **std::prev(iterator);
            route_distance_ += ComputeDistance(stop1.GetCoordinates(), stop2.GetCoordinates());
        }
        if (!circle_route_) {
            route_distance_ *= 2;
        }
    }

    std::deque<Stop *> &Bus::GetStops() {
        return route_;
    }

    const std::deque<Stop *> &Bus::GetStops() const {
        return route_;
    }

    namespace details {

        size_t StopPtrHasher::operator()(const std::pair<const Stop *, const Stop *> pair) const {
            const size_t first_stop_hash = std::hash<std::string_view>{}(pair.first->GetName());
            const size_t second_stop_hash = std::hash<std::string_view>{}(pair.second->GetName());
            return (first_stop_hash * 37) + (second_stop_hash * (37 ^ 2));
        }

        bool BusComparator::operator()(const Bus *lhs, const Bus *rhs) const {
            return lhs->GetNumber() < rhs->GetNumber();
        }
    }
}