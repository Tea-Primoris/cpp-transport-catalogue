#pragma once

#include <iomanip>
#include <memory>
#include <queue>

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "json.h"
#include "transport_router.h"

namespace jsonreader {
    using namespace std::literals;

    class JSONReader {
    public:
        JSONReader() = delete;

        explicit JSONReader(transport::Catalogue& catalogue, requesthandler::RequestHandler& request_handler)
            : catalogue_(catalogue), request_handler_(request_handler) {}

        void ReadInput(std::istream& input_stream);
        const renderer::MapRenderer& GetMapRenderer() const;

    private:
        transport::Catalogue& catalogue_;
        requesthandler::RequestHandler& request_handler_;
        std::shared_ptr<renderer::MapRenderer> map_renderer_;
        std::unique_ptr<transport::Router> router_;

        void ProcessBaseRequests(const json::Array& requests_array) const;

        void AddStopToCatalogue(const json::Dict& stop_object) const;

        static std::pair<std::string, const json::Dict*> CreateDistancePair(const json::Dict& stop_object);

        void ProcessDistances(std::queue<std::pair<std::string, const json::Dict*>>& distances_to_process) const;

        void ProcessBusses(std::queue<const json::Dict*>& busses_to_process) const;

        static std::vector<std::string> CreateStopsVector(const json::Array& stops);

        void AddBusToCatalogue(const json::Dict& stop_object) const;

        void ProcessStatRequests(const json::Array& requests_array) const;

        [[nodiscard]] renderer::SphereProjector GenerateSphereProjector(double width, double height,
                                                                        double padding) const;

        static std::vector<std::string> GenerateColorPalette(const json::Array& color_array);

        static svg::Point GenerateOffsetPoint(const json::Array& array);

        static std::string GenerateColorStringFromArray(const json::Array& array);

        void ConstructMapRenderer(const json::Dict& requests_array);

        std::vector<std::shared_ptr<transport::Bus>> GetSortedBusses() const;

        std::vector<std::weak_ptr<transport::Stop>> GetSortedStops() const;

        void ProcessRenderSettings(const json::Dict& requests_array);

        void ProcessRoutingSettings(const json::Dict& routing_settings);
    };
}
