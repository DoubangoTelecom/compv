#/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_TESTS_COMMON_H_)
#define _COMPV_TESTS_COMMON_H_

#include <compv/compv_api.h>

using namespace compv;

#define TAG_TESTS_COMMON	"TESTS_COMMON"

#define COMPV_numThreads			COMPV_NUM_THREADS_SINGLE
#define COMPV_enableIntrinsics		true
#define COMPV_enableAsm				true
#define COMPV_enableGPU				true
#define COMPV_enableMathFixedPoint	true
#define COMPV_enableTestingMode		true
#define COMPV_enableIntelIpp		false
#define COMPV_enableIntelTbb		false
#define COMPV_cpuDisable			kCpuFlagNone

COMPV_NAMESPACE_BEGIN()
COMPV_GCC_DISABLE_WARNINGS_BEGIN("-Wunused-function")

static const COMPV_ERROR_CODE compv_tests_init()
{
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	CompVDebugMgr::setLevel(COMPV_DEBUG_LEVEL_INFO);
	CompVParallel::setIntelTbbEnabled(COMPV_enableIntelTbb); // before initializing
	
	COMPV_CHECK_CODE_ASSERT(err = CompVInit(COMPV_numThreads));
	COMPV_CHECK_CODE_ASSERT(err = CompVBase::setTestingModeEnabled(COMPV_enableTestingMode));
	COMPV_CHECK_CODE_ASSERT(err = CompVCpu::setMathFixedPointEnabled(COMPV_enableMathFixedPoint));
	COMPV_CHECK_CODE_ASSERT(err = CompVCpu::setAsmEnabled(COMPV_enableAsm));
	COMPV_CHECK_CODE_ASSERT(err = CompVCpu::setIntrinsicsEnabled(COMPV_enableIntrinsics));
	COMPV_CHECK_CODE_ASSERT(err = CompVCpu::setIntelIppEnabled(COMPV_enableIntelIpp));
	COMPV_CHECK_CODE_ASSERT(err = CompVCpu::flagsDisable(COMPV_cpuDisable));
	COMPV_CHECK_CODE_ASSERT(err = CompVGpu::setEnabled(COMPV_enableGPU));
	
	return err;
}

static const COMPV_ERROR_CODE compv_tests_deInit()
{
	COMPV_ERROR_CODE err;
	COMPV_CHECK_CODE_ASSERT(err = CompVDeInit());
	return err;
}

static const std::string compv_tests_path_from_file(const char* filename, const char* optional_folder = NULL)
{
	std::string path = COMPV_PATH_FROM_NAME(filename); // path from android's assets, iOS' bundle....
	// The path isn't correct when the binary is loaded from another process(e.g. when Intel VTune is used)
	if (optional_folder && !CompVFileUtils::exists(path.c_str())) {
		path = std::string(optional_folder) + std::string("/") + std::string(filename);
	}
	return path;
}

static COMPV_ERROR_CODE compv_tests_write_to_file(const CompVMatPtr& mat, const char* filename)
{
	COMPV_CHECK_EXP_RETURN(!mat || mat->isEmpty() || !filename, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	FILE* file = fopen(filename, "wb+");
	COMPV_CHECK_EXP_RETURN(!file, COMPV_ERROR_CODE_E_FILE_NOT_FOUND, "Failed to open file");

	COMPV_DEBUG_INFO_EX(TAG_TESTS_COMMON, "Writing %s file... (w=%zu, h=%zu, s=%zu)", filename, mat->cols(), mat->rows(), mat->stride());
	int32_t planes = static_cast<int32_t>(mat->planeCount());
	for (int32_t plane = 0; plane < planes; ++plane) {
		size_t planeHeight = mat->rows(plane);
		size_t planeWidth = mat->rowInBytes(plane);
		size_t planeStride = mat->strideInBytes(plane);
		const uint8_t* planePtr = mat->ptr<const uint8_t>(0, 0, plane);
		for (size_t i = 0; i < planeHeight; ++i) {
			fwrite(planePtr, 1, planeWidth, file);
			planePtr += planeStride;
		}
	}
	if (file) {
		fclose(file);
	}
	return COMPV_ERROR_CODE_S_OK;
}

static const std::string compv_tests_md5(const CompVMatPtr& mat)
{
	if (mat && !mat->isEmpty()) {
		CompVMd5Ptr md5;
		COMPV_CHECK_CODE_ASSERT(CompVMd5::newObj(&md5), "Failed to create MD5 computer");
		int32_t planes = static_cast<int32_t>(mat->planeCount());
		for (int32_t plane = 0; plane < planes; ++plane) {
			size_t planeHeight = mat->rows(plane);
			size_t planeWidth = mat->rowInBytes(plane);
			size_t planeStride = mat->strideInBytes(plane);
			const uint8_t* planePtr = mat->ptr<const uint8_t>(0, 0, plane);
			for (size_t i = 0; i < planeHeight; ++i) {
				COMPV_CHECK_CODE_ASSERT(md5->update(planePtr, planeWidth));
				planePtr += planeStride;
			}
		}
		return md5->compute();
	}
	return COMPV_MD5_EMPTY;
}

static COMPV_INLINE bool compv_tests_is_fma_enabled()
{
    // On X86 FMA requires AVX2 (both intrinsics and asm)
    // On ARM FMA requires NEON (only asm) - When intrinsic code is built without "-mfpu=neon-vfpv4" then, the compiler generate very slooow code for vfma function.
#if COMPV_ARCH_X86
    return (CompVCpu::isEnabled(kCpuFlagAVX2) && CompVCpu::isEnabled(kCpuFlagFMA3)) && (CompVCpu::isAsmEnabled() || CompVCpu::isIntrinsicsEnabled());
#elif COMPV_ARCH_ARM
    return (CompVCpu::isEnabled(kCpuFlagARM_NEON) && CompVCpu::isEnabled(kCpuFlagARM_NEON_FMA)) && CompVCpu::isAsmEnabled();
#endif
    return false;
}


COMPV_GCC_DISABLE_WARNINGS_END()
COMPV_NAMESPACE_END()

#endif /* _COMPV_TESTS_COMMON_H_ */
