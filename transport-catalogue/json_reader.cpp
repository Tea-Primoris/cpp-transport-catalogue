#include "json_reader.h"

namespace jsonreader {
    void JSONReader::ReadInput(std::istream& input_stream) {
        const json::Document inputed_json_document(std::move(json::Load(input_stream)));
        const json::Dict& requests = inputed_json_document.GetRoot().AsDict();

        const json::Array& base_requests = requests.at("base_requests"s).AsArray();
        ProcessBaseRequests(base_requests);

        const json::Dict& render_settings = requests.at("render_settings"s).AsDict();
        ProcessRenderSettings(render_settings);

        const json::Array& stat_requests = requests.at("stat_requests"s).AsArray();
        ProcessStatRequests(stat_requests);
    }

    const renderer::MapRenderer& JSONReader::GetMapRenderer() const {
        return *map_renderer_;
    }

    void JSONReader::ProcessBaseRequests(const json::Array& requests_array) const {
        std::queue<std::pair<std::string, const json::Dict*>> distances_to_process;
        std::queue<const json::Dict*> routes_to_process;
        for (const json::Node& node : requests_array) {
            const json::Dict& catalogue_object = node.AsDict();
            if (catalogue_object.at("type"s) == "Stop"s) {
                AddStopToCatalogue(catalogue_object);
                std::pair<std::string, const json::Dict*> stop_to_distances;
                distances_to_process.push(std::move(CreateDistancePair(catalogue_object)));
            }
            else if (catalogue_object.at("type"s) == "Bus"s) {
                routes_to_process.push(&catalogue_object);
            }
        }
        ProcessDistances(distances_to_process);
        ProcessRoutes(routes_to_process);
    }

    void JSONReader::AddStopToCatalogue(const json::Dict& stop_object) const {
        const std::string stop_name = stop_object.at("name"s).AsString();
        const double latitude = stop_object.at("latitude"s).AsDouble();
        const double longitide = stop_object.at("longitude"s).AsDouble();
        catalogue_.AddStop(transport::Stop{stop_name, {latitude, longitide}});
    }

    std::pair<std::string, const json::Dict*> JSONReader::CreateDistancePair(const json::Dict& stop_object) {
        std::pair<std::string, const json::Dict*> stopname_to_distances;
        stopname_to_distances.first = stop_object.at("name"s).AsString();
        stopname_to_distances.second = &stop_object.at("road_distances"s).AsDict();
        return stopname_to_distances;
    }

    void JSONReader::
    ProcessDistances(std::queue<std::pair<std::string, const json::Dict*>>& distances_to_process) const {
        while (!distances_to_process.empty()) {
            const auto& stopname_to_distances = distances_to_process.front();
            const std::string from_stop = stopname_to_distances.first;
            const json::Dict& distances = *stopname_to_distances.second;
            for (const auto& [to_stop, distance] : distances) {
                catalogue_.SetDistance(from_stop, to_stop, distance.AsInt());
            }
            distances_to_process.pop();
        }
    }

    void JSONReader::ProcessRoutes(std::queue<const json::Dict*>& routes_to_process) const {
        while (!routes_to_process.empty()) {
            const json::Dict& route = *routes_to_process.front();
            AddRouteToCatalogue(route);
            routes_to_process.pop();
        }
    }

    std::vector<std::string> JSONReader::CreateStopsVector(const json::Array& stops) {
        std::vector<std::string> stops_as_strings;
        for (const json::Node& stop_node : stops) {
            stops_as_strings.push_back(stop_node.AsString());
        }
        return stops_as_strings;
    }

    void JSONReader::AddRouteToCatalogue(const json::Dict& stop_object) const {
        const std::string route_number = stop_object.at("name"s).AsString();
        const std::vector stops(CreateStopsVector(stop_object.at("stops").AsArray()));
        const bool is_circular = stop_object.at("is_roundtrip"s).AsBool();
        catalogue_.AddRoute(route_number, stops, is_circular);
    }

    void JSONReader::ProcessStatRequests(const json::Array& requests_array) const {
        for (const json::Node& request_object : requests_array) {
            if (request_object.AsDict().at("type"s) == "Stop"s) {
                const std::string_view stop_name(request_object.AsDict().at("name"s).AsString());
                const int request_id = request_object.AsDict().at("id"s).AsInt();
                request_handler_.PrepareStop(request_id, stop_name);
            }
            else if (request_object.AsDict().at("type"s) == "Bus"s) {
                const std::string_view route_number(request_object.AsDict().at("name"s).AsString());
                const int request_id = request_object.AsDict().at("id"s).AsInt();
                request_handler_.PrepareRoute(request_id, route_number);
            }
            else if (request_object.AsDict().at("type"s) == "Map"s) {
                const int request_id = request_object.AsDict().at("id"s).AsInt();
                std::ostringstream output_stream;
                map_renderer_->Render(output_stream);
                request_handler_.PrepareMap(request_id, output_stream.str());
            }
        }
    }

    renderer::SphereProjector JSONReader::GenerateSphereProjector(double width, double height, double padding) const {
        std::vector<geo::Coordinates> coords;
        for (const std::weak_ptr<transport::Stop> stop : catalogue_.GetAllStops()) {
            if (!stop.lock()->passing_routes.empty()) {
                coords.push_back(stop.lock()->coordinates);
            }
        }
        return {coords.begin(), coords.end(), width, height, padding};
    }

