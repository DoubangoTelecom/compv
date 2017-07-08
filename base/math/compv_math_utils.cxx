/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/compv_math_utils.h"
#include "compv/base/compv_cpu.h"

#include "compv/base/math/intrin/x86/compv_math_utils_intrin_sse2.h"
#include "compv/base/math/intrin/x86/compv_math_utils_intrin_ssse3.h"
#include "compv/base/math/intrin/x86/compv_math_utils_intrin_sse41.h"
#include "compv/base/math/intrin/x86/compv_math_utils_intrin_avx2.h"
#include "compv/base/math/intrin/arm/compv_math_utils_intrin_neon.h"

#include <algorithm>

COMPV_NAMESPACE_BEGIN()

#if COMPV_ASM
#	if COMPV_ARCH_X86
	COMPV_EXTERNC void CompVMathUtilsMax_16u_Asm_X86_SSE41(COMPV_ALIGNED(SSE) const uint16_t* data, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride, uint16_t *max);
	COMPV_EXTERNC void CompVMathUtilsSum_8u32u_Asm_X86_SSE2(COMPV_ALIGNED(SSE) const uint8_t* data, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride, uint32_t *sum1);
	COMPV_EXTERNC void CompVMathUtilsSumAbs_16s16u_Asm_X86_SSSE3(const COMPV_ALIGNED(SSE) int16_t* a, const COMPV_ALIGNED(SSE) int16_t* b, COMPV_ALIGNED(SSE) uint16_t* r, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
	COMPV_EXTERNC void CompVMathUtilsSumAbs_16s16u_Asm_X86_AVX2(const COMPV_ALIGNED(AVX) int16_t* a, const COMPV_ALIGNED(AVX) int16_t* b, COMPV_ALIGNED(AVX) uint16_t* r, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(AVX) compv_uscalar_t stride);
	COMPV_EXTERNC void CompVMathUtilsSum2_32s32s_Asm_X86_SSE2(COMPV_ALIGNED(SSE) const int32_t* a, COMPV_ALIGNED(SSE) const int32_t* b, COMPV_ALIGNED(SSE) int32_t* s, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
	COMPV_EXTERNC void CompVMathUtilsSum2_32s32s_256x1_Asm_X86_SSE2(COMPV_ALIGNED(SSE) const int32_t* a, COMPV_ALIGNED(SSE) const int32_t* b, COMPV_ALIGNED(SSE) int32_t* s, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
	COMPV_EXTERNC void CompVMathUtilsScaleAndClipPixel8_16u32f_Asm_X86_SSE2(COMPV_ALIGNED(SSE) const uint16_t* in, const compv_float32_t* scale1, COMPV_ALIGNED(SSE) uint8_t* out, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
#	endif /* COMPV_ARCH_X86 */
#	if COMPV_ARCH_X64
	COMPV_EXTERNC void CompVMathUtilsSum_8u32u_Asm_X64_SSE2(COMPV_ALIGNED(SSE) const uint8_t* data, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride, uint32_t *sum1);
	COMPV_EXTERNC void CompVMathUtilsSum2_32s32s_Asm_X64_SSE2(COMPV_ALIGNED(SSE) const int32_t* a, COMPV_ALIGNED(SSE) const int32_t* b, COMPV_ALIGNED(SSE) int32_t* s, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
#	endif /* COMPV_ARCH_X64 */
#	if COMPV_ARCH_ARM32
	COMPV_EXTERNC void CompVMathUtilsMax_16u_Asm_NEON32(COMPV_ALIGNED(NEON) const uint16_t* data, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride, uint16_t *max);
	COMPV_EXTERNC void CompVMathUtilsSum_8u32u_Asm_NEON32(COMPV_ALIGNED(NEON) const uint8_t* data, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride, uint32_t *sum1);
	COMPV_EXTERNC void CompVMathUtilsSumAbs_16s16u_Asm_NEON32(const COMPV_ALIGNED(NEON) int16_t* a, const COMPV_ALIGNED(NEON) int16_t* b, COMPV_ALIGNED(NEON) uint16_t* r, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
	COMPV_EXTERNC void CompVMathUtilsScaleAndClipPixel8_16u32f_Asm_NEON32(COMPV_ALIGNED(NEON) const uint16_t* in, const compv_float32_t* scale1, COMPV_ALIGNED(NEON) uint8_t* out, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
#   endif /* COMPV_ARCH_ARM32 */
#	if COMPV_ARCH_ARM64
    COMPV_EXTERNC void CompVMathUtilsMax_16u_Asm_NEON64(COMPV_ALIGNED(NEON) const uint16_t* data, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride, uint16_t *max);
    COMPV_EXTERNC void CompVMathUtilsSum_8u32u_Asm_NEON64(COMPV_ALIGNED(NEON) const uint8_t* data, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride, uint32_t *sum1);
    COMPV_EXTERNC void CompVMathUtilsSumAbs_16s16u_Asm_NEON64(const COMPV_ALIGNED(NEON) int16_t* a, const COMPV_ALIGNED(NEON) int16_t* b, COMPV_ALIGNED(NEON) uint16_t* r, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
    COMPV_EXTERNC void CompVMathUtilsScaleAndClipPixel8_16u32f_Asm_NEON64(COMPV_ALIGNED(NEON) const uint16_t* in, const compv_float32_t* scale1, COMPV_ALIGNED(NEON) uint8_t* out, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
#   endif /* COMPV_ARCH_ARM64 */
#endif /* COMPV_ASM */

bool CompVMathUtils::s_Initialized = false;

static compv_scalar_t maxVal_C(compv_scalar_t x, compv_scalar_t y);
static compv_scalar_t minVal_C(compv_scalar_t x, compv_scalar_t y);
static int32_t minArrayI32_C(const int32_t* array, compv_scalar_t count);
static compv_scalar_t clip3_C(compv_scalar_t min, compv_scalar_t max, compv_scalar_t val);
static compv_scalar_t clip2_C(compv_scalar_t max, compv_scalar_t val);
static void rand_C(uint32_t* r, compv_scalar_t count);
static int roundFloatUnsigned_C(float f);
static int roundFloatSigned_C(float f);

#if COMPV_ARCH_X86 && COMPV_ASM
COMPV_EXTERNC compv_scalar_t compv_mathutils_maxval_asm_x86_cmov(compv_scalar_t x, compv_scalar_t y);
COMPV_EXTERNC compv_scalar_t compv_mathutils_minval_asm_x86_cmov(compv_scalar_t x, compv_scalar_t y);
COMPV_EXTERNC compv_scalar_t compv_mathutils_clip3_asm_x86_cmov(compv_scalar_t min, compv_scalar_t max, compv_scalar_t val);
COMPV_EXTERNC compv_scalar_t compv_mathutils_clip2_asm_x86_cmov(compv_scalar_t max, compv_scalar_t val);
COMPV_EXTERNC void compv_mathutils_rand_asm_x86_rdrand(uint32_t* r, compv_scalar_t count);
#endif /* COMPV_ARCH_X86 && COMPV_ASM */

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
int roundFloat_Intrin_SSE2(float f);
#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */

compv_scalar_t(*CompVMathUtils::maxValFunc)(compv_scalar_t a, compv_scalar_t b) = maxVal_C;
compv_scalar_t(*CompVMathUtils::minValFunc)(compv_scalar_t a, compv_scalar_t b) = minVal_C;
int32_t(*CompVMathUtils::minArrayI32Func)(const int32_t* array, compv_scalar_t count) = minArrayI32_C;
compv_scalar_t(*CompVMathUtils::clip3Func)(compv_scalar_t min, compv_scalar_t max, compv_scalar_t val) = clip3_C;
compv_scalar_t(*CompVMathUtils::clip2Func)(compv_scalar_t max, compv_scalar_t val) = clip2_C;
void(*CompVMathUtils::randFunc)(uint32_t* r, compv_scalar_t count) = rand_C;
int(*CompVMathUtils::roundFloatUnsignedFunc)(float f) = roundFloatUnsigned_C;
int(*CompVMathUtils::roundFloatSignedFunc)(float f) = roundFloatSigned_C;

COMPV_ERROR_CODE CompVMathUtils::init()
{
    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;

    if (!CompVMathUtils::s_Initialized) {
        if (CompVCpu::isEnabled(kCpuFlagCMOV)) {
            COMPV_EXEC_IFDEF_ASM_X86(CompVMathUtils::maxValFunc = compv_mathutils_maxval_asm_x86_cmov);
            COMPV_EXEC_IFDEF_ASM_X86(CompVMathUtils::minValFunc = compv_mathutils_minval_asm_x86_cmov);
            COMPV_EXEC_IFDEF_ASM_X86(CompVMathUtils::clip3Func = compv_mathutils_clip3_asm_x86_cmov);
            COMPV_EXEC_IFDEF_ASM_X86(CompVMathUtils::clip2Func = compv_mathutils_clip2_asm_x86_cmov);
        }
        if (CompVCpu::isEnabled(kCpuFlagSSE)) {
            COMPV_EXEC_IFDEF_INTRIN_X86(roundFloatUnsignedFunc = roundFloat_Intrin_SSE2);
            COMPV_EXEC_IFDEF_INTRIN_X86(roundFloatSignedFunc = roundFloat_Intrin_SSE2);
        }
#if 0
        // RDRAND isn't a PRNG but a TRNG and is slower than ANSI's rand()
        // https://software.intel.com/en-us/articles/intel-digital-random-number-generator-drng-software-implementation-guide/
        if (CompVCpu::isEnabled(kCpuFlagRDRAND)) {
            COMPV_EXEC_IFDEF_ASM_X86(randFunc = compv_mathutils_rand_asm_x86_rdrand);
        }
#endif
        CompVMathUtils::s_Initialized = true;
    }

    return err_;
}

static compv_scalar_t maxVal_C(compv_scalar_t x, compv_scalar_t y)
{
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
    return std::max(x, y);
}

static compv_scalar_t minVal_C(compv_scalar_t x, compv_scalar_t y)
{
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
    return std::min(x, y);
}

static int32_t minArrayI32_C(const int32_t* array, compv_scalar_t count)
{
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
    int32_t min = array[0];
    for (compv_scalar_t i = 1; i < count; ++i) {
        min = std::min(min, array[i]);
    }
    return min;
}

compv_scalar_t clip3_C(compv_scalar_t min, compv_scalar_t max, compv_scalar_t val)
{
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
    return CompVMathUtils::maxVal(min, CompVMathUtils::minVal(val, max));
}

compv_scalar_t clip2_C(compv_scalar_t max, compv_scalar_t val)
{
    return clip3_C(0, max, val);
}

void rand_C(uint32_t* r, compv_scalar_t count)
{
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
    compv_scalar_t i = 0;
    for (i = 0; i < count - 3; i += 4) {
        r[i] = static_cast<uint32_t>(rand());
        r[i + 1] = static_cast<uint32_t>(rand());
        r[i + 2] = static_cast<uint32_t>(rand());
        r[i + 3] = static_cast<uint32_t>(rand());
    }
    for (; i < count; i += 1) {
        r[i] = static_cast<uint32_t>(rand());
    }
}

int roundFloatUnsigned_C(float f)
{
    return COMPV_MATH_ROUNDFU_2_NEAREST_INT(f, int);
}

int roundFloatSigned_C(float f)
{
    return COMPV_MATH_ROUNDF_2_NEAREST_INT(f, int);
}

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
int roundFloat_Intrin_SSE2(float f)
{
    return _mm_cvt_ss2si(_mm_set_ss(f));
}
#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */

template <> COMPV_BASE_API
COMPV_ERROR_CODE CompVMathUtils::max(const uint16_t* data, size_t width, size_t height, size_t stride, uint16_t &max)
{
	COMPV_CHECK_EXP_RETURN(!data || !width || !height || stride < width, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	void(*CompVMathUtilsMax_16u)(COMPV_ALIGNED(X) const uint16_t* data, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride, uint16_t *max)
		= NULL;
	const size_t strideInBytes = stride * sizeof(uint16_t);
#if COMPV_ARCH_X86 
	if (COMPV_IS_ALIGNED_SSE(strideInBytes) && COMPV_IS_ALIGNED_SSE(data)) {
		if (CompVCpu::isEnabled(kCpuFlagSSE2)) {
			COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathUtilsMax_16u = CompVMathUtilsMax_16u_Intrin_SSE2);
		}
		if (CompVCpu::isEnabled(kCpuFlagSSE41)) {
			COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathUtilsMax_16u = CompVMathUtilsMax_16u_Intrin_SSE41);
			COMPV_EXEC_IFDEF_ASM_X86(CompVMathUtilsMax_16u = CompVMathUtilsMax_16u_Asm_X86_SSE41);
		}
	}
#elif COMPV_ARCH_ARM
	if (COMPV_IS_ALIGNED_NEON(strideInBytes) && COMPV_IS_ALIGNED_NEON(data)) {
		if (CompVCpu::isEnabled(kCpuFlagARM_NEON)) {
			COMPV_EXEC_IFDEF_INTRIN_ARM(CompVMathUtilsMax_16u = CompVMathUtilsMax_16u_Intrin_NEON);
            COMPV_EXEC_IFDEF_ASM_ARM32(CompVMathUtilsMax_16u = CompVMathUtilsMax_16u_Asm_NEON32);
            COMPV_EXEC_IFDEF_ASM_ARM64(CompVMathUtilsMax_16u = CompVMathUtilsMax_16u_Asm_NEON64);
		}
	}
#endif
	if (CompVMathUtilsMax_16u) {
		CompVMathUtilsMax_16u(data, static_cast<compv_uscalar_t>(width), static_cast<compv_uscalar_t>(height), static_cast<compv_uscalar_t>(stride), &max);
		return COMPV_ERROR_CODE_S_OK;
	}
	COMPV_CHECK_CODE_RETURN((CompVMathUtils::max_C<uint16_t>(data, width, height, stride, max)));
	return COMPV_ERROR_CODE_S_OK;
}

template <> COMPV_BASE_API
COMPV_ERROR_CODE CompVMathUtils::sumAbs(const int16_t* a, const int16_t* b, uint16_t* r, size_t width, size_t height, size_t stride)
{
    COMPV_CHECK_EXP_RETURN(!a || !b || !r || !width || !height || stride < width, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	void(*CompVMathUtilsSumAbs_16s16u)(const COMPV_ALIGNED(X) int16_t* a, const COMPV_ALIGNED(X) int16_t* b, COMPV_ALIGNED(X) uint16_t* r, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(X) compv_uscalar_t stride) 
		= NULL;
	const size_t strideInBytes = stride * sizeof(int16_t);
#if COMPV_ARCH_X86
    if (COMPV_IS_ALIGNED_SSE(strideInBytes) && COMPV_IS_ALIGNED_SSE(a) && COMPV_IS_ALIGNED_SSE(b) && COMPV_IS_ALIGNED_SSE(r)) {
        if (CompVCpu::isEnabled(kCpuFlagSSSE3)) {
            COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathUtilsSumAbs_16s16u = CompVMathUtilsSumAbs_16s16u_Intrin_SSSE3);
            COMPV_EXEC_IFDEF_ASM_X86(CompVMathUtilsSumAbs_16s16u = CompVMathUtilsSumAbs_16s16u_Asm_X86_SSSE3);
        }
    }
    if (width >= 64 && COMPV_IS_ALIGNED_AVX(strideInBytes) && COMPV_IS_ALIGNED_AVX(a) && COMPV_IS_ALIGNED_AVX(b) && COMPV_IS_ALIGNED_AVX(r)) {
        if (CompVCpu::isEnabled(kCpuFlagAVX2)) {
            COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathUtilsSumAbs_16s16u = CompVMathUtilsSumAbs_16s16u_Intrin_AVX2);
            COMPV_EXEC_IFDEF_ASM_X86(CompVMathUtilsSumAbs_16s16u = CompVMathUtilsSumAbs_16s16u_Asm_X86_AVX2);
        }
    }
#elif COMPV_ARCH_ARM
	if (COMPV_IS_ALIGNED_NEON(strideInBytes) && COMPV_IS_ALIGNED_NEON(a) && COMPV_IS_ALIGNED_NEON(b) && COMPV_IS_ALIGNED_NEON(r)) {
		if (CompVCpu::isEnabled(kCpuFlagARM_NEON)) {
			COMPV_EXEC_IFDEF_INTRIN_ARM(CompVMathUtilsSumAbs_16s16u = CompVMathUtilsSumAbs_16s16u_Intrin_NEON);
            COMPV_EXEC_IFDEF_ASM_ARM32(CompVMathUtilsSumAbs_16s16u = CompVMathUtilsSumAbs_16s16u_Asm_NEON32);
            COMPV_EXEC_IFDEF_ASM_ARM64(CompVMathUtilsSumAbs_16s16u = CompVMathUtilsSumAbs_16s16u_Asm_NEON64);
		}
	}
#endif
	if (CompVMathUtilsSumAbs_16s16u) {
		CompVMathUtilsSumAbs_16s16u(reinterpret_cast<const int16_t*>(a), reinterpret_cast<const int16_t*>(b), reinterpret_cast<uint16_t*>(r), static_cast<compv_uscalar_t>(width), static_cast<compv_uscalar_t>(height), static_cast<compv_uscalar_t>(stride));
		return COMPV_ERROR_CODE_S_OK;
	}
    COMPV_CHECK_CODE_RETURN((CompVMathUtils::sumAbs_C<int16_t, uint16_t>(a, b, r, width, height, stride)));
    return COMPV_ERROR_CODE_S_OK;
}

template <> COMPV_BASE_API
COMPV_ERROR_CODE CompVMathUtils::sum(const uint8_t* a, size_t width, size_t height, size_t stride, uint32_t &r)
{
	void(*CompVMathUtilsSum_8u32u)(COMPV_ALIGNED(X) const uint8_t* a, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(X) compv_uscalar_t stride, uint32_t *sum1)
		= NULL;
#if COMPV_ARCH_X86
	// TODO(dmi): add AVX implementation (not urgent, SSE2 version is already insanely fast)
    if (width >= 16 && COMPV_IS_ALIGNED_SSE(a) && COMPV_IS_ALIGNED_SSE(stride)) {
        if (CompVCpu::isEnabled(kCpuFlagSSE2)) {
            COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathUtilsSum_8u32u = CompVMathUtilsSum_8u32u_Intrin_SSE2);
            COMPV_EXEC_IFDEF_ASM_X86(CompVMathUtilsSum_8u32u = CompVMathUtilsSum_8u32u_Asm_X86_SSE2);
			COMPV_EXEC_IFDEF_ASM_X64(CompVMathUtilsSum_8u32u = CompVMathUtilsSum_8u32u_Asm_X64_SSE2);
        }
    }
#elif COMPV_ARCH_ARM
	if (width >= 16 && COMPV_IS_ALIGNED_NEON(a) && COMPV_IS_ALIGNED_NEON(stride)) {
		if (CompVCpu::isEnabled(kCpuFlagARM_NEON)) {
			COMPV_EXEC_IFDEF_INTRIN_ARM(CompVMathUtilsSum_8u32u = CompVMathUtilsSum_8u32u_Intrin_NEON);
            COMPV_EXEC_IFDEF_ASM_ARM32(CompVMathUtilsSum_8u32u = CompVMathUtilsSum_8u32u_Asm_NEON32);
            COMPV_EXEC_IFDEF_ASM_ARM64(CompVMathUtilsSum_8u32u = CompVMathUtilsSum_8u32u_Asm_NEON64);
		}
	}
#endif
	if (CompVMathUtilsSum_8u32u) {
		CompVMathUtilsSum_8u32u(a, static_cast<compv_uscalar_t>(width), static_cast<compv_uscalar_t>(height), static_cast<compv_uscalar_t>(stride), &r);
		return COMPV_ERROR_CODE_S_OK;
	}
    COMPV_CHECK_CODE_RETURN((CompVMathUtils::sum_C<uint8_t>(a, width, height, stride, r)));
    return COMPV_ERROR_CODE_S_OK;
}

template<> COMPV_BASE_API
COMPV_ERROR_CODE CompVMathUtils::sum2(const int32_t* a, const int32_t* b, int32_t* s, size_t width, size_t height, size_t stride)
{
	void(*CompVMathUtilsSum2_32s32s)(COMPV_ALIGNED(X) const int32_t* a, COMPV_ALIGNED(X) const int32_t* b, COMPV_ALIGNED(X) int32_t* s, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(X) compv_uscalar_t stride) 
		= NULL;
	bool have_fast256x1 = false;
	const bool need_fast256x1 = (width == 256 && height == 1);
#if COMPV_ARCH_X86
    if (width >= 4 && CompVCpu::isEnabled(kCpuFlagSSE2) && COMPV_IS_ALIGNED_SSE(a) && COMPV_IS_ALIGNED_SSE(b) && COMPV_IS_ALIGNED_SSE(stride*sizeof(int32_t))) {
        COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathUtilsSum2_32s32s = CompVMathUtilsSum2_32s32s_Intrin_SSE2);
        COMPV_EXEC_IFDEF_ASM_X86(CompVMathUtilsSum2_32s32s = CompVMathUtilsSum2_32s32s_Asm_X86_SSE2);
		COMPV_EXEC_IFDEF_ASM_X64(CompVMathUtilsSum2_32s32s = CompVMathUtilsSum2_32s32s_Asm_X64_SSE2);
		if (need_fast256x1) {
			COMPV_EXEC_IFDEF_INTRIN_X86((CompVMathUtilsSum2_32s32s = CompVMathUtilsSum2_32s32s_256x1_Intrin_SSE2, have_fast256x1 = true));
			COMPV_EXEC_IFDEF_ASM_X86((CompVMathUtilsSum2_32s32s = CompVMathUtilsSum2_32s32s_256x1_Asm_X86_SSE2, have_fast256x1 = true));
		}
    }
#elif COMPV_ARCH_ARM
	if (width >= 4 && CompVCpu::isEnabled(kCpuFlagARM_NEON) && COMPV_IS_ALIGNED_NEON(a) && COMPV_IS_ALIGNED_NEON(b) && COMPV_IS_ALIGNED_NEON(stride * sizeof(int32_t))) {
		if (width == 256 && height == 1) {
			COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation for common size (histogram 8u)");
		}
	}
#endif
	if (need_fast256x1 && !have_fast256x1) {
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation for common size (histogram 8u)");
	}
	if (CompVMathUtilsSum2_32s32s) {
		CompVMathUtilsSum2_32s32s(a, b, s, static_cast<compv_uscalar_t>(width), static_cast<compv_uscalar_t>(height), static_cast<compv_uscalar_t>(stride));
		return COMPV_ERROR_CODE_S_OK;
	}
    COMPV_CHECK_CODE_RETURN((CompVMathUtils::sum2_C<int32_t, int32_t>(a, b, s, width, height, stride)));
    return COMPV_ERROR_CODE_S_OK;
}

template <> COMPV_BASE_API 
COMPV_ERROR_CODE CompVMathUtils::mean(const uint8_t* data, size_t count, uint8_t &mean)
{
    uint32_t r;
	COMPV_CHECK_CODE_RETURN((CompVMathUtils::sum<uint8_t, uint32_t>(data, count, 1, static_cast<size_t>(CompVMem::alignForward(count)), r))); // aligning stride (read-only data)
    mean = (uint8_t)(r / count);
    return COMPV_ERROR_CODE_S_OK;
}

template <> COMPV_BASE_API
COMPV_ERROR_CODE CompVMathUtils::scaleAndClip(const uint16_t* in, const compv_float32_t scale, uint8_t*& out, uint8_t min, uint8_t max, size_t width, size_t height, size_t stride)
{
	COMPV_CHECK_EXP_RETURN(!in || !width || !height || stride < width || max < min, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	if (!out) {
		out = reinterpret_cast<uint8_t*>(CompVMem::malloc(height * stride * sizeof(uint8_t)));
		COMPV_CHECK_EXP_RETURN(!out, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	}
	void (*CompVMathUtilsScaleAndClipPixel8_16u32f)(COMPV_ALIGNED(X) const uint16_t* in, const compv_float32_t* scale1, COMPV_ALIGNED(X) uint8_t* out, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(X) compv_uscalar_t stride)
		= NULL; // requires min = 0 and max = 255
#if COMPV_ARCH_X86
	if (min == 0 && max == 255 && CompVCpu::isEnabled(kCpuFlagSSE2) && COMPV_IS_ALIGNED_SSE(in) && COMPV_IS_ALIGNED_SSE(out) && COMPV_IS_ALIGNED_SSE(stride * sizeof(uint16_t)) && COMPV_IS_ALIGNED_SSE(stride * sizeof(uint8_t))) {
		COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathUtilsScaleAndClipPixel8_16u32f = CompVMathUtilsScaleAndClipPixel8_16u32f_Intrin_SSE2);
		COMPV_EXEC_IFDEF_ASM_X86(CompVMathUtilsScaleAndClipPixel8_16u32f = CompVMathUtilsScaleAndClipPixel8_16u32f_Asm_X86_SSE2);
	}
#elif COMPV_ARCH_ARM
	if (min == 0 && max == 255 && CompVCpu::isEnabled(kCpuFlagARM_NEON) && COMPV_IS_ALIGNED_NEON(in) && COMPV_IS_ALIGNED_NEON(out) && COMPV_IS_ALIGNED_NEON(stride * sizeof(uint16_t)) && COMPV_IS_ALIGNED_NEON(stride * sizeof(uint8_t))) {
		COMPV_EXEC_IFDEF_INTRIN_ARM(CompVMathUtilsScaleAndClipPixel8_16u32f = CompVMathUtilsScaleAndClipPixel8_16u32f_Intrin_NEON);
        COMPV_EXEC_IFDEF_ASM_ARM32(CompVMathUtilsScaleAndClipPixel8_16u32f = CompVMathUtilsScaleAndClipPixel8_16u32f_Asm_NEON32);
        COMPV_EXEC_IFDEF_ASM_ARM64(CompVMathUtilsScaleAndClipPixel8_16u32f = CompVMathUtilsScaleAndClipPixel8_16u32f_Asm_NEON64);
	}
#endif
	if (CompVMathUtilsScaleAndClipPixel8_16u32f) {
		CompVMathUtilsScaleAndClipPixel8_16u32f(in, &scale, out, static_cast<compv_uscalar_t>(width), static_cast<compv_uscalar_t>(height), static_cast<compv_uscalar_t>(stride));
		return COMPV_ERROR_CODE_S_OK;
	}

	COMPV_CHECK_CODE_RETURN((CompVMathUtils::scaleAndClip_C<uint16_t, compv_float32_t, uint8_t>(in, scale, out, min, max, width, height, stride)));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
