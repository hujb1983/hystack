PROJECT(Asnode)

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

ADD_DEFINITIONS(-DOPENSSL)
ADD_DEFINITIONS(-DMODULE_NODES)

# libs src include.
INCLUDE_DIRECTORIES(../../libs)
INCLUDE_DIRECTORIES(../../src/core)
INCLUDE_DIRECTORIES(../../src/kits)
INCLUDE_DIRECTORIES(../../src/unix)
INCLUDE_DIRECTORIES(../../src/nodes)

# Add the all of file.
AUX_SOURCE_DIRECTORY(../../src/kits/alloc ALLOC)
AUX_SOURCE_DIRECTORY(../../src/kits/conv CONV)
AUX_SOURCE_DIRECTORY(../../src/kits/dev DEV)
AUX_SOURCE_DIRECTORY(../../src/kits/event EVENT)
AUX_SOURCE_DIRECTORY(../../src/kits/files FILES)
AUX_SOURCE_DIRECTORY(../../src/kits/inet INET)
AUX_SOURCE_DIRECTORY(../../src/kits/logs LOGS)
AUX_SOURCE_DIRECTORY(../../src/kits/method METHOD)
AUX_SOURCE_DIRECTORY(../../src/kits/module MODULE)
AUX_SOURCE_DIRECTORY(../../src/kits/process PROCESS)
AUX_SOURCE_DIRECTORY(../../src/kits/thread THREADS)

# Add the all of file.
AUX_SOURCE_DIRECTORY(../../src/core SRC_CORE)
AUX_SOURCE_DIRECTORY(../../src/unix SRC_UNIX)
AUX_SOURCE_DIRECTORY(../../src/nodes SRC_NODES)

# build to Asnode
ADD_EXECUTABLE(Asnode ${SRC_CORE} ${SRC_UNIX} 
	${ALLOC} ${CONV} ${DEV} ${EVENT} ${FILES} 
	${INET} ${LOGS} ${METHOD} ${MODULE} ${PROCESS} ${THREADS}
	${SRC_NODES})

TARGET_LINK_LIBRARIES(Asnode dl rt pthread curl crypto ssl)
