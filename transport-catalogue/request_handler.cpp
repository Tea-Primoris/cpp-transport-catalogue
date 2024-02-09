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
        for (const transport::Bus* const passing_bus : stop.passing_busses) {
            builder_.Value(passing_bus->number);
        }
        builder_.EndArray().EndDict();
    }

    void RequestHandler::PrepareBus(int request_id, std::string_view bus_number) {
        builder_.StartDict().Key("request_id").Value(request_id);

        if (!catalogue_.HasBus(bus_number)) {
            builder_.Key("error_message"s).Value("not found"s).EndDict();
            return;
        }

        BusInfo bus_info = GetBusInfo(bus_number);
        builder_.Key("route_length"s).Value(bus_info.route_length)
                .Key("curvature"s).Value(bus_info.curvature)
                .Key("stop_count"s).Value(bus_info.stop_count)
                .Key("unique_stop_count"s).Value(bus_info.unique_stop_count)
                .EndDict();
    }

    void RequestHandler::PrepareMap(int request_id, const std::string& str) {
        builder_.StartDict().Key("request_id").Value(request_id)
                .Key("map"s).Value(str).EndDict();
    }

    RequestHandler::BusInfo RequestHandler::GetBusInfo(std::string_view bus_number) const {
        const transport::Bus& bus = catalogue_.GetBus(bus_number);
        const int stop_count = static_cast<int>(CountStops(bus));
        const int unique_stops_count = static_cast<int>(CountUniqueStops(bus));
        const double route_length = GetBusRouteDistance(bus);
        const double route_curvature = GetBusCurvature(bus);
        return {stop_count, unique_stops_count, route_length, route_curvature};
    }

    size_t RequestHandler::CountStops(const transport::Bus& bus) {
        return bus.is_circular ? bus.stops.size() : bus.stops.size() * 2 - 1;
    }

    size_t RequestHandler::CountUniqueStops(const transport::Bus& bus) {
        std::set<std::string_view> unique_stops;
        for (const std::weak_ptr<transport::Stop>& stop : bus.stops) {
            unique_stops.insert(stop.lock()->name);
        }
        return unique_stops.size();
    }

    int RequestHandler::GetBusRouteDistance(std::string_view bus_number) const {
        return GetBusRouteDistance(catalogue_.GetBus(bus_number));
    }

    int RequestHandler::GetBusRouteDistance(const transport::Bus& bus) const {
        int distance = 0;

        for (auto stop_iterator = std::next(bus.stops.begin());
             stop_iterator != bus.stops.end(); stop_iterator = std::next(stop_iterator)) {
            const transport::Stop& from_stop = *std::prev(stop_iterator)->lock();
            const transport::Stop& to_stop = *stop_iterator->lock();
            distance += catalogue_.GetDistanceBetweenStops(from_stop, to_stop);
            if (!bus.is_circular) {
                distance += catalogue_.GetDistanceBetweenStops(to_stop, from_stop);
            }
        }

        return distance;
    }

    double RequestHandler::GetBusGeoDistance(const transport::Bus& bus) {
        double route_distance = 0.;
        for (auto stop_iterator = std::next(bus.stops.begin());
             stop_iterator != bus.stops.end(); ++stop_iterator) {
            const transport::Stop& from_stop = *std::prev(stop_iterator)->lock();
            const transport::Stop& to_stop = *stop_iterator->lock();
            route_distance += geo::ComputeDistance(from_stop.coordinates, to_stop.coordinates);
        }
        if (!bus.is_circular) {
            route_distance *= 2.;
        }
        return route_distance;
    }

    double RequestHandler::GetBusCurvature(const transport::Bus& bus) const {
        return static_cast<double>(GetBusRouteDistance(bus)) / GetBusGeoDistance(bus);
    }
}
