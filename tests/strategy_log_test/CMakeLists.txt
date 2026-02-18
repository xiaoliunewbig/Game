#
# CMakeLists.txt for the strategy log system test
#

cmake_minimum_required(VERSION 3.16)

# 定义测试可执行文件
add_executable(run_log_test main.cpp)

# PostgreSQL - 支持 vcpkg (find_package) 和系统安装 (pkg-config)
find_package(libpqxx CONFIG QUIET)
if(NOT libpqxx_FOUND)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(PQXX REQUIRED libpqxx)
        pkg_check_modules(PGSQL REQUIRED libpq)
    endif()
endif()

# 查找线程库
find_package(Threads REQUIRED)

# 链接库
if(libpqxx_FOUND)
    target_link_libraries(run_log_test PRIVATE
        strategy_service
        Threads::Threads
        libpqxx::pqxx
    )
else()
    target_link_libraries(run_log_test PRIVATE
        strategy_service
        Threads::Threads
        ${PQXX_LIBRARIES}
        ${PGSQL_LIBRARIES}
    )
endif()

# 添加头文件目录
target_include_directories(run_log_test PRIVATE
    "${CMAKE_SOURCE_DIR}/strategy/include"
    $<$<NOT:$<BOOL:${libpqxx_FOUND}>>:${PQXX_INCLUDE_DIRS}>
    $<$<NOT:$<BOOL:${libpqxx_FOUND}>>:${PGSQL_INCLUDE_DIRS}>
) 