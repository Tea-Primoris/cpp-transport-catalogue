cmake_minimum_required(VERSION 3.27)
project(transport_catalogue)

set(CMAKE_CXX_STANDARD 20)

add_executable(transport_catalogue transport-catalogue/main.cpp
        transport-catalogue/domain.cpp
        transport-catalogue/geo.cpp
        transport-catalogue/svg.cpp
        transport-catalogue/transport_catalogue.cpp
        transport-catalogue/json_reader.cpp
        transport-catalogue/json.cpp
        transport-catalogue/map_renderer.cpp
        transport-catalogue/request_handler.cpp
        transport-catalogue/json_builder.cpp
        transport-catalogue/transport_router.cpp)
add_subdirectory(tests)