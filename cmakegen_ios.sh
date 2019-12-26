# More info https://github.com/leetal/ios-cmake

mkdir build
cd build
cmake .. -G Xcode -DCMAKE_TOOLCHAIN_FILE=../ios.toolchain.cmake -DPLATFORM=OS64
cd ..

