#include "transport_catalogue.h"

namespace transport {
    void Catalogue::SetDistance(std::string_view from_stop, std::string_view to_stop, int distance) {
        std::pair<const Stop* const, const Stop* const> from_to_pair(&GetStop(from_stop), &GetStop(to_stop));
        distances_.emplace(from_to_pair, distance);
    }

    void Catalogue::AddStop(Stop&& stop) {
        stops_.push_back(std::make_shared<Stop>(std::move(stop)));
        std::weak_ptr added_stop = stops_.back();
        stopnames_to_stops[added_stop.lock()->name] = std::move(added_stop);
    }

    bool Catalogue::HasStop(std::string_view stop_name) const {
        return stopnames_to_stops.find(stop_name) != stopnames_to_stops.end();
    }

    const Stop& Catalogue::GetStop(std::string_view stop_name) const {
        return *stopnames_to_stops.at(stop_name).lock();
    }

    std::weak_ptr<Stop> Catalogue::GetStopWeak(std::string_view stop_name) const {
        return stopnames_to_stops.at(stop_name).lock();
    }

    void Catalogue::AddRoute(Route&& route) {
        routes_.push_back(std::make_shared<Route>(std::move(route)));
        std::weak_ptr added_route = routes_.back();
        for (const std::weak_ptr<Stop>& stop : added_route.lock()->stops) {
            stop.lock()->passing_routes.emplace(added_route.lock().get());
        }
        routenumber_to_route[added_route.lock()->number] = std::move(added_route);
    }

    bool Catalogue::HasRoute(std::string_view route_number) const {
        return routenumber_to_route.find(route_number) != routenumber_to_route.end();
    }

    void Catalogue::AddRoute(const std::string_view route_number, const std::vector<std::string>& stops,
                             const bool is_circular) {
        Route new_route;
        new_route.number = route_number;
        for (const std::string_view stop_name : stops) {
            new_route.stops.push_back(GetStopWeak(stop_name));
        }
        new_route.is_circular = is_circular;
        AddRoute(std::move(new_route));
    }

    const Route& Catalogue::GetRoute(const std::string_view route_number) const {
        return *routenumber_to_route.at(route_number).lock();
    }

    int Catalogue::GetDistanceBetweenStops(const Stop& from_stop, const Stop& to_stop) {
        std::pair<const transport::Stop*, const transport::Stop*> stops_pair;
        stops_pair.first = &from_stop;
        stops_pair.second = &to_stop;

        const auto distance_iterator = distances_.find(stops_pair);
        if (distance_iterator == distances_.end()) {
            std::pair<const Stop*, const Stop*> reverse_stops_pair;
            reverse_stops_pair.first = stops_pair.second;
            reverse_stops_pair.second = stops_pair.first;
            return distances_.at(reverse_stops_pair);
        }
        else {
            return distance_iterator->second;
        }
    }

    const std::vector<std::shared_ptr<Stop>>& Catalogue::GetAllStops() const {
        return stops_;
    }

    const std::vector<std::shared_ptr<Route>>& Catalogue::GetAllRoutes() const {
        return routes_;
    }
}
