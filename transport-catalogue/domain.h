#pragma once
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "geo.h"

namespace transport {
    struct Stop;

    struct Bus {
        std::string number;
        std::vector<std::weak_ptr<Stop>> stops;
        bool is_circular = false;
    };

    namespace details {
        struct BusComparator {
            bool operator()(const Bus* lhs, const Bus* rhs) const;
        };
    }

    struct Stop {
        std::string name;
        geo::Coordinates coordinates;
        std::set<const Bus*, details::BusComparator> passing_busses = {};
    };

    namespace details {
        struct StopPtrHasher {
            size_t operator()(const std::pair<const Stop*, const Stop*>& pair) const;
        };
    }
}
