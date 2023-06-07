cmake_minimum_required(VERSION 3.25)
project(cpp_transport_catalogue)

set(CMAKE_CXX_STANDARD 23)

add_executable(cpp_transport_catalogue
        transport-catalogue/main.cpp
        transport-catalogue/input_reader.cpp
        transport-catalogue/stat_reader.cpp
        transport-catalogue/transport_catalogue.cpp)
