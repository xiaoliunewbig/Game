# strategy/tests/CMakeLists.txt

# Build a lightweight test library with only the sources needed for testing
# This avoids requiring libpqxx just to test SHA256, PasswordHasher, etc.
add_library(strategy_test_lib STATIC
    ${CMAKE_SOURCE_DIR}/strategy/src/security/SHA256.cpp
    ${CMAKE_SOURCE_DIR}/strategy/src/security/PasswordHasher.cpp
    ${CMAKE_SOURCE_DIR}/strategy/src/database/JsonConfigParser.cpp
    ${CMAKE_SOURCE_DIR}/strategy/src/database/DatabaseConfig.cpp
)

target_include_directories(strategy_test_lib PUBLIC
    "${CMAKE_SOURCE_DIR}/strategy/include"
)

set_property(TARGET strategy_test_lib PROPERTY CXX_STANDARD 20)
set_property(TARGET strategy_test_lib PROPERTY CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)
target_link_libraries(strategy_test_lib PUBLIC Threads::Threads)

if(MSVC)
    target_compile_options(strategy_test_lib PRIVATE /W4 /utf-8)
    target_compile_definitions(strategy_test_lib PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
    target_compile_options(strategy_test_lib PRIVATE -Wall -Wextra -Wpedantic -fPIC)
endif()

# Test executable
add_executable(strategy_tests
    test_sha256.cpp
    test_password_hasher.cpp
    test_json_config_parser.cpp
    test_database_config.cpp
)

target_include_directories(strategy_tests PRIVATE
    "${CMAKE_SOURCE_DIR}/strategy/include"
)

target_link_libraries(strategy_tests PRIVATE
    strategy_test_lib
    GTest::gtest
    GTest::gtest_main
    Threads::Threads
)

set_property(TARGET strategy_tests PROPERTY CXX_STANDARD 20)
set_property(TARGET strategy_tests PROPERTY CXX_STANDARD_REQUIRED ON)

if(MSVC)
    target_compile_options(strategy_tests PRIVATE /W4 /utf-8)
else()
    target_compile_options(strategy_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(NAME StrategyTests COMMAND strategy_tests)
