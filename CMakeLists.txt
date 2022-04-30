cmake_minimum_required(VERSION 3.22)
project(SprintPCB C)

set(CMAKE_C_STANDARD 99)

add_subdirectory(libsprintpcb)

add_executable(SprintPCB main.c nuklear.c nuklear.h nuklear_gdi.c)

target_link_libraries(SprintPCB PUBLIC libsprintpcb user32.lib gdi32.lib Msimg32.lib)

target_include_directories(SprintPCB PUBLIC
        "${PROJECT_BINARY_DIR}"
        "${PROJECT_SOURCE_DIR}/../libsprintpcb"
        )
