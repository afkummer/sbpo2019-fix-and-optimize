# The MIT License (MIT)
#
# Copyright (c) 2019
# Alberto Francisco Kummer Neto (afkneto@inf.ufrgs.br),
# Luciana Salete Buriol (buriol@inf.ufrgs.br) and
# Olinto César Bassi de Araújo (olinto@ctism.ufsm.br)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the "Software"), to deal in
# the Software without restriction, including without limitation the rights to
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
# the Software, and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
# IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

cmake_minimum_required(VERSION 3.6)

#
# Project settings and generic definitions.
#
project(hhcrsp-feo)

add_definitions(
   -Wall 
   -Wextra 
   -Wfloat-conversion 
   -std=c++17 
   -pipe 
   -march=native
   -mtune=native
   -flto 
   -fopenmp
)

# Debug and Release definitions.
# To set the build mode, specify the value of var `CMAKE_BUILD_TYPE` when
# running CMake.
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -s")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -ggdb3 -fno-omit-frame-pointer")

# Fallback to Debug build type automatically
# if no compilation mode was specified.
if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE "Debug")
endif()

message("-- Using compilation flags of mode \"" ${CMAKE_BUILD_TYPE} "\"")

# Include dependencies from CPLEX MIP solver.
include(cplex.cmake)


#
# Project compilation settings.
#

# Set the compiler to search for headers inside the source code dir.
include_directories(src)


add_executable(fixAndOptimize
   src/FixAndOptimize.cpp
   src/InitialRouting.cpp
   src/Instance.cpp
   src/mainFeo.cpp
   src/MipModel.cpp
)
target_link_libraries(fixAndOptimize ${CPLEX_LIBRARIES})

