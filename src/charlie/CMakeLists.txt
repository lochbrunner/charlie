cmake_minimum_required(VERSION 3.0.0)
project(charlie)

find_package(Protobuf REQUIRED)
# find_package(Boost 1.66.0 REQUIRED system)
set(Boost_LIBRARIES "/usr/local/lib/libboost_system.so")

include_directories(
    ${Protobuf_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS 
    ${CMAKE_CURRENT_SOURCE_DIR}/protobuf/debug.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/protobuf/mapping.proto
)

add_library(charlie
    ./scanner.cc
    ./compiler.cc
    ./vm/register.cc
    ./vm/instruction.cc
    ./vm/state.cc
    ./vm/runtime.cc
    ./api/external_function_manager.cc
    ./program/function_declaration.cc
    ./program/variable_declaration.cc
    ./program/functionDef.cc
    ./program/mapping.cc
    ./program/statement.cc
    ./program/scope.cc
    ./program/unresolved_program.cc
    ./common/logging_component.cc
    ./common/comparer_string.cc
    ./common/io.cc
    ./token/base.cc
    ${PROTO_SRCS}
    ${PROTO_HDRS}
)

target_link_libraries(charlie
    ${Protobuf_LIBRARIES}
    ${Boost_LIBRARIES}
)
