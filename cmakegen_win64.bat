mkdir build
cd build
cmake .. -DCMAKE_GENERATOR_PLATFORM=x64 -DVCPKG_TARGET_TRIPLET=x64-windows -DCMAKE_TOOLCHAIN_FILE=C:/Projects/vcpkg/scripts/buildsystems/vcpkg.cmake
cd ..