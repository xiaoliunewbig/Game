# ============================================================================
# 《幻境传说》游戏框架 - 主构建配置文件
# ============================================================================
# 文件名: CMakeLists.txt
# 说明: 项目根目录的主CMake配置文件，定义整个项目的构建规则
# 作者: 彭承康
# 创建时间: 2026-02-18
# 版本: v1.0.0
#
# 功能描述:
# - 设置项目基本信息和编译标准
# - 配置全局编译选项和依赖项
# - 管理子项目的构建顺序
# - 定义安装规则和打包配置
# - 提供一键构建和测试目标
#
# 项目结构:
# - algorithm/    : 算法层服务 (C++ gRPC服务)
# - strategy/     : 策略层服务 (C++ gRPC服务)  
# - application/  : Qt应用层 (C++ Qt6应用)
# - tests/        : 集成测试套件
# - tools/        : Python开发工具
#
# 构建目标:
# - algorithm_service : 算法层可执行文件
# - strategy_service  : 策略层可执行文件
# - game_client      : Qt游戏客户端
# - build_all        : 一键构建所有目标
# ============================================================================

# ----------------------------------------------------------------------------
# 项目基本配置
# ----------------------------------------------------------------------------

# 设置CMake最低版本要求
# 3.16+ 支持Qt6和现代CMake特性
cmake_minimum_required(VERSION 3.16)

# 定义项目信息
project(Game 
    VERSION 1.0.0                    # 项目版本号
    DESCRIPTION "幻境传说游戏框架"     # 项目描述
    LANGUAGES CXX                     # 使用的编程语言
)

# ----------------------------------------------------------------------------
# 编译器和语言标准配置
# ----------------------------------------------------------------------------

# 设置C++标准为C++20
# 使用现代C++特性：概念、协程、模块等
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)        # 禁用编译器扩展，确保可移植性

# 设置构建类型（如果未指定）
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "构建类型" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# 输出构建信息
message(STATUS "=== 《幻境传说》游戏框架构建配置 ===")
message(STATUS "项目版本: ${PROJECT_VERSION}")
message(STATUS "构建类型: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++标准: C++${CMAKE_CXX_STANDARD}")
message(STATUS "编译器: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")

# ----------------------------------------------------------------------------
# 全局编译选项
# ----------------------------------------------------------------------------

# 设置全局编译选项
if(MSVC)
    # Microsoft Visual C++ 编译器选项
    add_compile_options(
        /W4                          # 启用高级警告
        /WX                          # 将警告视为错误
        /utf-8                       # 使用UTF-8编码
        /permissive-                 # 禁用非标准扩展
    )
    # Debug模式特定选项
    add_compile_options(
        "$<$<CONFIG:Debug>:/Od>"
        "$<$<CONFIG:Debug>:/Zi>"
        "$<$<CONFIG:Debug>:/RTC1>"
    )
    # Release模式特定选项
    add_compile_options(
        "$<$<CONFIG:Release>:/O2>"
        "$<$<CONFIG:Release>:/DNDEBUG>"
    )
else()
    # GCC/Clang 编译器选项
    add_compile_options(
        -Wall                        # 启用常见警告
        -Wextra                      # 启用额外警告
        -Wpedantic                   # 启用严格标准检查
        -Werror                      # 将警告视为错误
        -fPIC                        # 生成位置无关代码
    )
    # Debug模式特定选项
    add_compile_options(
        "$<$<CONFIG:Debug>:-g>"
        "$<$<CONFIG:Debug>:-O0>"
        "$<$<CONFIG:Debug>:-D_DEBUG_MODE>"
    )
    # Release模式特定选项
    add_compile_options(
        "$<$<CONFIG:Release>:-O3>"
        "$<$<CONFIG:Release>:-DNDEBUG>"
    )
endif()

# ----------------------------------------------------------------------------
# 全局依赖项查找
# ----------------------------------------------------------------------------

# 查找必需的依赖包
message(STATUS "查找项目依赖...")

