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
        explicit Reader(transport::Catalogue& catalogue);

        void Read(std::istream& input_stream);

        void BuildJSON(std::ostream& output_stream) const;

    private:
        transport::Catalogue& catalogue_;
        mutable Builder builder_;

        void ProcessRenderRequests(const Node& render_settings) const;

        static std::vector<std::string> ConstructColorPalette(const Array& nodes_array);

        void ProcessBaseRequests(const Node& base_requests) const;

        void AddDistance(const std::pair<const std::string *, const Node *>& pair) const;

        void AddStop(const Node& node,
                     std::queue<std::pair<const std::string *, const Node *>>& distances_to_process) const;

        void AddBus(const Node& node) const;

        void ProcessStatRequests(const Node& stat_requests) const;

        void GetStopInfo(const Node& node) const;

        void GetBusInfo(const Node& node) const;
    };
}
