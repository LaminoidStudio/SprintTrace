cmake_minimum_required(VERSION 3.22)
project(SprintPCB C)

set(CMAKE_C_STANDARD 99)

add_subdirectory(libsprintpcb)

add_executable(SprintPCB main.c)

target_link_libraries(SprintPCB PUBLIC libsprintpcb)

target_include_directories(SprintPCB PUBLIC
        "${PROJECT_BINARY_DIR}"
        "${PROJECT_SOURCE_DIR}/../libsprintpcb"
        )
