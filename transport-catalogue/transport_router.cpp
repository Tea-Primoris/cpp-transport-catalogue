#include "transport_router.h"

namespace transport {
    Router::Router(const Catalogue& catalogue, const RouterSettings& settings)
        : settings_(settings),
          catalogue_(catalogue),
          graph_(catalogue_.GetAllStops().size() * 2) {
        vertexid_to_stopname_.reserve(catalogue_.GetAllStops().size() * 2);
        BuildGraph();
    }

    void Router::BuildGraph() {
        AddRoutesToGraph();
        router_ = std::make_unique<graph::Router<WeightType>>(graph_);
    }

    Route Router::GenerateRouteInformation(const graph::Router<double>::RouteInfo& route_info) const {
        using namespace std::literals;
        Route route;
        route.total_time = route_info.weight;
        for (graph::EdgeId edge_id : route_info.edges) {
            const graph::Edge<double> edge = graph_.GetEdge(edge_id);
            const std::string_view from_stop = vertexid_to_stopname_.at(edge.from);
            const std::string_view to_stop = vertexid_to_stopname_.at(edge.to);
            if (from_stop == to_stop && edge.from < edge.to) {
                RouteItem waiting;
                waiting.type = "Wait"s;
                waiting.time = edge.weight;
                waiting.stop_name = from_stop;
                route.route_items.push_back(waiting);
            }
            else {
                RouteItem busing;
                busing.type = "Bus"s;
                busing.time = edge.weight;
                const auto [bus_number, stops_count] = edgeid_to_edgeinfo_.at(edge_id);
                busing.bus = bus_number;
                busing.span_count = stops_count;
                route.route_items.push_back(busing);
            }
        }
        return route;
    }

    std::optional<Route> Router::PlotRoute(const std::string_view from_stop,
                                           const std::string_view to_stop) const {
        const auto from_edge = GetStopEdge(from_stop);
        const auto to_edge = GetStopEdge(to_stop);
        if (from_edge.has_value() && to_edge.has_value()) {
            const graph::VertexId from_id = from_edge.value().get().from;
            const graph::VertexId to_id = to_edge.value().get().from;
            const std::optional<graph::Router<double>::RouteInfo> route_info = router_->BuildRoute(from_id, to_id);
            if (route_info.has_value()) {
                return GenerateRouteInformation(route_info.value());
            }
        }
        return std::nullopt;
    }

    std::optional<std::reference_wrapper<const graph::Edge<double>>> Router::GetStopEdge(
        const std::string_view stop_name) const {
        if (const auto edge = stopname_to_stop_edgeid_.find(stop_name); edge != stopname_to_stop_edgeid_.end()) {
            return graph_.GetEdge(edge->second);
        }
        return std::nullopt;
    }

    graph::VertexId Router::GetVertextId(const std::string_view stop_name) {
        const graph::VertexId vertex_id = vertexid_to_stopname_.size();
        vertexid_to_stopname_.push_back(stop_name);
        return vertex_id;
    }

    const graph::Edge<double>& Router::CreateStopEdge(const std::string_view stop_name) {
        const graph::VertexId waiting_begin = GetVertextId(stop_name);
        const graph::VertexId waiting_stop = GetVertextId(stop_name);
        const auto stop_edge_id =
            graph_.AddEdge({waiting_begin, waiting_stop, settings_.bus_wait_time});
        stopname_to_stop_edgeid_[stop_name] = stop_edge_id;
        return graph_.GetEdge(stop_edge_id);
    }

    const graph::Edge<double>& Router::GetOrCreateStopEdge(const std::string_view stop_name) {
        if (const auto edge = GetStopEdge(stop_name); edge.has_value()) {
            return edge.value();
        }
        return CreateStopEdge(stop_name);
    }

    void Router::AddRoutesToGraph() {
        for (const std::weak_ptr<Bus> bus : catalogue_.GetAllBusses()) {
            const std::vector<std::weak_ptr<Stop>>& stops = bus.lock()->stops;
            for (auto from_iterator = stops.begin(); from_iterator != std::prev(stops.end());
                 std::advance(from_iterator, 1)) {
                for (auto to_iterator = std::next(from_iterator); to_iterator != stops.end();
                     std::advance(to_iterator, 1)) {
                    CreateRouteBetweenStops(from_iterator, to_iterator, *bus.lock());
                    if (!bus.lock()->is_circular) {
                        CreateRouteBetweenStops(to_iterator, from_iterator, *bus.lock());
                    }
                }
            }
        }
    }
}
