#include "transport_catalogue.h"

namespace transport {
    Stop &Catalogue::add_stop(transport::Stop &&stop) {
        stops_.push_back(std::move(stop));
        Stop &added_stop = stops_.back();
        stopnames_to_stops_[added_stop.get_name()] = &added_stop;
        return added_stop;
    }

    Stop &Catalogue::add_stop(const std::string_view name, double latitude, double longitude) {
        return add_stop(std::move(Stop{name, latitude, longitude}));
    }

    void Catalogue::add_bus(std::string_view number, std::vector<std::string> &stops, bool is_circle_route) {
        std::deque<Stop *> stops_deque;
        for (std::string_view stop: stops) {
            stops_deque.push_back(stopnames_to_stops_.at(stop));
        }
        add_bus(Bus{number, std::move(stops_deque), is_circle_route});
    }

    void Catalogue::add_bus(Bus &&bus) {
        buses_.push_back(std::move(bus));
        Bus &added_bus = buses_.back();
        for (Stop *stop: added_bus.get_stops()) {
            stop->add_bus(&added_bus);
        }
        busnumber_to_buses_[added_bus.get_number()] = &added_bus;
    }

    const Stop &Catalogue::get_stop(std::string_view stop_name) const {
        return *stopnames_to_stops_.at(stop_name);
    }

    Stop &Catalogue::get_stop(std::string_view stop_name) {
        return *stopnames_to_stops_.at(stop_name);
    }

    const Bus &Catalogue::get_bus(std::string_view bus_name) const {
        return *busnumber_to_buses_.at(bus_name);
    }

    int Catalogue::get_bus_route_distance(std::string_view bus_name) const {
        const Bus *bus = busnumber_to_buses_.at(bus_name);
        int distance = 0;
        const auto &route = bus->get_stops();
        bool is_circle_route = bus->is_circle();
        for (auto iterator = next(route.begin()); iterator != route.end(); iterator = next(iterator)) {
            std::pair<Stop *, Stop *> stops_pair{*(iterator - 1), *iterator};
            distance += get_distance_between_stops(stops_pair);
            if (!is_circle_route) {
                std::pair<Stop *, Stop *> reverse_stops_pair{*iterator, *(iterator - 1)};
                distance += get_distance_between_stops(reverse_stops_pair);
            }
        }

        return distance;
    }

    double Catalogue::get_bus_route_geo_distance(std::string_view bus_name) const {
        const Bus *bus = busnumber_to_buses_.at(bus_name);
        return bus->get_route_geo_distance();
    }

    const Coordinates &Stop::get_coordinates() const {
        return coordinates_;
    }

    std::string_view Stop::get_name() const {
        return name_;
    }

    void Stop::add_bus(Bus *bus) {
        buses_.insert(bus);
    }

    const std::set<Bus *, details::BusComparator> &Stop::get_buses() const {
        return buses_;
    }

    std::string_view Bus::get_number() const {
        return number_;
    }

    size_t Bus::get_stops_count() const {
        return circle_route_ ? route_.size() : route_.size() * 2 - 1;
    }

    size_t Bus::count_unique_stops() const {
        std::set<std::string_view> unique_stops;
        for (const Stop *stop: route_) {
            unique_stops.insert(stop->get_name());
        }
        return unique_stops.size();
    }

    double Bus::get_route_geo_distance() const {
        return route_distance_;
    }

    Bus::Bus(std::string_view number, std::deque<Stop *> &&stops, bool is_circle_route) : number_(number),
                                                                                          route_(std::move(stops)),
                                                                                          circle_route_(
                                                                                                  is_circle_route) {
        calculate_distance();
    }

    void Bus::calculate_distance() {
        route_distance_ = 0;
        for (auto iterator = std::next(route_.begin()); iterator != route_.end(); iterator = std::next(iterator)) {
            auto &stop1 = **iterator;
            auto &stop2 = **std::prev(iterator);
            route_distance_ += ComputeDistance(stop1.get_coordinates(), stop2.get_coordinates());
        }
        if (!circle_route_) {
            route_distance_ *= 2;
        }
    }

    std::deque<Stop *> &Bus::get_stops() {
        return route_;
    }

    const std::deque<Stop *> &Bus::get_stops() const {
        return route_;
    }

    namespace details {

    }
}