#include "map_renderer.h"

namespace renderer {
    MapRenderer::MapRenderer(RenderSettings settings, const SphereProjector& projector): settings_(std::move(settings)),
        projector_(projector) {}

    void MapRenderer::Render(std::ostream& out_stream) const {
        map_.Render(out_stream);
    }

    void MapRenderer::AddRouteToMap(const transport::Route& route) {
        const std::string route_color = PickColor();
        DrawRouteLine(route, route_color);
    }

    void MapRenderer::AddRouteNumberAtStop(const std::string& text, geo::Coordinates coordinates,
                                           const std::string& color) {
        svg::Text basic_text;
        basic_text.SetData(text).SetPosition(projector_(coordinates)).SetOffset(settings_.bus_label_offset)
                  .SetFontFamily("Verdana"s).SetFontWeight("bold"s).SetFontSize(settings_.bus_label_font_size);

        svg::Text substrate = basic_text;
        substrate.SetFillColor(settings_.underlayer_color)
                 .SetStrokeColor(settings_.underlayer_color).SetStrokeWidth(settings_.underlayer_width)
                 .SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        svg::Text route_number = basic_text;
        route_number.SetFillColor(color);

        map_.Add(substrate);
        map_.Add(route_number);
    }

    void MapRenderer::AddRouteNumberToMap(const transport::Route& route) {
        const std::string color = PickColor();

        AddRouteNumberAtStop(route.number, route.stops.front().lock()->coordinates, color);

        if (!route.is_circular && route.stops.front().lock().get() != route.stops.back().lock().get()) {
            AddRouteNumberAtStop(route.number, route.stops.back().lock()->coordinates, color);
        }
    }

    void MapRenderer::SetCurrentColor(const size_t color_number) {
        current_color_ = color_number;
    }

    void MapRenderer::DrawStopCircle(const transport::Stop& stop) {
        svg::Circle circle;
        circle.SetCenter(projector_(stop.coordinates)).SetRadius(settings_.stop_radius).SetFillColor("white"s);
        map_.Add(circle);
    }

    void MapRenderer::DrawStopName(const transport::Stop& stop) {
        svg::Text basic_text;
        basic_text.SetData(stop.name).SetPosition(projector_(stop.coordinates)).SetOffset(settings_.stop_label_offset)
                  .SetFontSize(settings_.stop_label_font_size).SetFontFamily("Verdana"s);

        svg::Text substrate = basic_text;
        substrate.SetFillColor(settings_.underlayer_color)
                 .SetStrokeColor(settings_.underlayer_color).SetStrokeWidth(settings_.underlayer_width)
                 .SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        svg::Text stop_name = basic_text;
        stop_name.SetFillColor("black"s);

        map_.Add(substrate);
        map_.Add(stop_name);
    }

    std::string MapRenderer::PickColor() {
        std::string color = settings_.color_palette.at(current_color_);
        current_color_ = (current_color_ + 1) < settings_.color_palette.size() ? current_color_ + 1 : 0;
        return color;
    }

    void MapRenderer::DrawRouteLine(const transport::Route& route, const std::string& route_color) {
        const auto& stops = route.stops;

        svg::Polyline route_line;
        route_line.SetFillColor(svg::NoneColor).SetStrokeColor(route_color).SetStrokeWidth(settings_.line_width)
                  .SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        for (const std::weak_ptr<transport::Stop>& stop : route.stops) {
            route_line.AddPoint(projector_(stop.lock()->coordinates));
        }
        if (!route.is_circular) {
            std::for_each(stops.rbegin() + 1, stops.rend(), [&](const std::weak_ptr<transport::Stop>& stop) {
                route_line.AddPoint(projector_.operator()(stop.lock()->coordinates));
            });
        }

        map_.Add(route_line);
    }
}
