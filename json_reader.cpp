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
}
