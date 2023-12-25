#include "map_renderer.h"

bool maprender::details::IsZero(double value) {
    return std::abs(value) < EPSILON;
}

svg::Point maprender::details::SphereProjector::operator()(geo::Coordinates coords) const {
    return {
        (coords.lng - min_lon_) * zoom_coeff_ + padding_,
        (max_lat_ - coords.lat) * zoom_coeff_ + padding_
    };
}

namespace maprender {
    MapRenderer::MapRenderer(transport::Catalogue& catalogue, std::vector<std::string> color_palette, double width,
                             double height, double padding, double line_width): catalogue_(catalogue),
        color_palette_(std::move(color_palette)), line_width_(line_width) {
        const auto& all_routes = catalogue_.GetAllRoutes();

        std::deque<geo::Coordinates> all_stop_coords;
        for (const auto& route: all_routes) {
            for (const auto& stop: route.GetStops()) {
                all_stop_coords.push_back(stop->GetCoordinates());
            }
        }

        projector_ = std::make_unique<details::SphereProjector>(all_stop_coords.begin(), all_stop_coords.end(),
                                                                width, height, padding);
    }

    void MapRenderer::Render(std::ostream& output_stream) {
        const auto& all_routes = catalogue_.GetAllRoutes();
        std::set<std::string> sorted_routes;
        for (const auto& route: all_routes) {
            sorted_routes.emplace(route.GetNumber());
        }
        for (const auto& route_number: sorted_routes) {
            RenderRoute(catalogue_.GetBus(route_number));
        }
        map_.Render(output_stream);
    }

    void MapRenderer::RenderRoute(const transport::Bus& route) {
        const auto& stops = route.GetStops();
        if (stops.empty()) {
            return;
        }

        svg::Polyline route_line;
        route_line.SetFillColor("none"s).SetStrokeColor(PickStrokeColor()).SetStrokeWidth(line_width_)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        for (const auto& stop: stops) {
            route_line.AddPoint(projector_->operator()(stop->GetCoordinates()));
        }
        if (!route.IsCircle()) {
            std::for_each(stops.rbegin() + 1, stops.rend(), [&](const auto& stop) {
                route_line.AddPoint(projector_->operator()(stop->GetCoordinates()));
            });
        }
        map_.Add(std::move(route_line));
    }

    std::string MapRenderer::PickStrokeColor() {
        std::string color = color_palette_.at(current_color_);
        current_color_ = (current_color_ + 1) < color_palette_.size() ? current_color_ + 1 : 0;
        return color;
    }
}
