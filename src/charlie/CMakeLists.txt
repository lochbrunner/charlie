cmake_minimum_required(VERSION 3.0.0)
project(charlie)

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
    ./program/statement.cc
    ./program/scope.cc
    ./program/unresolved_program.cc
    ./common/logging_component.cc
    ./common/comparer_string.cc
    ./common/io.cc
    ./token/base.cc
)