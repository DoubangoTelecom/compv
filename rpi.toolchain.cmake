## Raspberry PI toolchain ##
#	- Windows: http://sysprogs.com/getfile/566/raspberry-gcc8.3.0.exe
#	- Linux: sudo apt-get install crossbuild-essential-armhf
# More info cross compilation at https://cmake.org/cmake/help/v3.6/manual/cmake-toolchains.7.html

message(STATUS "Using Raspberry PI cross compilation toolchain")

set (CMAKE_SYSTEM_NAME Linux)
set (CMAKE_SYSTEM_PROCESSOR arm)

# Set cross compile target os
set (CROSSCOMPILING_TARGET_OS "PI")
set (TARGET_OS "PI")

## Set name if not defined as argument ##
if (NOT TOOLCHAIN_RPI_TRIPLET)
	set(TOOLCHAIN_RPI_TRIPLET "arm-linux-gnueabihf") # may also be set to different value using cmake .. -DTOOLCHAIN_RPI_TRIPLET="arm-none-eabi" ......
endif ()

## Get path to the toolchain using ENV ##
if (DEFINED ENV{TOOLCHAIN_RPI})
	string(REPLACE "\\" "/" TOOLCHAIN_RPI_BIN_FOLDER "$ENV{TOOLCHAIN_RPI}/bin/")
else ()
	set(TOOLCHAIN_RPI_BIN_FOLDER "")
endif()
if (WIN32)
    set(TOOLCHAIN_RPI_BIN_EXTENSION ".exe")
endif (WIN32)

set(CMAKE_C_COMPILER "${TOOLCHAIN_RPI_BIN_FOLDER}${TOOLCHAIN_RPI_TRIPLET}-gcc${TOOLCHAIN_RPI_BIN_EXTENSION}")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_RPI_BIN_FOLDER}${TOOLCHAIN_RPI_TRIPLET}-g++${TOOLCHAIN_RPI_BIN_EXTENSION}")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)