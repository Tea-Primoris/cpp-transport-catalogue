#pragma once
#include <unordered_map>
#include <vector>

#include "domain.h"

namespace transport {
    class Catalogue {
    public:
        void SetDistance(std::string_view from_stop, std::string_view to_stop, int distance);

        void AddStop(Stop&& stop);

        bool HasStop(std::string_view stop_name) const;

        const Stop& GetStop(std::string_view stop_name) const;

        void AddRoute(Route&& route);

        bool HasRoute(std::string_view route_number) const;

        void AddRoute(std::string_view route_number, const std::vector<std::string>& stops, bool is_circular);

        const Route& GetRoute(std::string_view route_number) const;

        int GetDistanceBetweenStops(const Stop& from_stop, const Stop& to_stop);

        const std::vector<std::shared_ptr<Stop>>& GetAllStops() const;

        const std::vector<std::shared_ptr<Route>>& GetAllRoutes() const;

    private:
        std::vector<std::shared_ptr<Stop>> stops_;
        std::unordered_map<std::string_view, std::weak_ptr<Stop>> stopnames_to_stops;

        std::unordered_map<std::pair<const Stop*, const Stop*>, int, details::StopPtrHasher> distances_;

        std::vector<std::shared_ptr<Route>> routes_;
        std::unordered_map<std::string_view, std::weak_ptr<Route>> routenumber_to_route;

        std::weak_ptr<Stop> GetStopWeak(std::string_view stop_name) const;
    };
}
