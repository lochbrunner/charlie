cmake_minimum_required(VERSION 3.0.0)
project(charlie)

FIND_PACKAGE( Boost 1.40 COMPONENTS program_options REQUIRED )

include_directories(
    ../charlie
    ${Boost_INCLUDE_DIR}
)

add_executable(charliec
    ./log.cc
    ./main.cc
)

target_link_libraries(charliec
    charlie
    ${Boost_LIBRARIES}
)
