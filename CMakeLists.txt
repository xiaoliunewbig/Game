cmake_minimum_required(VERSION 3.16)

project(Game
    VERSION 1.0.0
    DESCRIPTION "Fantasy Legend game framework"
    LANGUAGES CXX
)

# -----------------------------------------------------------------------------
# Global build configuration
# -----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

message(STATUS "=== Configuring Fantasy Legend ===")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")

# -----------------------------------------------------------------------------
# Compiler options
# -----------------------------------------------------------------------------
if(MSVC)
    add_compile_options(
        /W4
        /WX
        /utf-8
        /permissive-
    )
    add_compile_options(
        "$<$<CONFIG:Debug>:/Od>"
        "$<$<CONFIG:Debug>:/Zi>"
        "$<$<CONFIG:Debug>:/RTC1>"
    )
    add_compile_options(
        "$<$<CONFIG:Release>:/O2>"
        "$<$<CONFIG:Release>:/DNDEBUG>"
    )
else()
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Werror
        -fPIC
    )
    add_compile_options(
        "$<$<CONFIG:Debug>:-g>"
        "$<$<CONFIG:Debug>:-O0>"
        "$<$<CONFIG:Debug>:-D_DEBUG_MODE>"
    )
    add_compile_options(
        "$<$<CONFIG:Release>:-O3>"
        "$<$<CONFIG:Release>:-DNDEBUG>"
    )
endif()

# -----------------------------------------------------------------------------
# Dependency discovery
# -----------------------------------------------------------------------------
message(STATUS "Frontend: Electron (Qt frontend removed from default build)")

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
    message(WARNING "gRPC/Protobuf not found. gRPC services will be skipped.")
endif()

find_package(Threads REQUIRED)

# -----------------------------------------------------------------------------
# Output and include paths
# -----------------------------------------------------------------------------
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# -----------------------------------------------------------------------------
# Tests
# -----------------------------------------------------------------------------
enable_testing()
set(CMAKE_TEST_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests)

# -----------------------------------------------------------------------------
# Subdirectories
# -----------------------------------------------------------------------------
if(HAS_GRPC)
    add_subdirectory(proto)
    message(STATUS "Enabled: proto")
endif()

add_subdirectory(algorithm)
message(STATUS "Enabled: algorithm")

add_subdirectory(strategy)
message(STATUS "Enabled: strategy")

add_subdirectory(tests)
message(STATUS "Enabled: tests")

# -----------------------------------------------------------------------------
# Aggregate targets
# -----------------------------------------------------------------------------
set(BUILD_ALL_DEPS algorithm_service strategy_service)

if(HAS_GRPC)
    list(APPEND BUILD_ALL_DEPS algorithm_server strategy_server)
endif()

add_custom_target(build_all
    DEPENDS ${BUILD_ALL_DEPS}
    COMMENT "Build all available components"
)

add_custom_target(clean_all
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target clean
    COMMENT "Clean all generated artifacts"
)

find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generate API docs"
    )
endif()

# -----------------------------------------------------------------------------
# Install rules
# -----------------------------------------------------------------------------
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Install prefix" FORCE)
endif()

set(INSTALL_TARGETS algorithm_service strategy_service)

install(TARGETS ${INSTALL_TARGETS}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/configs")
    install(DIRECTORY configs/
        DESTINATION share/game/configs
        FILES_MATCHING PATTERN "*.json" PATTERN "*.yaml"
    )
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/docs")
    install(DIRECTORY docs/
        DESTINATION share/doc/game
        FILES_MATCHING PATTERN "*.md" PATTERN "*.pdf"
    )
endif()

# -----------------------------------------------------------------------------
# Package configuration
# -----------------------------------------------------------------------------
set(CPACK_PACKAGE_NAME "FantasyLegend")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "2D action RPG framework")
set(CPACK_PACKAGE_VENDOR "Game Studio")
set(CPACK_PACKAGE_CONTACT "chengkangpeng@example.com")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}")

if(WIN32)
    set(CPACK_GENERATOR "NSIS;ZIP")
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop;TGZ")
else()
    set(CPACK_GENERATOR "DEB;RPM;TGZ")
endif()

include(CPack)

message(STATUS "=== Configure complete ===")
message(STATUS "Targets: build_all, test, install, package, clean_all")
if(DOXYGEN_FOUND)
    message(STATUS "Docs target available: docs")
endif()
