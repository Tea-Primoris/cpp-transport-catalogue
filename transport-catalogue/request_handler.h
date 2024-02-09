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

        void PrepareBus(int request_id, std::string_view bus_number);

        void PrepareMap(int request_id, const std::string& str);

    private:
        transport::Catalogue& catalogue_;
        json::Builder builder_;

        struct BusInfo {
            int stop_count, unique_stop_count;
            double route_length, curvature;
        };

        BusInfo GetBusInfo(std::string_view bus_number) const;

        static size_t CountStops(const transport::Bus& bus);

        static size_t CountUniqueStops(const transport::Bus& bus);

        [[nodiscard]] int GetBusRouteDistance(std::string_view bus_number) const;

        [[nodiscard]] int GetBusRouteDistance(const transport::Bus& bus) const;

        static double GetBusGeoDistance(const transport::Bus& bus);

        [[nodiscard]] double GetBusCurvature(const transport::Bus& bus) const;
    };
}
