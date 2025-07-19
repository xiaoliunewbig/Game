# 策略层 CMakeLists.txt
# 文件名: CMakeLists.txt
# 说明: 策略层构建配置文件
# 作者: 彭承康
# 创建时间: 2025-07-20

cmake_minimum_required(VERSION 3.16)

project(StrategyService CXX)

# 添加策略服务静态库
add_library(strategy_service STATIC
    src/Algorithm_interact/StrategyService.cpp
    src/Algorithm_interact/GameRuleManager.cpp
    src/Algorithm_interact/WorldStateEngine.cpp
    src/Algorithm_interact/EventScheduler.cpp
    src/Algorithm_interact/AlgorithmInterface.cpp
    src/Log/AsyncLogService.cpp
    src/Log/LogServiceConsole.cpp
    src/Log/LogServiceFile.cpp
    src/player_service/PlayerRepositoryPostgres.cpp
    src/player_service/PlayerRepositoryDatabase.cpp
    src/player_service/PlayerService.cpp
    src/player_service/PlayerServiceFactory.cpp
    src/database/DatabaseConfig.cpp
    src/database/DatabaseConfigManager.cpp
    src/database/DatabaseFactory.cpp
    src/database/PostgreSQLConnection.cpp
    src/database/BaseRepository.cpp
    src/config/ConfigService.cpp
    src/monitor/PerformanceMonitor.cpp
)

# 头文件目录
target_include_directories(strategy_service PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/../algorithm/include"
)

# C++20 标准
set_property(TARGET strategy_service PROPERTY CXX_STANDARD 20)
set_property(TARGET strategy_service PROPERTY CXX_STANDARD_REQUIRED ON)

# 查找依赖库
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)

# 查找 PostgreSQL
pkg_check_modules(PQXX REQUIRED libpqxx)

# 链接库
target_link_libraries(strategy_service PUBLIC 
    Threads::Threads
    ${PQXX_LIBRARIES}
)

# 编译选项
target_compile_options(strategy_service PRIVATE
    -Wall -Wextra -Wpedantic
    $<$<CONFIG:Debug>:-g -O0 -DDEBUG>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

# 链接目录
target_link_directories(strategy_service PUBLIC ${PQXX_LIBRARY_DIRS})

# 编译定义
target_compile_definitions(strategy_service PUBLIC ${PQXX_CFLAGS_OTHER})
