#pragma once

#include <sstream>

#include "transport_catalogue.h"
#include "geo.h"
#include "json.h"
#include "json_builder.h"


namespace requesthandler {
    using namespace std::literals;

    class RequestHandler {
    public:
        explicit RequestHandler(transport::Catalogue& catalogue);

        json::Document GetDocument();

        void PrepareStop(int request_id, std::string_view stop_name);

        void PrepareRoute(int request_id, std::string_view route_number);

        void PrepareMap(int request_id, const std::string& str);

    private:
        transport::Catalogue& catalogue_;
        json::Builder builder_;

        struct RouteInfo {
            int stop_count, unique_stop_count;
            double route_length, curvature;
        };

        RouteInfo GetRouteInfo(std::string_view route_number) const;

        static size_t CountStops(const transport::Route& route);

        static size_t CountUniqueStops(const transport::Route& route);

        [[nodiscard]] int GetBusRouteDistance(std::string_view route_number) const;

        [[nodiscard]] int GetBusRouteDistance(const transport::Route& route) const;

        static double GetRouteGeoDistance(const transport::Route& route);

        [[nodiscard]] double GetRouteCurvature(const transport::Route& route) const;
    };
}
