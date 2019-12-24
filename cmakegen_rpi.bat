mkdir build
cd build
cmake .. -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DLIB_BUILD_TYPE=SHARED -DCMAKE_VERBOSE_MAKEFILE=on -DTOOLCHAIN_RPI_TRIPLET="arm-linux-gnueabihf" -DCMAKE_TOOLCHAIN_FILE:PATH="rpi.toolchain.cmake"
cd ..

@echo off
REM to build: cd build && make
REM If you're using a Windows machine without "make" command then using the one under %TOOLCHAIN_RPI%/bin/make