    std::vector<std::string> JSONReader::GenerateColorPalette(const json::Array& color_array) {
        std::vector<std::string> color_palette;
        color_palette.reserve(color_array.size());
        for (const json::Node& color : color_array) {
            if (color.IsString()) {
                color_palette.push_back(color.AsString());
            }
            else if (color.IsArray()) {
                color_palette.push_back(std::move(GenerateColorStringFromArray(color.AsArray())));
            }
        }
        return color_palette;
    }

    svg::Point JSONReader::GenerateOffsetPoint(const json::Array& array) {
        return {array.at(0).AsDouble(), array.at(1).AsDouble()};
    }

    std::string JSONReader::GenerateColorStringFromArray(const json::Array& array) {
        std::stringstream color_stream;

        if (array.size() == 3) { color_stream << "rgb("s; }
        else if (array.size() == 4) { color_stream << "rgba("s; }

        color_stream << std::to_string(array.at(0).AsInt()) << ",";
        color_stream << std::to_string(array.at(1).AsInt()) << ",";
        color_stream << std::to_string(array.at(2).AsInt());

        if (array.size() == 4) {
            color_stream << std::setprecision(6) << std::noshowpoint << "," << array.at(3).AsDouble();
        }

        color_stream << ")"s;
        return color_stream.str();
    }

    void JSONReader::ConstructMapRenderer(const json::Dict& requests_array) {
        renderer::RenderSettings render_settings;
        render_settings.width = requests_array.at("width"s).AsDouble();
        render_settings.height = requests_array.at("height"s).AsDouble();
        render_settings.padding = requests_array.at("padding"s).AsDouble();
        render_settings.stop_radius = requests_array.at("stop_radius"s).AsDouble();
        render_settings.line_width = requests_array.at("line_width"s).AsDouble();
        render_settings.bus_label_font_size = requests_array.at("bus_label_font_size"s).AsDouble();
        render_settings.bus_label_offset = GenerateOffsetPoint(requests_array.at("bus_label_offset"s).AsArray());
        render_settings.stop_label_font_size = requests_array.at("stop_label_font_size"s).AsDouble();
        render_settings.stop_label_offset = GenerateOffsetPoint(requests_array.at("stop_label_offset"s).AsArray());
        render_settings.underlayer_color =
            requests_array.at("underlayer_color"s).IsString()
                ? requests_array.at("underlayer_color"s).AsString()
                : GenerateColorStringFromArray(requests_array.at("underlayer_color"s).AsArray());
        render_settings.underlayer_width = requests_array.at("underlayer_width"s).AsDouble();
        render_settings.color_palette = GenerateColorPalette(requests_array.at("color_palette"s).AsArray());

        renderer::SphereProjector projector(
            GenerateSphereProjector(render_settings.width, render_settings.height, render_settings.padding));

        map_renderer_ = std::make_shared<renderer::MapRenderer>(std::move(render_settings), projector);
    }

    std::vector<std::shared_ptr<transport::Route>> JSONReader::GetSortedRoutes() const {
        std::vector<std::shared_ptr<transport::Route>> sorted_routes = catalogue_.GetAllRoutes();
        std::sort(sorted_routes.begin(), sorted_routes.end(),
                  [](const std::weak_ptr<transport::Route>& lhs, const std::weak_ptr<transport::Route>& rhs) {
                      return lhs.lock()->number < rhs.lock()->number;
                  });
        return sorted_routes;
    }

    std::vector<std::weak_ptr<transport::Stop>> JSONReader::GetSortedStops() const {
        std::vector<std::weak_ptr<transport::Stop>> sorted_stops;
        for (std::weak_ptr<transport::Stop> stop : catalogue_.GetAllStops()) {
            if (!stop.lock()->passing_routes.empty()) {
                sorted_stops.push_back(stop);
            }
        }
        std::sort(sorted_stops.begin(), sorted_stops.end(),
                  [](const std::weak_ptr<transport::Stop>& lhs, const std::weak_ptr<transport::Stop>& rhs) {
                      return lhs.lock()->name < rhs.lock()->name;
                  });
        return sorted_stops;
    }

    void JSONReader::ProcessRenderSettings(const json::Dict& requests_array) {
        ConstructMapRenderer(requests_array);

        const std::vector sorted_routes = std::move(GetSortedRoutes());

        for (std::weak_ptr<transport::Route> route : sorted_routes) {
            map_renderer_->AddRouteToMap(*route.lock());
        }
        map_renderer_->SetCurrentColor(0);

        for (std::weak_ptr<transport::Route> route : sorted_routes) {
            map_renderer_->AddRouteNumberToMap(*route.lock());
        }
        map_renderer_->SetCurrentColor(0);

        const std::vector sorted_stops = std::move(GetSortedStops());
        for (std::weak_ptr<transport::Stop> stop : sorted_stops) {
            map_renderer_->DrawStopCircle(*stop.lock());
        }

        for (std::weak_ptr<transport::Stop> stop : sorted_stops) {
            map_renderer_->DrawStopName(*stop.lock());
        }
    }
}
