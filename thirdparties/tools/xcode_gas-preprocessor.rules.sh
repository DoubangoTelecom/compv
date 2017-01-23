#!/bin/bash
export GAS_ARCH=arm
if [ "${CURRENT_ARCH}" = "arm64" ]; then
export GAS_ARCH=aarch64
fi
perl ${PROJECT_DIR}/thirdparties/tools/gas-preprocessor.pl -arch ${GAS_ARCH} -as-type apple-clang -- xcrun -sdk iphoneos clang -c -arch ${CURRENT_ARCH} -I${PROJECT_DIR}/base/asm/arm -o ${OBJECT_FILE_DIR_normal}/${CURRENT_ARCH}/${INPUT_FILE_BASE}.o -v ${INPUT_FILE_DIR}/${INPUT_FILE_NAME}
