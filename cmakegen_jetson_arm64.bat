REM sudo apt-get update
REM sudo apt-get install gcc-aarch64-linux-gnu
REM sudo apt install g++-aarch64-linux-gnu

mkdir build
cd build
cmake .. -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DLIB_BUILD_TYPE=STATIC -DCMAKE_VERBOSE_MAKEFILE=on -DTOOLCHAIN_CROSS_TRIPLET="aarch64-linux-gnu" -DCMAKE_TOOLCHAIN_FILE:PATH="../compv/jetson.arm64.toolchain.cmake"
cd ..

@echo off
REM to build: cd build && make
REM If you're using a Windows machine without "make" command then using the one under %TOOLCHAIN_ARM64%/bin/make