#pragma once
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "geo.h"

namespace transport {
    struct Stop;

    struct Route {
        std::string number;
        std::vector<std::weak_ptr<Stop>> stops;
        bool is_circular = false;
    };

    namespace details {
        struct RouteComparator {
            bool operator()(const Route* lhs, const Route* rhs) const;
        };
    }

    struct Stop {
        std::string name;
        geo::Coordinates coordinates;
        std::set<const Route*, details::RouteComparator> passing_routes = {};
    };

    namespace details {
        struct StopPtrHasher {
            size_t operator()(const std::pair<const Stop*, const Stop*>& pair) const;
        };
    }
}
