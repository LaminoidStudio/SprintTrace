cmake_minimum_required(VERSION 3.22)
project(SprintPCB C)

set(CMAKE_C_STANDARD 99)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")

add_subdirectory(libsprintpcb)

add_executable(SprintPCB main.c nuklear.c nuklear.h)

target_link_libraries(SprintPCB PUBLIC libsprintpcb)

if (WIN32)
    target_sources(SprintPCB PUBLIC nuklear_gdi.c nuklear_gdi.h)
    target_link_libraries(SprintPCB PUBLIC user32.lib gdi32.lib Msimg32.lib)
    target_link_options(SprintPCB PUBLIC "-mwindows")
else()
    target_sources(SprintPCB PUBLIC nuklear_sdl.c nuklear_sdl.h)
    find_package(SDL2 REQUIRED)
    include_directories(SprintPCB PUBLIC ${SDL2_INCLUDE_DIR})
    target_link_libraries(SprintPCB PUBLIC ${SDL2_LIBRARY})
endif()

target_include_directories(SprintPCB PUBLIC
        "${PROJECT_BINARY_DIR}"
        "${PROJECT_SOURCE_DIR}/../libsprintpcb"
        )
