# Proto 层 CMakeLists.txt
# 编译 .proto 文件并生成 C++ 源代码

cmake_minimum_required(VERSION 3.16)

# 查找 protobuf 编译器和 gRPC 插件
if(TARGET protobuf::protoc)
    get_target_property(PROTOBUF_PROTOC_EXECUTABLE protobuf::protoc IMPORTED_LOCATION_RELEASE)
    if(NOT PROTOBUF_PROTOC_EXECUTABLE)
        get_target_property(PROTOBUF_PROTOC_EXECUTABLE protobuf::protoc IMPORTED_LOCATION)
    endif()
else()
    find_program(PROTOBUF_PROTOC_EXECUTABLE protoc)
endif()

if(TARGET gRPC::grpc_cpp_plugin)
    get_target_property(GRPC_CPP_PLUGIN gRPC::grpc_cpp_plugin IMPORTED_LOCATION_RELEASE)
    if(NOT GRPC_CPP_PLUGIN)
        get_target_property(GRPC_CPP_PLUGIN gRPC::grpc_cpp_plugin IMPORTED_LOCATION)
    endif()
else()
    find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin)
endif()

set(PROTO_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(PROTO_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")

# Proto 文件列表
set(PROTO_FILES
    "${PROTO_SRC_DIR}/AlgorithmService.proto"
    "${PROTO_SRC_DIR}/StrategyService.proto"
)

# 生成的文件列表
set(PROTO_GENERATED_SRCS
    "${PROTO_OUT_DIR}/AlgorithmService.pb.cc"
    "${PROTO_OUT_DIR}/AlgorithmService.grpc.pb.cc"
    "${PROTO_OUT_DIR}/StrategyService.pb.cc"
    "${PROTO_OUT_DIR}/StrategyService.grpc.pb.cc"
)

set(PROTO_GENERATED_HDRS
    "${PROTO_OUT_DIR}/AlgorithmService.pb.h"
    "${PROTO_OUT_DIR}/AlgorithmService.grpc.pb.h"
    "${PROTO_OUT_DIR}/StrategyService.pb.h"
    "${PROTO_OUT_DIR}/StrategyService.grpc.pb.h"
)

# 自定义命令：编译 AlgorithmService.proto
add_custom_command(
    OUTPUT
        "${PROTO_OUT_DIR}/AlgorithmService.pb.cc"
        "${PROTO_OUT_DIR}/AlgorithmService.pb.h"
        "${PROTO_OUT_DIR}/AlgorithmService.grpc.pb.cc"
        "${PROTO_OUT_DIR}/AlgorithmService.grpc.pb.h"
    COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
        --proto_path="${PROTO_SRC_DIR}"
        --cpp_out="${PROTO_OUT_DIR}"
        --grpc_out="${PROTO_OUT_DIR}"
        --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
        "${PROTO_SRC_DIR}/AlgorithmService.proto"
    DEPENDS "${PROTO_SRC_DIR}/AlgorithmService.proto"
    COMMENT "编译 AlgorithmService.proto"
)

# 自定义命令：编译 StrategyService.proto
add_custom_command(
    OUTPUT
        "${PROTO_OUT_DIR}/StrategyService.pb.cc"
        "${PROTO_OUT_DIR}/StrategyService.pb.h"
        "${PROTO_OUT_DIR}/StrategyService.grpc.pb.cc"
        "${PROTO_OUT_DIR}/StrategyService.grpc.pb.h"
    COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
        --proto_path="${PROTO_SRC_DIR}"
        --cpp_out="${PROTO_OUT_DIR}"
        --grpc_out="${PROTO_OUT_DIR}"
        --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
        "${PROTO_SRC_DIR}/StrategyService.proto"
    DEPENDS "${PROTO_SRC_DIR}/StrategyService.proto"
    COMMENT "编译 StrategyService.proto"
)

# 创建静态库
add_library(proto_generated STATIC
    ${PROTO_GENERATED_SRCS}
)

target_include_directories(proto_generated PUBLIC
    "${PROTO_OUT_DIR}"
)

# 链接 protobuf 和 gRPC
if(TARGET gRPC::grpc++)
    target_link_libraries(proto_generated PUBLIC
        protobuf::libprotobuf
        gRPC::grpc++
    )
else()
    target_link_libraries(proto_generated PUBLIC
        ${GRPC_LIBRARIES}
        ${PROTOBUF_LIBRARIES}
    )
endif()

# Proto 生成的代码可能有编译器警告，禁用 -Werror
if(MSVC)
    target_compile_options(proto_generated PRIVATE /W0)
else()
    target_compile_options(proto_generated PRIVATE -w)
endif()

set_property(TARGET proto_generated PROPERTY CXX_STANDARD 20)
set_property(TARGET proto_generated PROPERTY CXX_STANDARD_REQUIRED ON)
