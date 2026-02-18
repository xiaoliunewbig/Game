#
# tests/CMakeLists.txt
#

# Google Test via FetchContent
include(FetchContent)
FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# 现有集成测试（需要数据库和 strategy_service）
if(TARGET strategy_service)
    add_subdirectory(strategy_log_test)
endif()

# 单元测试
add_subdirectory(algorithm_tests)
add_subdirectory(${CMAKE_SOURCE_DIR}/strategy/tests strategy_tests_build)
