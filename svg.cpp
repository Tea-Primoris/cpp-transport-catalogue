#include "svg.h"

namespace svg {

    using namespace std::literals;

    void Object::Render(const RenderContext &context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

// ---------- Circle ------------------

    Circle &Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle &Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext &context) const {
        auto &out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        // Выводим атрибуты, унаследованные от PathProps
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    Polyline &Polyline::AddPoint(Point point) {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext &context) const {
        auto &out = context.out;
        out << "<polyline points=\"";

        if (!points_.empty()) {
            out << points_.front().x << ',' << points_.front().y;
            for (auto iterator = points_.begin() + 1; iterator != points_.end(); iterator = std::next(iterator)) {
                out << ' ' << iterator->x << ',' << iterator->y;
            }
        }
        out << "\"";

        // Выводим атрибуты, унаследованные от PathProps
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    Text &Text::SetPosition(Point pos) {
        position_ = pos;
        return *this;
    }

    Text &Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text &Text::SetFontSize(uint32_t size) {
        font_size_ = size;
        return *this;
    }

    Text &Text::SetFontFamily(std::string font_family) {
        font_family_ = std::move(font_family);
        return *this;
    }

    Text &Text::SetFontWeight(std::string font_weight) {
        font_weight_ = std::move(font_weight);
        return *this;
    }

    Text &Text::SetData(std::string data) {
        for (const auto &item: replace_) {
            data = std::regex_replace(data, std::regex(item.first), item.second);
        }
        data_ = std::move(data);
        return *this;
    }

    void Text::RenderObject(const RenderContext &context) const {
        auto &out = context.out;
        out << R"(<text x=")" << position_.x << R"(" y=")" << position_.y << "\"";
        out << R"( dx=")" << offset_.x << R"(" dy=")" << offset_.y << "\"";
        out << R"( font-size=")" << font_size_ << "\"";
        if (!font_family_.empty()) {
            out << R"( font-family=")" << font_family_ << "\"";
        }
        if (!font_weight_.empty()) {
            out << R"( font-weight=")" << font_weight_ << "\"";
        }
        // Выводим атрибуты, унаследованные от PathProps
        RenderAttrs(context.out);
        out << ">" << data_ << "</text>"sv;
    }

    void Document::AddPtr(std::unique_ptr<Object> &&obj) {
        objects_.push_back(std::move(obj));
    }

    void Document::Render(std::ostream &out) const {
        out << R"(<?xml version="1.0" encoding="UTF-8" ?>)" << std::endl;
        out << R"(<svg xmlns="http://www.w3.org/2000/svg" version="1.1">)" << std::endl;
        RenderContext context(out, 2, 2);
        for (const auto &object: objects_) {
            object->Render(context);
        }
        std::cout << "</svg>"sv;
    }
}  // namespace svg