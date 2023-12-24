#include <sstream>
#include <string>
#include <doctest/doctest.h>

#include "../json.h"

using namespace std::literals;

TEST_SUITE("JSON lib") {
    TEST_CASE("JSON reading") {
        SUBCASE("Stop") {
            std::string stop_json = R"(
            {
                "base_requests": [ ... ],
                "stat_requests": [ ... ]
            }
            {
                "type": "Stop",
                "name": "Электросети",
                "latitude": 43.598701,
                "longitude": 39.730623,
                "road_distances": {
                    "Улица Докучаева": 3000,
                    "Улица Лизы Чайкиной": 4300
                }
            }
            )";
            std::istringstream strm(stop_json);
            json::Document test = json::Load(strm);
            CHECK(true);
        }
    }
}
