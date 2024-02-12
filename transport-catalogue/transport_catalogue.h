#pragma once
#include <unordered_map>
#include <vector>
#include <optional>
#include <algorithm>

#include "domain.h"

namespace transport {
    class Catalogue {
    public:
        void SetDistance(std::string_view from_stop, std::string_view to_stop, int distance);

        void AddStop(Stop&& stop);

        bool HasStop(std::string_view stop_name) const;

        const Stop& GetStop(std::string_view stop_name) const;

        void AddBus(Bus&& bus);

        bool HasBus(std::string_view bus_number) const;

        void AddBus(std::string_view bus_number, const std::vector<std::string>& stops, bool is_circular);

        const Bus& GetBus(std::string_view bus_number) const;

        std::optional<int> GetDistanceBetweenStops(const Stop& from_stop, const Stop& to_stop) const;

        const std::vector<std::shared_ptr<Stop>>& GetAllStops() const;

        const std::vector<std::shared_ptr<Bus>>& GetAllBusses() const;

        template <typename IterType>
        std::optional<int> GetDistanceBetweenStopsOnOneRoute(IterType from_stop, IterType to_stop, const Bus& bus) const;

    private:
        std::vector<std::shared_ptr<Stop>> stops_;
        std::unordered_map<std::string_view, std::weak_ptr<Stop>> stopnames_to_stops_;

        std::unordered_map<std::pair<const Stop *, const Stop *>, int, details::StopPtrHasher> distances_;

        std::vector<std::shared_ptr<Bus>> busses_;
        std::unordered_map<std::string_view, std::weak_ptr<Bus>> busnumber_to_bus_;

        std::weak_ptr<Stop> GetStopWeak(std::string_view stop_name) const;

        template <typename IterType>
        std::optional<int> GetDistanceBetweenStopIteratos(IterType begin, IterType end) const;
    };

    template <typename IterType>
    std::optional<int> Catalogue::GetDistanceBetweenStopsOnOneRoute(const IterType from_stop,
                                                                    const IterType to_stop, const Bus& bus) const {
        const ptrdiff_t stops_between_stops = to_stop - from_stop;
        if (std::abs(stops_between_stops) == 1) {
            return GetDistanceBetweenStops(*from_stop->lock(), *to_stop->lock());
        }

        if (stops_between_stops < 0 && !bus.is_circular) {
            const auto range_begin = std::make_reverse_iterator(std::next(from_stop));
            const auto range_end = std::make_reverse_iterator(to_stop);
            return GetDistanceBetweenStopIteratos(range_begin, range_end);
        }
        const auto range_begin = from_stop;
        const auto range_end = std::next(to_stop);
        return GetDistanceBetweenStopIteratos(range_begin, range_end);


        return std::nullopt;
    }

    template <typename IterType>
    std::optional<int> Catalogue::GetDistanceBetweenStopIteratos(IterType begin, IterType end) const {
        int distance = 0;
        for (auto iterator = std::next(begin); iterator != end; ++iterator) {
            const Stop& from_stop = *std::prev(iterator)->lock();
            const Stop& to_stop = *iterator->lock();
            const std::optional<int> distance_between_stops = GetDistanceBetweenStops(from_stop, to_stop);
            if (distance_between_stops.has_value()) {
                distance += distance_between_stops.value();
            }
            else {
                return std::nullopt;
            }
        }
        return distance;
    }
}
