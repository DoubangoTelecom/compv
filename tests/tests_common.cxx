#/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "tests_common.h"

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

COMPV_GCC_DISABLE_WARNINGS_BEGIN("-Wunused-function")

const COMPV_ERROR_CODE compv_tests_init()
{
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	CompVDebugMgr::setLevel(COMPV_DEBUG_LEVEL_INFO);
	CompVParallel::setIntelTbbEnabled(COMPV_enableIntelTbb); // before initializing

	COMPV_CHECK_CODE_ASSERT(err = CompVInit(COMPV_numThreads));
	COMPV_CHECK_CODE_ASSERT(err = CompVParallel::multiThreadingSetMaxThreads(COMPV_numThreads)); // "CompVInit" is nop if already initialized -> using "multiThreadingSetMaxThreads" to change number of threads
	COMPV_CHECK_CODE_ASSERT(err = CompVBase::setTestingModeEnabled(COMPV_enableTestingMode));
	COMPV_CHECK_CODE_ASSERT(err = CompVCpu::setMathFixedPointEnabled(COMPV_enableMathFixedPoint));
	COMPV_CHECK_CODE_ASSERT(err = CompVCpu::setAsmEnabled(COMPV_enableAsm));
	COMPV_CHECK_CODE_ASSERT(err = CompVCpu::setIntrinsicsEnabled(COMPV_enableIntrinsics));
	COMPV_CHECK_CODE_ASSERT(err = CompVCpu::setIntelIppEnabled(COMPV_enableIntelIpp));
	COMPV_CHECK_CODE_ASSERT(err = CompVCpu::flagsDisable(COMPV_cpuDisable));
	COMPV_CHECK_CODE_ASSERT(err = CompVGpu::setEnabled(COMPV_enableGPU));

	return err;
}

const COMPV_ERROR_CODE compv_tests_deInit()
{
	COMPV_ERROR_CODE err;
	COMPV_CHECK_CODE_ASSERT(err = CompVDeInit());
	return err;
}

const std::string compv_tests_path_from_file(const char* filename, const char* optional_folder COMPV_DEFAULT(nullptr))
{
	std::string path = COMPV_PATH_FROM_NAME(filename); // path from android's assets, iOS' bundle....
													   // The path isn't correct when the binary is loaded from another process(e.g. when Intel VTune is used)
	if (optional_folder && !CompVFileUtils::exists(path.c_str())) {
		path = std::string(optional_folder) + std::string("/") + std::string(filename);
	}
	return path;
}

// YuvPlayer can guess image size from the name (e.g. out_1280x720.yuv)
const std::string compv_tests_build_filename(const CompVMatPtr& mat)
{
	const size_t width = mat ? mat->cols() : 0;
	const size_t height = mat ? mat->rows() : 0;
	return std::string("out_")
		+ CompVBase::to_string(width)
		+ std::string("x")
		+ CompVBase::to_string(height)
		+ std::string(".yuv");
}

COMPV_ERROR_CODE compv_tests_write_to_file(const CompVMatPtr& mat, const char* filename)
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

const std::string compv_tests_md5(const CompVMatPtr& mat)
{
	if (mat && !mat->isEmpty()) {
		CompVMd5Ptr md5;
		COMPV_CHECK_CODE_ASSERT(CompVMd5::newObj(&md5), "Failed to create MD5 computer");
		int planes = static_cast<int>(mat->planeCount());
		for (int plane = 0; plane < planes; ++plane) {
			const size_t planeHeight = mat->rows(plane);
			const size_t planeWidth = mat->rowInBytes(plane);
			const size_t planeStride = mat->strideInBytes(plane);
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

COMPV_ERROR_CODE compv_tests_draw_bbox(CompVMatPtr mat, const CompVConnectedComponentBoundingBox& bb, const uint8_t color)
{
	COMPV_CHECK_EXP_RETURN(!mat || mat->planeCount() != 1 || mat->elmtInBytes() != sizeof(uint8_t), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	uint8_t* top = mat->ptr<uint8_t>(static_cast<size_t>(bb.top));
	uint8_t* bottom = mat->ptr<uint8_t>(static_cast<size_t>(bb.bottom));
	// top and bottom hz lines
	for (int16_t k = bb.left; k <= bb.right; ++k) {
		top[k] = color;
		bottom[k] = color;
	}
	// vt lines
	const size_t stride = mat->stride();
	for (top = top + stride; top < bottom; top += stride) {
		top[bb.left] = color;
		top[bb.right] = color;
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE compv_tests_draw_segments(CompVMatPtr mat, const CompVConnectedComponentPoints& ccl_segments, const uint8_t color)
{
	COMPV_CHECK_EXP_RETURN(!mat || mat->planeCount() != 1 || mat->elmtInBytes() != sizeof(uint8_t) || (ccl_segments.size() & 1), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	for (CompVConnectedComponentPoints::const_iterator i = ccl_segments.begin(); i < ccl_segments.end(); i += 2) {
		uint8_t* ptr = mat->ptr<uint8_t>(static_cast<size_t>(i->y));
		for (int16_t x = i->x; x <= i[1].x; ++x) {
			ptr[x] = color;
		}
	}
	return COMPV_ERROR_CODE_S_OK;
}

// Fused-Multiply-Add
bool compv_tests_is_fma_enabled()
{
	// On X86 FMA requires AVX2 (only asm)
	// On ARM FMA requires NEON (only asm) - When intrinsic code is built without "-mfpu=neon-vfpv4" then, the compiler generate very slooow code for vfma function.
#if COMPV_ARCH_X86
	return ((CompVCpu::isEnabled(kCpuFlagAVX2) || CompVCpu::isEnabled(kCpuFlagAVX)) && CompVCpu::isEnabled(kCpuFlagFMA3) && CompVCpu::isAsmEnabled());
#elif COMPV_ARCH_ARM
	return (CompVCpu::isEnabled(kCpuFlagARM_NEON) && CompVCpu::isEnabled(kCpuFlagARM_NEON_FMA) && CompVCpu::isAsmEnabled());
#endif
	return false;
}

// Reciprocal
bool compv_tests_is_rcp()
{
#if COMPV_ARCH_X64
	return (CompVCpu::isEnabled(kCpuFlagSSSE3) || CompVCpu::isEnabled(kCpuFlagAVX2)) && (CompVCpu::isAsmEnabled() || CompVCpu::isIntrinsicsEnabled());
#elif COMPV_ARCH_X86
	return (CompVCpu::isEnabled(kCpuFlagSSSE3) || CompVCpu::isEnabled(kCpuFlagAVX2)) && (0 || CompVCpu::isIntrinsicsEnabled()); // No rcp impl for ASM.X86_32
#elif COMPV_ARCH_ARM
	return (CompVCpu::isEnabled(kCpuFlagARM_NEON)) && (CompVCpu::isAsmEnabled() || CompVCpu::isIntrinsicsEnabled());
#endif
	return false;
}

COMPV_GCC_DISABLE_WARNINGS_END()

