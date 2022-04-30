cmake_minimum_required(VERSION 3.22)
project(libsprintpcb C)

set(CMAKE_C_STANDARD 99)

add_library(libsprintpcb errors.h token.c token.h elements.h primitives.c primitives.h elements.c list.c list.h stringbuilder.c stringbuilder.h math.c math.h parser.c parser.h)
