## Linux ARM64 cross compilation toolchain ##
# sudo apt-get update
# sudo apt-get install gcc-arm-linux-gnueabihf
# sudo apt install g++-arm-linux-gnueabihf

message(STATUS "Using ARM32 cross compilation toolchain")

set (CMAKE_SYSTEM_NAME Linux)
set (CMAKE_SYSTEM_PROCESSOR arm)

# Set cross compile target os
set (CROSSCOMPILING_TARGET_OS "LINUX")
set (TARGET_OS "LINUX")

## Set name if not defined as argument ##
if (NOT TOOLCHAIN_CROSS_TRIPLET)
	set(TOOLCHAIN_CROSS_TRIPLET "arm-linux-gnueabihf") # may also be set to different value using cmake .. -DTOOLCHAIN_CROSS_TRIPLET="arm-linux-gnueabihf" ......
endif ()

## Get path to the toolchain using ENV ##
if (DEFINED ENV{TOOLCHAIN_CROSS_ARM32})
	string(REPLACE "\\" "/" TOOLCHAIN_CROSS_BIN_FOLDER "$ENV{TOOLCHAIN_CROSS_ARM32}/bin/")
else ()
	set(TOOLCHAIN_CROSS_BIN_FOLDER "")
endif()
if (WIN32)
    set(TOOLCHAIN_CROSS_BIN_EXTENSION ".exe")
endif (WIN32)

set(CMAKE_C_COMPILER "${TOOLCHAIN_CROSS_BIN_FOLDER}${TOOLCHAIN_CROSS_TRIPLET}-gcc${TOOLCHAIN_CROSS_BIN_EXTENSION}")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_CROSS_BIN_FOLDER}${TOOLCHAIN_CROSS_TRIPLET}-g++${TOOLCHAIN_CROSS_BIN_EXTENSION}")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)