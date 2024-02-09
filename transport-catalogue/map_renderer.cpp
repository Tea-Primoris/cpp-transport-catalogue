#include "map_renderer.h"

namespace renderer {
    MapRenderer::MapRenderer(RenderSettings settings, const SphereProjector& projector): settings_(std::move(settings)),
        projector_(projector) {}

    void MapRenderer::Render(std::ostream& out_stream) const {
        map_.Render(out_stream);
    }

    void MapRenderer::AddBusToMap(const transport::Bus& bus) {
        const std::string bus_color = PickColor();
        DrawBusLine(bus, bus_color);
    }

    void MapRenderer::AddBusNumberAtStop(const std::string& text, geo::Coordinates coordinates,
                                           const std::string& color) {
        svg::Text basic_text;
        basic_text.SetData(text).SetPosition(projector_(coordinates)).SetOffset(settings_.bus_label_offset)
                  .SetFontFamily("Verdana"s).SetFontWeight("bold"s).SetFontSize(settings_.bus_label_font_size);

        svg::Text substrate = basic_text;
        substrate.SetFillColor(settings_.underlayer_color)
                 .SetStrokeColor(settings_.underlayer_color).SetStrokeWidth(settings_.underlayer_width)
                 .SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        svg::Text bus_number = basic_text;
        bus_number.SetFillColor(color);

        map_.Add(substrate);
        map_.Add(bus_number);
    }

    void MapRenderer::AddBusNumberToMap(const transport::Bus& bus) {
        const std::string color = PickColor();

        AddBusNumberAtStop(bus.number, bus.stops.front().lock()->coordinates, color);

        if (!bus.is_circular && bus.stops.front().lock().get() != bus.stops.back().lock().get()) {
            AddBusNumberAtStop(bus.number, bus.stops.back().lock()->coordinates, color);
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

    void MapRenderer::DrawBusLine(const transport::Bus& bus, const std::string& bus_color) {
        const auto& stops = bus.stops;

        svg::Polyline bus_line;
        bus_line.SetFillColor(svg::NoneColor).SetStrokeColor(bus_color).SetStrokeWidth(settings_.line_width)
                  .SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        for (const std::weak_ptr<transport::Stop>& stop : bus.stops) {
            bus_line.AddPoint(projector_(stop.lock()->coordinates));
        }
        if (!bus.is_circular) {
            std::for_each(stops.rbegin() + 1, stops.rend(), [&](const std::weak_ptr<transport::Stop>& stop) {
                bus_line.AddPoint(projector_.operator()(stop.lock()->coordinates));
            });
        }

        map_.Add(bus_line);
    }
}
