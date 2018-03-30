cmake_minimum_required(VERSION 3.0.0)
project(charlie)

include_directories(../charlie)

add_executable(charliec
    ./flag.cc
    ./log.cc
    ./main.cc
    ./command.cc
)

target_link_libraries(charliec
    charlie
)


