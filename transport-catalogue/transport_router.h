#pragma once
#include <ranges>

#include "domain.h"
#include "router.h"
#include "transport_catalogue.h"

namespace transport {
    struct RouteItem {
        std::string type;
        double time;
        std::string stop_name;
        std::string bus;
        int span_count;
    };

    struct Route {
        double total_time;
        std::vector<RouteItem> route_items;
    };

    struct RouterSettings {
        double bus_wait_time;
        double bus_velocity;
    };

    class Router {
        using WeightType = double;
        static constexpr double TO_MPH = 1000. / 60.;

    public:
        Router() = delete;

        Router(const Catalogue& catalogue, const RouterSettings& settings);

        [[nodiscard]]
        std::optional<Route> PlotRoute(std::string_view from_stop,
                                       std::string_view to_stop) const;

    private:
        RouterSettings settings_;
        const Catalogue& catalogue_;
        graph::DirectedWeightedGraph<WeightType> graph_;
        std::unique_ptr<graph::Router<WeightType>> router_;

        std::unordered_map<std::string_view, graph::EdgeId> stopname_to_stop_edgeid_;
        std::vector<std::string_view> vertexid_to_stopname_;

        struct EdgeInfo {
            std::string_view bus_number;
            int stops_count;
        };
        std::unordered_map<graph::EdgeId, EdgeInfo> edgeid_to_edgeinfo_;

        void BuildGraph();

        [[nodiscard]]
        std::optional<std::reference_wrapper<const graph::Edge<double>>> GetStopEdge(std::string_view stop_name) const;

        graph::VertexId GetVertextId(std::string_view stop_name);

        [[nodiscard]]
        const graph::Edge<double>& CreateStopEdge(std::string_view stop_name);

        [[nodiscard]]
        const graph::Edge<double>& GetOrCreateStopEdge(std::string_view stop_name);

        void AddRoutesToGraph();

        Route GenerateRouteInformation(const graph::Router<double>::RouteInfo& route_info) const;

        template<typename IterType>
        void CreateRouteBetweenStops(IterType from_stop, IterType to_stop, const Bus& bus);
    };

    template <typename IterType>
    void Router::CreateRouteBetweenStops(const IterType from_stop, const IterType to_stop, const Bus& bus) {
        const std::optional<int> distance = catalogue_.GetDistanceBetweenStopsOnOneRoute(from_stop, to_stop, bus);
        double travel_time = settings_.bus_wait_time;
        if (distance.has_value()) {
            travel_time = static_cast<double>(distance.value()) / (settings_.bus_velocity * TO_MPH);
        }
        const graph::VertexId from_id = GetOrCreateStopEdge(from_stop->lock()->name).to;
        const graph::VertexId to_id = GetOrCreateStopEdge(to_stop->lock()->name).from;
        const graph::EdgeId edge_id = graph_.AddEdge({from_id, to_id, travel_time});
        edgeid_to_edgeinfo_[edge_id] = {bus.number, static_cast<int>(std::abs(to_stop - from_stop))};
    }
}