# ---------- 自动查找 Qt6 ----------
if(WIN32 AND NOT Qt6_DIR)
    set(QT6_POSSIBLE_PATHS
        "C:/Qt/6.8.0/msvc2022_64"
        "C:/Qt/6.7.0/msvc2022_64"
        "C:/Qt/6.6.0/msvc2022_64"
        "C:/Qt/6.5.3/msvc2019_64"
        "C:/Qt/6.5.0/msvc2022_64"
    )
    foreach(QT_PATH ${QT6_POSSIBLE_PATHS})
        if(EXISTS "${QT_PATH}/lib/cmake/Qt6")
            list(PREPEND CMAKE_PREFIX_PATH "${QT_PATH}")
            message(STATUS "自动检测到 Qt6: ${QT_PATH}")
            break()
        endif()
    endforeach()
endif()

# Qt6 - 用于应用层UI（可选，不影响算法/策略层和测试）
find_package(Qt6 QUIET COMPONENTS
    Core                            # Qt核心模块
    Quick                           # Qt Quick (QML)
    Widgets                         # Qt Widgets
    Multimedia                      # 多媒体支持
    Network                         # 网络支持
)
if(Qt6_FOUND)
    message(STATUS "Qt6版本: ${Qt6_VERSION}")
else()
    message(WARNING "未找到 Qt6 — game_client 将不会构建")
endif()

# gRPC 和 Protobuf - 支持 vcpkg (find_package) 和系统安装 (pkg-config)
find_package(Protobuf CONFIG QUIET)
find_package(gRPC CONFIG QUIET)
if(NOT gRPC_FOUND OR NOT Protobuf_FOUND)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        if(NOT gRPC_FOUND)
            pkg_check_modules(GRPC QUIET grpc++)
        endif()
        if(NOT Protobuf_FOUND)
            pkg_check_modules(PROTOBUF QUIET protobuf)
        endif()
    endif()
endif()

set(HAS_GRPC OFF)
if(gRPC_FOUND OR GRPC_FOUND)
    set(HAS_GRPC ON)
endif()
if(NOT HAS_GRPC)
    message(WARNING "未找到 gRPC/Protobuf — gRPC服务将不会构建")
endif()

# 线程支持
find_package(Threads REQUIRED)

# ---------- 自动查找 SQLite3 ----------
if(WIN32 AND NOT SQLite3_FOUND)
    # 尝试从 Miniconda/Anaconda 路径查找 SQLite3
    set(SQLITE3_POSSIBLE_PREFIXES
        "$ENV{CONDA_PREFIX}/Library"
        "D:/pip/miniconda/Library"
        "C:/Miniconda3/Library"
        "C:/Anaconda3/Library"
    )
    foreach(SQLITE_PREFIX ${SQLITE3_POSSIBLE_PREFIXES})
        if(EXISTS "${SQLITE_PREFIX}/include/sqlite3.h" AND EXISTS "${SQLITE_PREFIX}/lib/sqlite3.lib")
            list(PREPEND CMAKE_PREFIX_PATH "${SQLITE_PREFIX}")
            message(STATUS "自动检测到 SQLite3: ${SQLITE_PREFIX}")
            break()
        endif()
    endforeach()
endif()

# 输出依赖信息
if(gRPC_FOUND)
    message(STATUS "gRPC: 通过 find_package 找到")
elseif(GRPC_FOUND)
    message(STATUS "gRPC版本: ${GRPC_VERSION} (pkg-config)")
endif()

# ----------------------------------------------------------------------------
# 全局包含目录和链接目录
# ----------------------------------------------------------------------------

# 设置全局包含目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}      # 项目根目录
    ${CMAKE_CURRENT_BINARY_DIR}      # 构建目录
)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)     # 可执行文件
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)     # 动态库
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)     # 静态库

# ----------------------------------------------------------------------------
# 启用测试框架
# ----------------------------------------------------------------------------

# 启用CTest测试框架
enable_testing()

# 设置测试输出目录
set(CMAKE_TEST_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests)

message(STATUS "测试框架已启用")

# ----------------------------------------------------------------------------
# 添加子项目
# ----------------------------------------------------------------------------

message(STATUS "配置子项目...")

# 添加 Proto 生成库（必须在其他子项目之前，需要gRPC）
if(HAS_GRPC)
    add_subdirectory(proto)
    message(STATUS "  ✓ Proto 生成库 (proto)")
endif()

# 添加算法层服务
# 提供战斗计算、AI决策等核心算法
add_subdirectory(algorithm)
message(STATUS "  ✓ 算法层服务 (algorithm)")

# 添加策略层服务
# 提供游戏规则、世界状态管理
add_subdirectory(strategy)
message(STATUS "  ✓ 策略层服务 (strategy)")

