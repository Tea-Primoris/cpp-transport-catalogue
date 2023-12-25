#include "json_reader.h"

namespace json {
    Builder::Builder() {
        nodes_stack_.push_back(&root_);
    }

    Builder::KeyContext Builder::Key(std::string key) {
        if ((!nodes_stack_.empty() && !nodes_stack_.back()->IsDict()) || !key_.empty()) {
            throw std::logic_error("Key specified outside dictionary."s);
        }
        key_ = std::move(key);
        key_specified_ = true;
        return KeyContext(*this);
    }

    Builder& Builder::Value(Node::Value value) {
        if (nodes_stack_.empty()) {
            throw std::logic_error("Node is already built."s);
        }

        const auto latest_node = nodes_stack_.back();

        if (latest_node->IsNull()) {
            latest_node->GetValue() = std::move(value);
            nodes_stack_.pop_back();
        } else if (latest_node->IsDict()) {
            if (!key_specified_) {
                throw std::logic_error("Key is not specified"s);
            }
            auto& dict = std::get<Dict>(latest_node->GetValue());
            Node new_node;
            new_node.GetValue() = value;
            dict.emplace(std::move(key_), new_node);
            key_specified_ = false;
        } else if (latest_node->IsArray()) {
            auto& arr = std::get<Array>(latest_node->GetValue());
            Node new_node;
            new_node.GetValue() = value;
            arr.push_back(new_node);
        }

        return *this;
    }

    Builder::DictContext Builder::StartDict() {
        CreateNode(Dict());
        return DictContext(*this);
    }

    Builder::ArrayContext Builder::StartArray() {
        CreateNode(Array());
        return ArrayContext(*this);
    }

    Builder& Builder::EndDict() {
        if (nodes_stack_.empty() || !nodes_stack_.back()->IsDict()) {
            throw std::logic_error("Trying ending dictionary when there's no dictionary to end."s);
        }

        nodes_stack_.pop_back();

        return *this;
    }

    Builder& Builder::EndArray() {
        if (nodes_stack_.empty() || !nodes_stack_.back()->IsArray()) {
            throw std::logic_error("Trying ending array when there's no array to end."s);
        }

        nodes_stack_.pop_back();

        return *this;
    }

    Node Builder::Build() {
        if (!nodes_stack_.empty() || !key_.empty()) {
            throw std::logic_error("Trying to build not finished node."s);
        }

        return root_;
    }

    Reader::Reader(transport::Catalogue& catalogue): catalogue_(catalogue) {
        builder_.StartArray();
    }

    void Reader::Read(std::istream& input_stream) {
        std::map<std::string, Node> requests = json::Load(input_stream).GetRoot().AsDict();
        ProcessBaseRequests(requests.at("base_requests"s));
        ProcessStatRequests(requests.at("stat_requests"s));
        ProcessRenderRequests(requests.at("render_settings"s));
        builder_.EndArray();
    }

    void Reader::BuildJSON(std::ostream& output_stream) const {
        Document output{builder_.Build()};
        Print(output, output_stream);
    }

    void Reader::ProcessRenderRequests(const Node& render_settings) const {
        std::vector<std::string> color_palette(
            ConstructColorPalette(render_settings.AsDict().at("color_palette").AsArray()));
        const double width = render_settings.AsDict().at("width"s).AsDouble();
        const double height = render_settings.AsDict().at("height"s).AsDouble();
        const double padding = render_settings.AsDict().at("padding"s).AsDouble();
        const double line_width = render_settings.AsDict().at("line_width"s).AsDouble();
        maprender::MapRenderer renderer(catalogue_, std::move(color_palette), width, height, padding, line_width);
        renderer.Render(std::cout); //TODO Сделать вывод отдельным.
    }

    std::vector<std::string> Reader::ConstructColorPalette(const Array& nodes_array) {
        std::vector<std::string> color_palette;
        for (const auto& color: nodes_array) {
            if (color.IsArray()) {
                std::string color_string;
                if (color.AsArray().size() == 4) {
                    color_string = "rgba(";
                } else {
                    color_string = "rgb(";
                }
                color_string += std::to_string(color.AsArray().at(0).AsInt()) + ",";
                color_string += std::to_string(color.AsArray().at(1).AsInt()) + ",";
                color_string += std::to_string(color.AsArray().at(2).AsInt());
                if (color.AsArray().size() == 4) {
                    color_string += "," + std::to_string(color.AsArray().at(3).AsDouble());
                }
                color_string += ")";
                color_palette.push_back(std::move(color_string));
            } else {
                color_palette.push_back(color.AsString());
            }
        }
        return color_palette;
    }

