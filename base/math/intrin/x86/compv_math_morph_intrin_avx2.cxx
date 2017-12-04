/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/x86/compv_math_morph_intrin_avx2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_simd_globals.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

#define CompVMathMorphProcessOp_8u_Intrin_AVX2(op, strelInputPtrsPtr, strelInputPtrsCount, outPtr, width, height, stride) { \
	COMPV_DEBUG_INFO_CHECK_AVX2(); \
	_mm256_zeroupper(); \
	compv_uscalar_t i, j, k, v; \
	const compv_uscalar_t width128 = width & -128; \
	const compv_uscalar_t width32 = width & -32; \
	const compv_uscalar_t strelInputPtrsPad = (stride - ((width + 31) & -32)); \
	__m256i vec0, vec1, vec2, vec3; \
	COMPV_ALIGN_AVX() uint8_t mem[32]; \
	 \
	for (j = 0, k = 0; j < height; ++j) { \
		for (i = 0; i < width128; i += 128, k += 128) { \
			vec0 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(k + strelInputPtrsPtr[0])); \
			vec1 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(k + strelInputPtrsPtr[0]) + 1); \
			vec2 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(k + strelInputPtrsPtr[0]) + 2); \
			vec3 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(k + strelInputPtrsPtr[0]) + 3); \
			for (v = 1; v < strelInputPtrsCount; ++v) { \
				vec0 = _mm256_##op##_epu8(vec0, _mm256_loadu_si256(reinterpret_cast<const __m256i*>(k + strelInputPtrsPtr[v]))); \
				vec1 = _mm256_##op##_epu8(vec1, _mm256_loadu_si256(reinterpret_cast<const __m256i*>(k + strelInputPtrsPtr[v]) + 1)); \
				vec2 = _mm256_##op##_epu8(vec2, _mm256_loadu_si256(reinterpret_cast<const __m256i*>(k + strelInputPtrsPtr[v]) + 2)); \
				vec3 = _mm256_##op##_epu8(vec3, _mm256_loadu_si256(reinterpret_cast<const __m256i*>(k + strelInputPtrsPtr[v]) + 3)); \
			} \
			_mm256_storeu_si256(reinterpret_cast<__m256i*>(&outPtr[i]), vec0); \
			_mm256_storeu_si256(reinterpret_cast<__m256i*>(&outPtr[i]) + 1, vec1); \
			_mm256_storeu_si256(reinterpret_cast<__m256i*>(&outPtr[i]) + 2, vec2); \
			_mm256_storeu_si256(reinterpret_cast<__m256i*>(&outPtr[i]) + 3, vec3); \
		} \
		for (; i < width; i += 32, k += 32) { \
			vec0 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(k + strelInputPtrsPtr[0])); \
			for (v = 1; v < strelInputPtrsCount; ++v) { \
				vec0 = _mm256_##op##_epu8(vec0, _mm256_loadu_si256(reinterpret_cast<const __m256i*>(k + strelInputPtrsPtr[v]))); \
			} \
			if (i < width32) { \
				_mm256_storeu_si256(reinterpret_cast<__m256i*>(&outPtr[i]), vec0); \
			} \
			else { \
				_mm256_store_si256(reinterpret_cast<__m256i*>(mem), vec0); \
				for (v = 0; i < width; ++i, ++v) { \
					outPtr[i] = mem[v]; \
				} \
			} \
		} \
		outPtr += stride; \
		k += strelInputPtrsPad; \
	} \
	_mm256_zeroupper(); \
}

#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx2
#endif
void CompVMathMorphProcessErode_8u_Intrin_AVX2(const compv_uscalar_t* strelInputPtrsPtr, const compv_uscalar_t strelInputPtrsCount, uint8_t* outPtr, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride)
{
	CompVMathMorphProcessOp_8u_Intrin_AVX2(min, strelInputPtrsPtr, strelInputPtrsCount, outPtr, width, height, stride);
}

#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx2
#endif
void CompVMathMorphProcessDilate_8u_Intrin_AVX2(const compv_uscalar_t* strelInputPtrsPtr, const compv_uscalar_t strelInputPtrsCount, uint8_t* outPtr, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride)
{
	CompVMathMorphProcessOp_8u_Intrin_AVX2(max, strelInputPtrsPtr, strelInputPtrsCount, outPtr, width, height, stride);
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
