PROJECT(hystack-libnodes)

# The version number.
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

# set compiler.
SET(CMAKE_C_COMPILER "/usr/bin/gcc")
SET(CMAKE_CXX_COMPILER "/usr/bin/g++")
 
ADD_COMPILE_OPTIONS("-Wall")
ADD_COMPILE_OPTIONS("-Werror")

if(CMAKE_COMPILER_IS_GNUCXX)
ADD_COMPILE_OPTIONS("-g")
ADD_COMPILE_OPTIONS("-std=c++11")
ADD_COMPILE_OPTIONS("-D_GNU_SOURCE")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

ADD_DEFINITIONS(-DHAVE_SSL)

# Add the all of headfile.
INCLUDE_DIRECTORIES(/usr/include)

# libs src include.
INCLUDE_DIRECTORIES(../../libs)
INCLUDE_DIRECTORIES(../../src/core)
INCLUDE_DIRECTORIES(../../src/libcpp)
INCLUDE_DIRECTORIES(../../src/unix)
INCLUDE_DIRECTORIES(../../src/nodes)
 
# Add the all of file.
AUX_SOURCE_DIRECTORY(../../src/nodes SRC_NODES)

FIND_LIBRARY(LIB_CPP libcpp.a ../hystack-libcpp NOTFOUND)

# Add the all of file.
ADD_LIBRARY(nodes-1.3.1 STATIC ${SRC_NODES})
TARGET_LINK_LIBRARIES(nodes-1.3.1 ${LIB_CPP})