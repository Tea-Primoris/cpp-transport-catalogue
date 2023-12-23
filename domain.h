#pragma once

#include <deque>
#include <string>
#include <set>

#include "geo.h"

namespace transport {
    struct BusInfo {
        size_t stops_on_route;
        size_t unique_stops;
        int route_length;
        double curvature;
    };

    class Stop;

    class Bus {
    public:
        Bus() = delete;

        Bus(std::string_view number, std::deque<Stop *>&& stops, bool is_circle_route);

        [[nodiscard]] std::string_view GetNumber() const;

        [[nodiscard]] size_t GetStopsCount() const;

        [[nodiscard]] size_t CountUniqueStops() const;

        [[nodiscard]] double GetRouteGeoDistance() const;

        [[nodiscard]] const std::deque<Stop *>& GetStops() const;

        [[nodiscard]] std::deque<Stop *>& GetStops();

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
            bool operator()(const Bus* lhs, const Bus* rhs) const;
        };
    }

    class Stop {
    public:
        Stop() = delete;

        Stop(std::string_view name, double latitude, double longitude) : name_(name),
                                                                         coordinates_({latitude, longitude}) {
        }

        Stop(std::string_view name, geo::Coordinates&& coordinates) : name_(name), coordinates_(coordinates) {
        }

        [[nodiscard]] std::string_view GetName() const;

        [[nodiscard]] const geo::Coordinates& GetCoordinates() const;

        void AddBus(Bus* bus);

        [[nodiscard]] const std::set<Bus *, details::BusComparator>& GetBusses() const;

    private:
        std::string name_;
        geo::Coordinates coordinates_;
        std::set<Bus *, details::BusComparator> buses_;
    };

    namespace details {
        struct StopPtrHasher {
            size_t operator()(std::pair<const Stop *, const Stop *> pair) const;
        };
    }
}
