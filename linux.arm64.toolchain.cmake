## Linux ARM64 cross compilation toolchain ##
# sudo apt-get update
# sudo apt-get install gcc-aarch64-linux-gnu
# sudo apt install g++-aarch64-linux-gnu

message(STATUS "Using ARM64 cross compilation toolchain")

set (CMAKE_SYSTEM_NAME Linux)
set (CMAKE_SYSTEM_PROCESSOR arm64)

# Set cross compile target os
set (CROSSCOMPILING_TARGET_OS "LINUX")
set (TARGET_OS "LINUX")

## Set name if not defined as argument ##
if (NOT TOOLCHAIN_ARM64_TRIPLET)
	set(TOOLCHAIN_ARM64_TRIPLET "aarch64-linux-gnu") # may also be set to different value using cmake .. -DTOOLCHAIN_ARM64_TRIPLET="aarch64-linux-gnu" ......
endif ()

## Get path to the toolchain using ENV ##
if (DEFINED ENV{TOOLCHAIN_ARM64})
	string(REPLACE "\\" "/" TOOLCHAIN_ARM64_BIN_FOLDER "$ENV{TOOLCHAIN_ARM64}/bin/")
else ()
	set(TOOLCHAIN_ARM64_BIN_FOLDER "")
endif()
if (WIN32)
    set(TOOLCHAIN_ARM64_BIN_EXTENSION ".exe")
endif (WIN32)

set(CMAKE_C_COMPILER "${TOOLCHAIN_ARM64_BIN_FOLDER}${TOOLCHAIN_ARM64_TRIPLET}-gcc${TOOLCHAIN_ARM64_BIN_EXTENSION}")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_ARM64_BIN_FOLDER}${TOOLCHAIN_ARM64_TRIPLET}-g++${TOOLCHAIN_ARM64_BIN_EXTENSION}")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)