# 添加Qt应用层（需要Qt6）
if(Qt6_FOUND)
    add_subdirectory(application)
    message(STATUS "  ✓ Qt应用层 (application)")
else()
    message(STATUS "  ✗ Qt应用层 (application) — 跳过 (缺少Qt6)")
endif()

# 添加测试套件
# 集成测试和单元测试
add_subdirectory(tests)
message(STATUS "  ✓ 测试套件 (tests)")

# ----------------------------------------------------------------------------
# 自定义构建目标
# ----------------------------------------------------------------------------

# 创建一键构建目标
# 按正确顺序构建所有核心组件
set(BUILD_ALL_DEPS algorithm_service strategy_service)
if(HAS_GRPC)
    list(APPEND BUILD_ALL_DEPS algorithm_server strategy_server)
endif()
if(Qt6_FOUND)
    list(APPEND BUILD_ALL_DEPS game_client)
endif()
add_custom_target(build_all
    DEPENDS ${BUILD_ALL_DEPS}
    COMMENT "构建所有服务和应用程序"
)

# 创建清理目标
add_custom_target(clean_all
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target clean
    COMMENT "清理所有构建文件"
)

# 创建文档生成目标（如果有Doxygen）
find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "生成API文档"
    )
    message(STATUS "  ✓ 文档生成目标 (docs)")
endif()

# ----------------------------------------------------------------------------
# 安装配置
# ----------------------------------------------------------------------------

# 设置安装路径
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "安装路径" FORCE)
endif()

message(STATUS "安装路径: ${CMAKE_INSTALL_PREFIX}")

# 安装可执行文件
set(INSTALL_TARGETS algorithm_service strategy_service)
if(Qt6_FOUND)
    list(APPEND INSTALL_TARGETS game_client)
endif()
install(TARGETS ${INSTALL_TARGETS}
    RUNTIME DESTINATION bin         # Windows: .exe文件
    LIBRARY DESTINATION lib         # Linux: .so文件
    ARCHIVE DESTINATION lib         # 静态库文件
)

# 安装配置文件
install(DIRECTORY configs/
    DESTINATION share/game/configs
    FILES_MATCHING PATTERN "*.json" PATTERN "*.yaml"
)

# 安装资源文件
install(DIRECTORY application/resources/
    DESTINATION share/game/resources
    FILES_MATCHING 
        PATTERN "*.png" 
        PATTERN "*.jpg" 
        PATTERN "*.wav" 
        PATTERN "*.mp3"
)

# 安装文档
install(DIRECTORY docs/
    DESTINATION share/doc/game
    FILES_MATCHING PATTERN "*.md" PATTERN "*.pdf"
)

# ----------------------------------------------------------------------------
# 打包配置 (CPack)
# ----------------------------------------------------------------------------

# 设置打包信息
set(CPACK_PACKAGE_NAME "幻境传说")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "基于Qt/C++的2D动作角色扮演游戏")
set(CPACK_PACKAGE_VENDOR "Game Studio")
set(CPACK_PACKAGE_CONTACT "chengkangpeng@example.com")

# 设置打包文件名
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}")

# 根据平台选择打包器
if(WIN32)
    set(CPACK_GENERATOR "NSIS;ZIP")         # Windows: NSIS安装包和ZIP压缩包
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop;TGZ")    # macOS: DMG镜像和压缩包
else()
    set(CPACK_GENERATOR "DEB;RPM;TGZ")      # Linux: DEB包、RPM包和压缩包
endif()

# 包含CPack模块
include(CPack)

# ----------------------------------------------------------------------------
# 构建信息输出
# ----------------------------------------------------------------------------

message(STATUS "=== 构建配置完成 ===")
message(STATUS "可用目标:")
message(STATUS "  make build_all     - 构建所有组件")
message(STATUS "  make test          - 运行测试套件") 
message(STATUS "  make install       - 安装到指定目录")
message(STATUS "  make package       - 创建发布包")
if(DOXYGEN_FOUND)
message(STATUS "  make docs          - 生成API文档")
endif()
message(STATUS "  make clean_all     - 清理构建文件")
message(STATUS "")
message(STATUS "构建命令示例:")
message(STATUS "  mkdir build && cd build")
message(STATUS "  cmake ..")
message(STATUS "  make build_all")
message(STATUS "================================")
