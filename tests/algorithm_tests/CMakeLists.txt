# tests/algorithm_tests/CMakeLists.txt

add_executable(algorithm_tests
    test_character_stats.cpp
    test_status_effect.cpp
    test_cooldown_tracker.cpp
    test_input_validator.cpp
    test_damage_calculator.cpp
    test_algorithm_service.cpp
)

target_link_libraries(algorithm_tests PRIVATE
    algorithm_service
    GTest::gtest
    GTest::gtest_main
)

set_property(TARGET algorithm_tests PROPERTY CXX_STANDARD 20)
set_property(TARGET algorithm_tests PROPERTY CXX_STANDARD_REQUIRED ON)

if(MSVC)
    target_compile_options(algorithm_tests PRIVATE /W4 /utf-8)
else()
    target_compile_options(algorithm_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(NAME AlgorithmTests COMMAND algorithm_tests)
