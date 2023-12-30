#include "request_handler.h"

namespace requesthandler {
    RequestHandler::RequestHandler(transport::Catalogue& catalogue): catalogue_(catalogue) {
        builder_.StartArray();
    }

    json::Document RequestHandler::GetDocument() {
        return json::Document{builder_.EndArray().Build()};
    }

    void RequestHandler::PrepareStop(int request_id, std::string_view stop_name) {
        builder_.StartDict().Key("request_id"s).Value(request_id);

        if (!catalogue_.HasStop(stop_name)) {
            builder_.Key("error_message"s).Value("not found"s).EndDict();
            return;
        }

        builder_.Key("buses"s).StartArray();
        const transport::Stop& stop = catalogue_.GetStop(stop_name);
        for (const transport::Route* const passing_route : stop.passing_routes) {
            builder_.Value(passing_route->number);
        }
        builder_.EndArray().EndDict();
    }

    void RequestHandler::PrepareRoute(int request_id, std::string_view route_number) {
        builder_.StartDict().Key("request_id").Value(request_id);

        if (!catalogue_.HasRoute(route_number)) {
            builder_.Key("error_message"s).Value("not found"s).EndDict();
            return;
        }

        RouteInfo route_info = GetRouteInfo(route_number);
        builder_.Key("route_length"s).Value(route_info.route_length)
                .Key("curvature"s).Value(route_info.curvature)
                .Key("stop_count"s).Value(route_info.stop_count)
                .Key("unique_stop_count"s).Value(route_info.unique_stop_count)
                .EndDict();
    }

    void RequestHandler::PrepareMap(int request_id, const std::string& str) {
        builder_.StartDict().Key("request_id").Value(request_id)
                .Key("map"s).Value(str).EndDict();
    }

    RequestHandler::RouteInfo RequestHandler::GetRouteInfo(std::string_view route_number) const {
        const transport::Route& route = catalogue_.GetRoute(route_number);
        const int stop_count = static_cast<int>(CountStops(route));
        const int unique_stops_count = static_cast<int>(CountUniqueStops(route));
        const double route_length = GetBusRouteDistance(route);
        const double route_curvature = GetRouteCurvature(route);
        return {stop_count, unique_stops_count, route_length, route_curvature};
    }

    size_t RequestHandler::CountStops(const transport::Route& route) {
        return route.is_circular ? route.stops.size() : route.stops.size() * 2 - 1;
    }

    size_t RequestHandler::CountUniqueStops(const transport::Route& route) {
        std::set<std::string_view> unique_stops;
        for (const std::weak_ptr<transport::Stop>& stop : route.stops) {
            unique_stops.insert(stop.lock()->name);
        }
        return unique_stops.size();
    }

    int RequestHandler::GetBusRouteDistance(std::string_view route_number) const {
        return GetBusRouteDistance(catalogue_.GetRoute(route_number));
    }

    int RequestHandler::GetBusRouteDistance(const transport::Route& route) const {
        int distance = 0;

        for (auto stop_iterator = std::next(route.stops.begin());
             stop_iterator != route.stops.end(); stop_iterator = std::next(stop_iterator)) {
            const transport::Stop& from_stop = *std::prev(stop_iterator)->lock();
            const transport::Stop& to_stop = *stop_iterator->lock();
            distance += catalogue_.GetDistanceBetweenStops(from_stop, to_stop);
            if (!route.is_circular) {
                distance += catalogue_.GetDistanceBetweenStops(to_stop, from_stop);
            }
        }

        return distance;
    }

    double RequestHandler::GetRouteGeoDistance(const transport::Route& route) {
        double route_distance = 0.;
        for (auto stop_iterator = std::next(route.stops.begin());
             stop_iterator != route.stops.end(); ++stop_iterator) {
            const transport::Stop& from_stop = *std::prev(stop_iterator)->lock();
            const transport::Stop& to_stop = *stop_iterator->lock();
            route_distance += geo::ComputeDistance(from_stop.coordinates, to_stop.coordinates);
        }
        if (!route.is_circular) {
            route_distance *= 2.;
        }
        return route_distance;
    }

    double RequestHandler::GetRouteCurvature(const transport::Route& route) const {
        return static_cast<double>(GetBusRouteDistance(route)) / GetRouteGeoDistance(route);
    }
}
