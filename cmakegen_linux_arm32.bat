REM sudo apt-get update
REM sudo apt-get install gcc-arm-linux-gnueabihf
REM sudo apt-get install g++-arm-linux-gnueabihf

mkdir build
cd build
cmake .. -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DLIB_BUILD_TYPE=STATIC -DCMAKE_VERBOSE_MAKEFILE=on -DTOOLCHAIN_CROSS_TRIPLET="arm-linux-gnueabihf" -DCMAKE_TOOLCHAIN_FILE:PATH="../compv/linux.arm32.toolchain.cmake"
cd ..

@echo off
REM to build: cd build && make
REM If you're using a Windows machine without "make" command then using the one under %TOOLCHAIN_ARM32%/bin/make