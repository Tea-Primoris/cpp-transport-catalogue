find_package(doctest REQUIRED)
add_executable(tests sample_test.cpp)
target_compile_features(tests PRIVATE cxx_std_17)
target_link_libraries(tests PRIVATE doctest::doctest)