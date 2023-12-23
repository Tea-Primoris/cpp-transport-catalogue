cmake_minimum_required(VERSION 3.25)
project(cpp_transport_catalogue)

set(CMAKE_CXX_STANDARD 23)

add_executable(cpp_transport_catalogue
        main.cpp
        domain.cpp
        geo.cpp
        json.cpp
        json_reader.cpp
        map_renderer.cpp
        request_handler.cpp
        svg.cpp
        transport_catalogue.cpp
        input_reader.cpp
        stat_reader.cpp)

add_subdirectory(tests)