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
#include "domain.h"

namespace transport {

    class Catalogue {
    public:
        void AddStop(const transport::Stop &stop);

        void AddStop(std::string_view name, geo::Coordinates coordinates);

        void AddBus(Bus &&bus);

        void AddBus(std::string_view number, const std::vector<std::string> &stops, bool is_circle_route = false);

        void AddDistance(const Stop &from_stop, const Stop &to_stop, int length);

        const Stop &GetStop(std::string_view stop_name) const;

        Stop &GetStop(std::string_view stop_name);

        bool HasStop(std::string_view stop_name) const;

        const Bus &GetBus(std::string_view bus_name) const;

        bool HasBus(std::string_view bus_name) const;

        BusInfo GetBusInfo(std::string_view bus_name) const;

    private:
        std::deque<Stop> stops_;
        std::unordered_map<std::string_view, Stop *> stopnames_to_stops_;

        std::deque<Bus> buses_;
        std::unordered_map<std::string_view, Bus *> busnumber_to_buses_;

        std::unordered_map<std::pair<const Stop *, const Stop *>, int, details::StopPtrHasher> distances_;

        int GetBusRouteDistance(std::string_view bus_name) const;

        double GetBusRouteGeoDistance(std::string_view bus_name) const;

        int GetDistanceBetweenStops(const Stop &from_stop, const Stop &to_stop) const;
    };
}