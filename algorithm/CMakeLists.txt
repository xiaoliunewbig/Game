# 算法层 CMakeLists.txt
# 文件名: CMakeLists.txt
# 说明: 算法层构建配置文件
# 作者: 彭承康
# 创建时间: 2025-07-19
#
# 本文件定义算法层的编译配置，包括源文件、头文件目录、
# 编译选项和依赖库的设置。

cmake_minimum_required(VERSION 3.16)

project(AlgorithmService CXX)

# 添加算法服务静态库
add_library(algorithm_service STATIC
    src/AlgorithmService.cpp
    src/DamageCalculator.cpp
    src/AIDecisionEngine.cpp
    src/SkillTreeManager.cpp
)

# 头文件目录
target_include_directories(algorithm_service PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

# C++20 标准
set_property(TARGET algorithm_service PROPERTY CXX_STANDARD 20)
set_property(TARGET algorithm_service PROPERTY CXX_STANDARD_REQUIRED ON)

# 链接线程库
find_package(Threads REQUIRED)
target_link_libraries(algorithm_service PUBLIC Threads::Threads)

# 编译选项
target_compile_options(algorithm_service PRIVATE
    -Wall -Wextra -Wpedantic
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)
