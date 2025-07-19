#
# CMakeLists.txt for the strategy log system test
#

cmake_minimum_required(VERSION 3.16)

# 定义测试可执行文件
add_executable(run_log_test main.cpp)

# 再次查找 PostgreSQL 库
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)
pkg_check_modules(PGSQL REQUIRED libpq)

# 查找线程库
find_package(Threads REQUIRED)

# 链接我们之前创建的 strategy_log 库，以及它所需要的所有底层库
target_link_libraries(run_log_test PRIVATE 
    strategy_log
    Threads::Threads
    ${PQXX_LIBRARIES}
    ${PGSQL_LIBRARIES}
)

# 添加头文件目录
target_include_directories(run_log_test PRIVATE
    "${CMAKE_SOURCE_DIR}/strategy/include"
    ${PQXX_INCLUDE_DIRS}
    ${PGSQL_INCLUDE_DIRS}
) 