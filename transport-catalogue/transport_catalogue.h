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

        [[nodiscard]] std::string_view GetNumber() const;

        [[nodiscard]] size_t GetStopsCount() const;

        [[nodiscard]] size_t CountUniqueStops() const;

        [[nodiscard]] double GetRouteGeoDistance() const;

        [[nodiscard]] const std::deque<Stop *> &GetStops() const;

        [[nodiscard]] std::deque<Stop *> &GetStops();

        [[nodiscard]] bool IsCircle() const {
            return circle_route_;
        }

    private:
        std::string number_;
        std::deque<Stop *> route_;
        bool circle_route_ = false;
        double route_distance_ = 0;

        void CalculateDistance();
    };

    namespace details {
        struct BusComparator {
            bool operator()(const Bus *lhs, const Bus *rhs) const {
                return lhs->GetNumber() < rhs->GetNumber();
            }
        };
    }

    class Stop {
    public:
        Stop() = delete;

        Stop(std::string_view name, double latitude, double longitude) : name_(name), coordinates_({latitude, longitude}) {}

        Stop(std::string_view name, Coordinates &&coordinates) : name_(name), coordinates_(coordinates) {}

        [[nodiscard]] std::string_view GetName() const;

        [[nodiscard]] const Coordinates &GetCoordinates() const;

        void AddBus(Bus *bus);

        [[nodiscard]] const std::set<Bus *, details::BusComparator> &GetBusses() const;

    private:
        std::string name_;
        Coordinates coordinates_;
        std::set<Bus *, details::BusComparator> buses_;
    };

    namespace details {
        struct StopPtrHasher {
            size_t operator()(const std::pair<Stop *, Stop *> pair) const {
                const size_t first_stop_hash = std::hash<std::string_view>{}(pair.first->GetName());
                const size_t second_stop_hash = std::hash<std::string_view>{}(pair.second->GetName());
                return (first_stop_hash * 37) + (second_stop_hash * (37 ^ 2));
            }
        };
    }

    class Catalogue {
    public:
        Stop &AddStop(transport::Stop &&stop);

        Stop &AddStop(const std::string_view name, Coordinates coordinates);

        void AddBus(Bus &&bus);

        void AddBus(std::string_view number, const std::vector<std::string> &stops, bool is_circle_route = false);

        void AddDistance(Stop &from_stop, Stop &to_stop, int length) {
            std::pair<transport::Stop *, transport::Stop *> stop_pair;
            stop_pair.first = &from_stop;
            stop_pair.second = &to_stop;
            distances_[stop_pair] = length;
        }

        const Stop &GetStop(std::string_view stop_name) const;

        Stop &GetStop(std::string_view stop_name);

        bool HasStop(std::string_view stop_name) const;

        const Bus &GetBus(std::string_view bus_name) const;

        bool HasBus(std::string_view bus_name) const;

        int GetBusRouteDistance(std::string_view bus_name) const;

        double GetBusRouteGeoDistance(std::string_view bus_name) const;

        int GetDistanceBetweenStops(Stop &from_stop, Stop &to_stop) const {
            std::pair<transport::Stop *, transport::Stop *> stops_pair;
            stops_pair.first = &from_stop;
            stops_pair.second = &to_stop;
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