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

cmake_minimum_required(VERSION 3.0)

# CPLEX-related configurations
# Configuration of CPLEX libraries.
if (NOT CPLEX_PREFIX)
   set(CPLEX_PREFIX "/home/alberto/.local/ibm/ILOG/CPLEX_Studio201")
endif()
message(STATUS "Using CPLEX_PREFIX = \"${CPLEX_PREFIX}\"")

# Search for header files.
find_path (CPLEX_HEADER ilcplex/ilocplex.h ${CPLEX_PREFIX}/cplex/include)
find_path (CONCERT_HEADER ilconcert/iloenv.h ${CPLEX_PREFIX}/concert/include)

# And look for its libraries.
# You may need to change these paths according to your architecture.
find_library (CPLEX_LIB_CPLEX cplex "${CPLEX_PREFIX}/cplex/lib/x86-64_linux/static_pic/")
find_library (CPLEX_LIB_ILOCPLEX ilocplex "${CPLEX_PREFIX}/cplex/lib/x86-64_linux/static_pic/")
find_library (CPLEX_LIB_CONCERT concert "${CPLEX_PREFIX}/concert/lib/x86-64_linux/static_pic/")
set(CPLEX_INCLUDE_DIRS ${CPLEX_HEADER} ${CONCERT_HEADER})
set(CPLEX_LIBRARIES ${CPLEX_LIB_ILOCPLEX} ${CPLEX_LIB_CPLEX} ${CPLEX_LIB_CONCERT} -pthread -lgomp -ldl -Wdeprecated-declarations)

# Needed by GCC > 4.9 (and > 5.2 for `ignored-attributes`) to
# supress warnings related to CPLEX headers.
add_definitions(-Wno-deprecated)

# Now, put the path of CPLEX header in compiler search path.
include_directories(${CPLEX_INCLUDE_DIRS})

