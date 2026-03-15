# tests/CMakeLists.txt

# Google Test via FetchContent.
include(FetchContent)
FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Integration tests that depend on strategy_service and optional DB environment.
if(TARGET strategy_service)
    add_subdirectory(strategy_log_test)
endif()

# Unit tests.
add_subdirectory(algorithm_tests)
add_subdirectory(${CMAKE_SOURCE_DIR}/strategy/tests strategy_tests_build)
