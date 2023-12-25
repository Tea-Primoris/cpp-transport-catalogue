#pragma once
#include <algorithm>
#include <ostream>
#include <utility>

#include "svg.h"
#include "transport_catalogue.h"

using namespace std::literals;

namespace maprender {
    namespace details {
        inline const double EPSILON = 1e-6;

        inline bool IsZero(double value) {
            return std::abs(value) < EPSILON;
        }

        class SphereProjector {
        public:
            // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
            template<typename PointInputIt>
            SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                            double max_width, double max_height, double padding)
                : padding_(padding) //
            {
                // Если точки поверхности сферы не заданы, вычислять нечего
                if (points_begin == points_end) {
                    return;
                }

                // Находим точки с минимальной и максимальной долготой
                const auto [left_it, right_it] = std::minmax_element(
                    points_begin, points_end,
                    [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
                min_lon_ = left_it->lng;
                const double max_lon = right_it->lng;

                // Находим точки с минимальной и максимальной широтой
                const auto [bottom_it, top_it] = std::minmax_element(
                    points_begin, points_end,
                    [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
                const double min_lat = bottom_it->lat;
                max_lat_ = top_it->lat;

                // Вычисляем коэффициент масштабирования вдоль координаты x
                std::optional<double> width_zoom;
                if (!IsZero(max_lon - min_lon_)) {
                    width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
                }

                // Вычисляем коэффициент масштабирования вдоль координаты y
                std::optional<double> height_zoom;
                if (!IsZero(max_lat_ - min_lat)) {
                    height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
                }

                if (width_zoom && height_zoom) {
                    // Коэффициенты масштабирования по ширине и высоте ненулевые,
                    // берём минимальный из них
                    zoom_coeff_ = std::min(*width_zoom, *height_zoom);
                } else if (width_zoom) {
                    // Коэффициент масштабирования по ширине ненулевой, используем его
                    zoom_coeff_ = *width_zoom;
                } else if (height_zoom) {
                    // Коэффициент масштабирования по высоте ненулевой, используем его
                    zoom_coeff_ = *height_zoom;
                }
            }

            // Проецирует широту и долготу в координаты внутри SVG-изображения
            svg::Point operator()(geo::Coordinates coords) const {
                return {
                    (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                    (max_lat_ - coords.lat) * zoom_coeff_ + padding_
                };
            }

        private:
            double padding_;
            double min_lon_ = 0;
            double max_lat_ = 0;
            double zoom_coeff_ = 0;
        };
    }

    class MapRenderer {
    public:
        explicit MapRenderer(transport::Catalogue& catalogue, std::vector<std::string> color_palette, double width,
                             double height, double padding, double line_width)
            : catalogue_(catalogue), color_palette_(std::move(color_palette)), line_width_(line_width) {
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

        void Render(std::ostream& output_stream) {
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

    private:
        svg::Document map_;
        transport::Catalogue& catalogue_;
        std::vector<std::string> color_palette_;
        double line_width_;

        size_t current_color_ = 0;
        std::unique_ptr<details::SphereProjector> projector_;

        void RenderRoute(const transport::Bus& route) {
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

        std::string PickStrokeColor() {
            std::string color = color_palette_.at(current_color_);
            current_color_ = (current_color_ + 1) < color_palette_.size() ? current_color_ + 1 : 0;
            return color;
        }
    };
}
