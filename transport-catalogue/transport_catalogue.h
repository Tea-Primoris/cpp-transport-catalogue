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
            bool operator()(const Bus *lhs, const Bus *rhs) const;
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
            size_t operator()(std::pair<const Stop *, const Stop *> pair) const;
        };
    }

    class Catalogue {
    public:
        void AddStop(transport::Stop &&stop);

        void AddStop(std::string_view name, Coordinates coordinates);

        void AddBus(Bus &&bus);

        void AddBus(std::string_view number, const std::vector<std::string> &stops, bool is_circle_route = false);

        void AddDistance(const Stop &from_stop, const Stop &to_stop, int length);

        const Stop &GetStop(std::string_view stop_name) const;

        Stop &GetStop(std::string_view stop_name);

        bool HasStop(std::string_view stop_name) const;

        const Bus &GetBus(std::string_view bus_name) const;

        bool HasBus(std::string_view bus_name) const;

        int GetBusRouteDistance(std::string_view bus_name) const;

        double GetBusRouteGeoDistance(std::string_view bus_name) const;

        int GetDistanceBetweenStops(const Stop &from_stop, const Stop &to_stop) const;

    private:
        std::deque<Stop> stops_;
        std::unordered_map<std::string_view, Stop *> stopnames_to_stops_;

        std::deque<Bus> buses_;
        std::unordered_map<std::string_view, Bus *> busnumber_to_buses_;

        std::unordered_map<std::pair<const Stop *, const Stop *>, int, details::StopPtrHasher> distances_;
    };
}