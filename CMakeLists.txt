cmake_minimum_required(VERSION 3.16)
project(Game CXX)

#
# 主 CMakeLists.txt
#

# 启用测试
enable_testing()

# 添加子目录
add_subdirectory(strategy)
add_subdirectory(tests)
