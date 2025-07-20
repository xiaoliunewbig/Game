# ============================================================================
# 《幻境传说》游戏框架 - 主构建配置文件
# ============================================================================
# 文件名: CMakeLists.txt
# 说明: 项目根目录的主CMake配置文件，定义整个项目的构建规则
# 作者: 彭承康
# 创建时间: 2025-07-20
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
    add_compile_options($<$<CONFIG:Debug>:/Od /Zi /RTC1>)
    # Release模式特定选项
    add_compile_options($<$<CONFIG:Release>:/O2 /DNDEBUG>)
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
    add_compile_options($<$<CONFIG:Debug>:-g -O0 -DDEBUG>)
    # Release模式特定选项
    add_compile_options($<$<CONFIG:Release>:-O3 -DNDEBUG>)
endif()

# ----------------------------------------------------------------------------
# 全局依赖项查找
# ----------------------------------------------------------------------------

# 查找必需的依赖包
message(STATUS "查找项目依赖...")

# Qt6 - 用于应用层UI
find_package(Qt6 REQUIRED COMPONENTS 
    Core                            # Qt核心模块
    Quick                           # Qt Quick (QML)
    Widgets                         # Qt Widgets
    Multimedia                      # 多媒体支持
    Network                         # 网络支持
)

# gRPC - 用于服务间通信
find_package(PkgConfig REQUIRED)
pkg_check_modules(GRPC REQUIRED grpc++)
pkg_check_modules(PROTOBUF REQUIRED protobuf)

# 线程支持
find_package(Threads REQUIRED)

# 输出依赖信息
message(STATUS "Qt6版本: ${Qt6_VERSION}")
message(STATUS "gRPC版本: ${GRPC_VERSION}")

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

# 添加算法层服务
# 提供战斗计算、AI决策等核心算法
add_subdirectory(algorithm)
message(STATUS "  ✓ 算法层服务 (algorithm)")

# 添加策略层服务  
# 提供游戏规则、世界状态管理
add_subdirectory(strategy)
message(STATUS "  ✓ 策略层服务 (strategy)")

# 添加Qt应用层
# 游戏客户端和用户界面
add_subdirectory(application)
message(STATUS "  ✓ Qt应用层 (application)")

# 添加测试套件
# 集成测试和单元测试
add_subdirectory(tests)
message(STATUS "  ✓ 测试套件 (tests)")

# ----------------------------------------------------------------------------
# 自定义构建目标
# ----------------------------------------------------------------------------

# 创建一键构建目标
# 按正确顺序构建所有核心组件
add_custom_target(build_all
    DEPENDS 
        algorithm_service           # 算法层服务
        strategy_service           # 策略层服务  
        game_client               # Qt游戏客户端
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
install(TARGETS 
    algorithm_service 
    strategy_service 
    game_client
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
