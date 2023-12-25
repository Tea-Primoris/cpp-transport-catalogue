#pragma once
#include <functional>
#include <istream>
#include <memory>
#include <queue>

#include "json.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "svg.h"

using namespace std::literals;

//JSON Builder start
namespace json {
    using namespace std::literals;


    class Builder {
        class BaseContext;
        class KeyContext;
        class ArrayContext;
        class DictContext;

    public:
        Builder();

        Builder(const Builder&) = delete;

        Builder(Builder&& other) = delete;

        Builder& operator=(const Builder&) = delete;

        Builder& operator=(Builder&& other) = delete;

        ~Builder() = default;

        KeyContext Key(std::string key);

        Builder& Value(Node::Value value);

        DictContext StartDict();

        ArrayContext StartArray();

        Builder& EndDict();

        Builder& EndArray();

        Node Build();

    private:
        std::string key_;
        bool key_specified_ = false;
        Node root_;
        std::vector<Node *> nodes_stack_;

        template<typename T>
        void CreateNode(T value) {
            if (nodes_stack_.empty()) {
                throw std::logic_error("Node is already built."s);
            }

            if (const auto& back_node = nodes_stack_.back(); back_node->IsNull()) {
                back_node->GetValue() = value;
            } else if (back_node->IsArray()) {
                auto& new_array = std::get<Array>(back_node->GetValue()).emplace_back(value);
                nodes_stack_.push_back(&new_array);
            } else if (back_node->IsDict()) {
                if (!key_specified_) {
                    throw std::logic_error("Key not specified."s);
                }
                auto new_dictionary = std::get<Dict>(back_node->GetValue()).emplace(std::move(key_), value);
                key_specified_ = false;
                nodes_stack_.push_back(&new_dictionary.first->second);
            }
        }

        // Вспомогательные классы

        class BaseContext {
        public:
            explicit BaseContext(Builder& builder)
                : builder_(builder) {
            }

            ~BaseContext() = default;

            auto Build() {
                return builder_.Build();
            }

            KeyContext Key(std::string key) {
                return builder_.Key(std::move(key));
            }

            Builder& Value(Node::Value value) {
                return builder_.Value(std::move(value));
            }

            DictContext StartDict() {
                return builder_.StartDict();
            }

            Builder& EndDict() {
                return builder_.EndDict();
            }

            ArrayContext StartArray() {
                return builder_.StartArray();
            }

            Builder& EndArray() {
                return builder_.EndArray();
            }

        protected:
            Builder& builder_;
        };

        class DictContext : public BaseContext {
        public:
            explicit DictContext(Builder& builder)
                : BaseContext(builder) {
            }

            auto Build() = delete;

            Builder& Value(Node::Value value) = delete;

            DictContext StartDict() = delete;

            ArrayContext StartArray() = delete;

            Builder& EndArray() = delete;
        };

        class ArrayContext : public BaseContext {
        public:
            explicit ArrayContext(Builder& builder)
                : BaseContext(builder) {
            }

            auto Build() = delete;

            KeyContext Key(std::string key) = delete;

            ArrayContext Value(Node::Value value) {
                return ArrayContext(builder_.Value(std::move(value)));
            }

            Builder& EndDict() = delete;
        };

        class KeyContext : public BaseContext {
        public:
            explicit KeyContext(Builder& builder)
                : BaseContext(builder) {
            }

            auto Build() = delete;

            KeyContext Key(std::string key) = delete;

            DictContext Value(Node::Value value) {
                return DictContext(builder_.Value(std::move(value)));
            }

            Builder& EndDict() = delete;

            Builder& EndArray() = delete;
        };
    };
}

//JSON builder end

namespace json {
    class Reader {
    public:
        explicit Reader(transport::Catalogue& catalogue) : catalogue_(catalogue) {
            builder_.StartArray();
        }

        void Read(std::istream& input_stream) {
            std::map<std::string, Node> requests = json::Load(input_stream).GetRoot().AsDict();
            ProcessBaseRequests(requests.at("base_requests"s));
            ProcessStatRequests(requests.at("stat_requests"s));
            ProcessRenderRequests(requests.at("render_settings"s));
            builder_.EndArray();
        }

        void BuildJSON(std::ostream& output_stream) {
            Document output{builder_.Build()};
            Print(output, output_stream);
        }

    private:
        transport::Catalogue& catalogue_;
        mutable Builder builder_;

        void ProcessRenderRequests(const Node& render_settings) {
            std::vector<std::string> color_palette(
                ConstructColorPalette(render_settings.AsDict().at("color_palette").AsArray()));
            const double width = render_settings.AsDict().at("width"s).AsDouble();
            const double height = render_settings.AsDict().at("height"s).AsDouble();
            const double padding = render_settings.AsDict().at("padding"s).AsDouble();
            const double line_width = render_settings.AsDict().at("line_width"s).AsDouble();
            maprender::MapRenderer renderer(catalogue_, std::move(color_palette), width, height, padding, line_width);
            renderer.Render(std::cout); //TODO Сделать вывод отдельным.
        }

        static std::vector<std::string> ConstructColorPalette(const Array& nodes_array) {
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


        void ProcessBaseRequests(const Node& base_requests) const {
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

        void AddDistance(const std::pair<const std::string *, const Node *>& pair) const {
            const auto& road_distances = pair.second->AsDict();
            const transport::Stop& from_stop = catalogue_.GetStop(*pair.first);
            for (const auto& stop_to_distance: road_distances) {
                const transport::Stop& to_stop = catalogue_.GetStop(stop_to_distance.first);
                const int distance = stop_to_distance.second.AsInt();
                catalogue_.AddDistance(from_stop, to_stop, distance);
            }
        }

        void AddStop(const Node& node,
                     std::queue<std::pair<const std::string *, const Node *>>& distances_to_process) const {
            const auto& stop = node.AsDict();
            const std::string& name = stop.at("name"s).AsString();
            const double latitude = stop.at("latitude"s).AsDouble();
            const double longitude = stop.at("longitude"s).AsDouble();
            catalogue_.AddStop(name, {latitude, longitude});


            const auto& road_distances = stop.at("road_distances"s);
            const std::pair<const std::string *, const Node *> stop_to_distances = {&name, &road_distances};
            distances_to_process.push(stop_to_distances);
        }

        void AddBus(const Node& node) const {
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

        void ProcessStatRequests(const Node& stat_requests) const {
            for (const Node& node: stat_requests.AsArray()) {
                if (const Node& type = node.AsDict().at("type"s); type == "Stop"s) {
                    GetStopInfo(node);
                } else if (type == "Bus"s) {
                    GetBusInfo(node);
                }
            }
        }

        void GetStopInfo(const Node& node) const {
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

        void GetBusInfo(const Node& node) const {
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
    };
}
