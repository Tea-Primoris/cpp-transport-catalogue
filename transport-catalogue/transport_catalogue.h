#pragma once

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <deque>
#include <vector>
#include <stdexcept>
#include <set>
#include <iostream>

#include "geo.h"

namespace transport {

    class Stop;

    class Bus {
    public:
        Bus() = delete;

        Bus(std::string_view number, std::deque<Stop *> &&stops, bool is_circle_route);

        [[nodiscard]] std::string_view get_number() const;

        [[nodiscard]] size_t get_stops_count() const;

        [[nodiscard]] size_t count_unique_stops() const;

        [[nodiscard]] double get_route_geo_distance() const;

        [[nodiscard]] const std::deque<Stop *> &get_stops() const;

        [[nodiscard]] std::deque<Stop *> &get_stops();

        [[nodiscard]] bool is_circle() const {
            return circle_route_;
        }

    private:
        std::string number_;
        std::deque<Stop *> route_;
        bool circle_route_ = false;
        double route_distance_ = 0;

        void calculate_distance();
    };

    namespace details {
        struct BusComparator {
            bool operator()(const Bus *lhs, const Bus *rhs) const {
                return lhs->get_number() < rhs->get_number();
            }
        };
    }

    class Stop {
    public:
        Stop() = delete;

        Stop(std::string_view name, double latitude, double longitude) : name_(name), coordinates_({latitude, longitude}) {}

        Stop(std::string_view name, Coordinates &&coordinates) : name_(name), coordinates_(coordinates) {}

        [[nodiscard]] std::string_view get_name() const;

        [[nodiscard]] const Coordinates &get_coordinates() const;

        void add_bus(Bus *bus);

        [[nodiscard]] const std::set<Bus *, details::BusComparator> &get_buses() const;

    private:
        std::string name_;
        Coordinates coordinates_;
        std::set<Bus *, details::BusComparator> buses_;
    };

    namespace details {
        struct StopPtrHasher {
            size_t operator()(const std::pair<Stop *, Stop *> pair) const {
                const size_t first_stop_hash = std::hash<std::string_view>{}(pair.first->get_name());
                const size_t second_stop_hash = std::hash<std::string_view>{}(pair.second->get_name());
                return (first_stop_hash * 37) + (second_stop_hash * (37 ^ 2));
            }
        };
    }

    class Catalogue {
    public:
        Stop &add_stop(transport::Stop &&stop);

        Stop &add_stop(const std::string_view name, double latitude, double longitude);

        void add_bus(Bus &&bus);

        void add_bus(std::string_view number, std::vector<std::string> &stops, bool is_circle_route = false);

        void add_distance(std::pair<Stop *, Stop *> &stops, int length) {
            distances_[stops] = length;
        }

        const Stop &get_stop(std::string_view stop_name) const;

        Stop &get_stop(std::string_view stop_name);

        const Bus &get_bus(std::string_view bus_name) const;

        int get_bus_route_distance(std::string_view bus_name) const;

        double get_bus_route_geo_distance(std::string_view bus_name) const;

        int get_distance_between_stops(std::pair<Stop *, Stop *> stops_pair) const {
            try {
                return distances_.at(stops_pair);
            }
            catch (std::out_of_range&) {
                std::pair<Stop *, Stop *> reverse_stops_pair;
                reverse_stops_pair.first = stops_pair.second;
                reverse_stops_pair.second = stops_pair.first;
                return distances_.at(reverse_stops_pair);
            }
        }

    private:
        std::deque<Stop> stops_;
        std::unordered_map<std::string_view, Stop *> stopnames_to_stops_;

        std::deque<Bus> buses_;
        std::unordered_map<std::string_view, Bus *> busnumber_to_buses_;

        std::unordered_map<std::pair<Stop *, Stop *>, int, details::StopPtrHasher> distances_;
    };
}