    void Reader::ProcessBaseRequests(const Node& base_requests) const {
        std::queue<const Node *> buses_to_process;
        std::queue<std::pair<const std::string *, const Node *>> distances_to_process;
        for (const Node& node: base_requests.AsArray()) {
            if (const Node& type = node.AsDict().at("type"s); type == "Stop"s) {
                AddStop(node, distances_to_process);
            } else if (type == "Bus"s) {
                buses_to_process.push(&node);
            }
        }

        while (!buses_to_process.empty()) {
            AddBus(*buses_to_process.front());
            buses_to_process.pop();
        }
        while (!distances_to_process.empty()) {
            AddDistance(distances_to_process.front());
            distances_to_process.pop();
        }
    }

    void Reader::AddDistance(const std::pair<const std::string*, const Node*>& pair) const {
        const auto& road_distances = pair.second->AsDict();
        const transport::Stop& from_stop = catalogue_.GetStop(*pair.first);
        for (const auto& stop_to_distance: road_distances) {
            const transport::Stop& to_stop = catalogue_.GetStop(stop_to_distance.first);
            const int distance = stop_to_distance.second.AsInt();
            catalogue_.AddDistance(from_stop, to_stop, distance);
        }
    }

    void Reader::AddStop(const Node& node,
        std::queue<std::pair<const std::string*, const Node*>>& distances_to_process) const {
        const auto& stop = node.AsDict();
        const std::string& name = stop.at("name"s).AsString();
        const double latitude = stop.at("latitude"s).AsDouble();
        const double longitude = stop.at("longitude"s).AsDouble();
        catalogue_.AddStop(name, {latitude, longitude});


        const auto& road_distances = stop.at("road_distances"s);
        const std::pair<const std::string *, const Node *> stop_to_distances = {&name, &road_distances};
        distances_to_process.push(stop_to_distances);
    }

    void Reader::AddBus(const Node& node) const {
        const auto& bus = node.AsDict();
        const std::string& bus_number = bus.at("name"s).AsString();
        const bool is_roudtrip = bus.at("is_roundtrip"s).AsBool();
        std::vector<std::string> stops;
        const auto& stops_array = bus.at("stops"s).AsArray();
        for (const auto& stop: stops_array) {
            stops.push_back(stop.AsString());
        }
        catalogue_.AddBus(bus_number, stops, is_roudtrip);
    }

    void Reader::ProcessStatRequests(const Node& stat_requests) const {
        for (const Node& node: stat_requests.AsArray()) {
            if (const Node& type = node.AsDict().at("type"s); type == "Stop"s) {
                GetStopInfo(node);
            } else if (type == "Bus"s) {
                GetBusInfo(node);
            }
        }
    }

    void Reader::GetStopInfo(const Node& node) const {
        const auto& stop_request = node.AsDict();
        builder_.StartDict().Key("request_id"s).Value(stop_request.at("id"s).AsInt());
        const auto& stop_name = stop_request.at("name").AsString();

        if (catalogue_.HasStop(stop_name)) {
            const transport::Stop& stop = catalogue_.GetStop(stop_name);
            builder_.Key("buses"s).StartArray();
            for (const transport::Bus* bus: stop.GetBusses()) {
                builder_.Value(std::string(bus->GetNumber()));
            }
            builder_.EndArray();
        } else {
            builder_.Key("error_message"s).Value("not found"s);
        }

        builder_.EndDict();
    }

    void Reader::GetBusInfo(const Node& node) const {
        const auto& bus_request = node.AsDict();
        builder_.StartDict().Key("request_id"s).Value(bus_request.at("id"s).AsInt());
        const auto& bus_name = bus_request.at("name").AsString();

        if (catalogue_.HasBus(bus_name)) {
            auto bus_info = catalogue_.GetBusInfo(bus_name);

            builder_.Key("curvature"s).Value(bus_info.curvature)
                    .Key("route_length"s).Value(bus_info.route_length)
                    .Key("stop_count"s).Value(static_cast<int>(bus_info.stops_on_route))
                    .Key("unique_stop_count"s).Value(static_cast<int>(bus_info.unique_stops));
        } else {
            builder_.Key("error_message"s).Value("not found"s);
        }

        builder_.EndDict();
    }
}
