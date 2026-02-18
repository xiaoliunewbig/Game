# 策略层 CMakeLists.txt
# 文件名: CMakeLists.txt
# 说明: 策略层构建配置文件
# 作者: 彭承康
# 创建时间: 2026-02-18

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
    src/database/SQLiteConnection.cpp
    src/database/BaseRepository.cpp
    src/database/ConnectionPool.cpp
    src/database/MigrationManager.cpp
    src/database/JsonConfigParser.cpp
    src/database/MySQLConnection.cpp
    src/config/ConfigService.cpp
    src/monitor/PerformanceMonitor.cpp
    src/security/SHA256.cpp
    src/security/PasswordHasher.cpp
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

# PostgreSQL - 支持 vcpkg (find_package) 和系统安装 (pkg-config)
find_package(libpqxx CONFIG QUIET)
if(NOT libpqxx_FOUND)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(PQXX QUIET libpqxx)
    endif()
endif()

set(HAS_PQXX_SUPPORT OFF)
if(libpqxx_FOUND OR PQXX_FOUND)
    set(HAS_PQXX_SUPPORT ON)
    message(STATUS "找到 libpqxx — 启用 PostgreSQL 支持")
else()
    message(STATUS "未找到 libpqxx — PostgreSQL 支持已禁用")
endif()

# SQLite3
find_package(SQLite3 QUIET)
set(HAS_SQLITE_SUPPORT OFF)
if(SQLite3_FOUND)
    set(HAS_SQLITE_SUPPORT ON)
    message(STATUS "找到 SQLite3 — 启用 SQLite 支持")
else()
    message(STATUS "未找到 SQLite3 — SQLite 支持已禁用")
endif()

# 至少需要一个数据库后端
if(NOT HAS_PQXX_SUPPORT AND NOT HAS_SQLITE_SUPPORT)
    message(WARNING "未找到任何数据库后端（libpqxx, SQLite3）— strategy_service 将不会构建")
    set(STRATEGY_SERVICE_AVAILABLE OFF PARENT_SCOPE)
    return()
endif()

# 基础链接库
target_link_libraries(strategy_service PUBLIC Threads::Threads)

# 条件链接: PostgreSQL
if(HAS_PQXX_SUPPORT)
    target_compile_definitions(strategy_service PUBLIC HAS_PQXX)
    if(libpqxx_FOUND)
        target_link_libraries(strategy_service PUBLIC libpqxx::pqxx)
    else()
        target_link_libraries(strategy_service PUBLIC ${PQXX_LIBRARIES})
        target_link_directories(strategy_service PUBLIC ${PQXX_LIBRARY_DIRS})
        target_compile_definitions(strategy_service PUBLIC ${PQXX_CFLAGS_OTHER})
    endif()
endif()

# 条件链接: SQLite3
if(HAS_SQLITE_SUPPORT)
    target_compile_definitions(strategy_service PUBLIC HAS_SQLITE)
    target_link_libraries(strategy_service PUBLIC SQLite::SQLite3)
endif()

# 编译选项（跨平台）
if(MSVC)
    target_compile_options(strategy_service PRIVATE /W4 /WX /utf-8 /permissive- /wd4267 /wd4996 /wd4100)
else()
    target_compile_options(strategy_service PRIVATE
        -Wall -Wextra -Wpedantic -Werror -fPIC
        $<$<CONFIG:Debug>:-g -O0 -D_DEBUG_MODE>
        $<$<CONFIG:Release>:-O3 -DNDEBUG>
    )
endif()

# 可选: MySQL 支持
find_package(MySQL QUIET)
if(MySQL_FOUND)
    target_compile_definitions(strategy_service PRIVATE HAS_MYSQL)
    target_link_libraries(strategy_service PRIVATE ${MYSQL_LIBRARIES})
    target_include_directories(strategy_service PRIVATE ${MYSQL_INCLUDE_DIRS})
endif()

# gRPC 服务实现库（需要 proto_generated）
if(TARGET proto_generated)

add_library(strategy_grpc_impl STATIC
    src/StrategyGrpcServiceImpl.cpp
)

target_include_directories(strategy_grpc_impl PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../algorithm/include"
)

target_link_libraries(strategy_grpc_impl PUBLIC
    strategy_service
    proto_generated
)

set_property(TARGET strategy_grpc_impl PROPERTY CXX_STANDARD 20)
set_property(TARGET strategy_grpc_impl PROPERTY CXX_STANDARD_REQUIRED ON)

if(MSVC)
    target_compile_options(strategy_grpc_impl PRIVATE /W4 /utf-8 /wd4267)
else()
    target_compile_options(strategy_grpc_impl PRIVATE -Wall -Wextra -Wpedantic -fPIC)
endif()

# gRPC 服务器可执行文件
add_executable(strategy_server
    src/main_server.cpp
)

target_include_directories(strategy_server PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(strategy_server PRIVATE
    strategy_grpc_impl
)

set_property(TARGET strategy_server PROPERTY CXX_STANDARD 20)
set_property(TARGET strategy_server PROPERTY CXX_STANDARD_REQUIRED ON)

if(MSVC)
    target_compile_options(strategy_server PRIVATE /wd4267)
endif()

endif() # TARGET proto_generated
