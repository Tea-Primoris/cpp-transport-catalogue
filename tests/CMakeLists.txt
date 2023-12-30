include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(google_tests
        sample_test.cpp
        io_tests.cpp

        ../transport-catalogue/domain.cpp
        ../transport-catalogue/geo.cpp
        ../transport-catalogue/svg.cpp
        ../transport-catalogue/transport_catalogue.cpp
        ../transport-catalogue/json_reader.cpp
        ../transport-catalogue/json.cpp
        ../transport-catalogue/map_renderer.cpp
        ../transport-catalogue/request_handler.cpp)

target_link_libraries(google_tests GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(google_tests)
