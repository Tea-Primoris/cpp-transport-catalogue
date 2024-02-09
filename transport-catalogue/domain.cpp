#include "domain.h"

namespace transport::details {
    bool BusComparator::operator()(const Bus* lhs, const Bus* rhs) const {
        return lhs->number < rhs->number;
    }

    size_t StopPtrHasher::operator()(const std::pair<const Stop*, const Stop*>& pair) const {
        const size_t first_stop_hash = std::hash<std::string_view>{}(pair.first->name);
        const size_t second_stop_hash = std::hash<std::string_view>{}(pair.second->name);
        return (first_stop_hash * 37) + (second_stop_hash * (37 ^ 2));
    }
}
