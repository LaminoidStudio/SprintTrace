#
# libsprintpcb: PCB manipulation library and plugin foundation
# Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
# Licensed under the terms and conditions of the GPLv3.
#

cmake_minimum_required(VERSION 3.22)
project(SprintPCB C)

set(CMAKE_C_STANDARD 99)

add_library(SprintPCB errors.c errors.h token.c token.h elements.c elements.h primitives.c primitives.h list.c list.h stringbuilder.c stringbuilder.h parser.c parser.h pcb.c pcb.h plugin.c plugin.h grid.c grid.h output.c output.h)
set_target_properties(SprintPCB PROPERTIES OUTPUT_NAME "sprintpcb")
