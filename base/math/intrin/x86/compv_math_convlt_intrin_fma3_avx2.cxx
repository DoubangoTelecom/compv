/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/x86/compv_math_convlt_intrin_fma3_avx2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/intrin/x86/compv_intrin_avx.h"
#include "compv/base/compv_simd_globals.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx2
#endif
void CompVMathConvlt1VtHz_8u32f8u_Intrin_FMA3_AVX2(const uint8_t* inPtr, uint8_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const compv_float32_t* vthzKernPtr, compv_uscalar_t kernSize)
{
	COMPV_DEBUG_INFO_CHECK_AVX2(); // AVX/SSE transition issues
	COMPV_DEBUG_INFO_CHECK_FMA3();
	_mm256_zeroupper();
	compv_uscalar_t i, j, k, row, stride = width + pad;
	__m256i vecInPtr, vec0i, vec1i, vec2i, vec3i;
	__m256 vecCoeff, vecSum0, vecSum1, vecSum2, vecSum3, vec0f, vec1f, vec2f, vec3f;
	const __m256i vecZero = _mm256_setzero_si256();
	const __m256i vecMaskToExtractFirst64Bits = _mm256_load_si256(reinterpret_cast<const __m256i*>(kAVXMaskstore_0_u64));
	__m128 vecSum0n, vecCoeffn, vec0fn;

	for (j = 0; j < height; ++j) {
		/* Per #32 samples */
		for (i = 0; i < width - 31; i += 32) {
			vecSum0 = _mm256_setzero_ps();
			vecSum1 = _mm256_setzero_ps();
			vecSum2 = _mm256_setzero_ps();
			vecSum3 = _mm256_setzero_ps();
			for (row = 0, k = 0; row < kernSize; ++row, k += step) {
				vecInPtr = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&inPtr[i + k]));
				vecCoeff = _mm256_set1_ps(vthzKernPtr[row]);
				vec2i = _mm256_unpacklo_epi8(vecInPtr, vecZero); // epi8 -> epi16
				vec3i = _mm256_unpackhi_epi8(vecInPtr, vecZero); // epi8 -> epi16
				vec0i = _mm256_unpacklo_epi16(vec2i, vecZero); // epi16 -> epi32
				vec1i = _mm256_unpackhi_epi16(vec2i, vecZero); // epi16 -> epi32
				vec2i = _mm256_unpacklo_epi16(vec3i, vecZero); // epi16 -> epi32
				vec3i = _mm256_unpackhi_epi16(vec3i, vecZero); // epi16 -> epi32
				vec0f = _mm256_cvtepi32_ps(vec0i);
				vec1f = _mm256_cvtepi32_ps(vec1i);
				vec2f = _mm256_cvtepi32_ps(vec2i);
				vec3f = _mm256_cvtepi32_ps(vec3i);
				vecSum0 = _mm256_fmadd_ps(vec0f, vecCoeff, vecSum0);
				vecSum1 = _mm256_fmadd_ps(vec1f, vecCoeff, vecSum1);
				vecSum2 = _mm256_fmadd_ps(vec2f, vecCoeff, vecSum2);
				vecSum3 = _mm256_fmadd_ps(vec3f, vecCoeff, vecSum3);
			}
			vec0i = _mm256_cvttps_epi32(vecSum0);
			vec1i = _mm256_cvttps_epi32(vecSum1);
			vec2i = _mm256_cvttps_epi32(vecSum2);
			vec3i = _mm256_cvttps_epi32(vecSum3);
			vec0i = _mm256_packs_epi32(vec0i, vec1i); // _mm256_packus_epi32 is SSE4.1
			vec2i = _mm256_packs_epi32(vec2i, vec3i);
			vec0i = _mm256_packus_epi16(vec0i, vec2i);
			_mm256_storeu_si256(reinterpret_cast<__m256i*>(&outPtr[i]), vec0i);
		}

		/* Per #8 samples */
		for (; i < width - 7; i += 8) {
			vecSum0 = _mm256_setzero_ps();
			for (row = 0, k = 0; row < kernSize; ++row, k += step) {
				vecInPtr = _mm256_cvtepu8_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&inPtr[i + k])));
				vecCoeff = _mm256_set1_ps(vthzKernPtr[row]);
				vec0f = _mm256_cvtepi32_ps(vecInPtr);
				vecSum0 = _mm256_fmadd_ps(vec0f, vecCoeff, vecSum0);
			}
			vec0i = _mm256_cvttps_epi32(vecSum0);
			vec0i = _mm256_packs_epi32(vec0i, vec0i);
			vec0i = _mm256_permute4x64_epi64(vec0i, 0xD8);
			vec0i = _mm256_packus_epi16(vec0i, vec0i);
			_mm256_maskstore_epi64(reinterpret_cast<int64_t*>(&outPtr[i]), vecMaskToExtractFirst64Bits, vec0i); // ASM code: movq [mem], xmm0
		}

		/* Per #1 samples */
		for (; i < width; i += 1) {
			vecSum0n = _mm_setzero_ps();
			for (row = 0, k = 0; row < kernSize; ++row, k += step) {
				vecCoeffn = _mm_load_ss(&vthzKernPtr[row]);
				vec0fn = _mm_cvtsi32_ss(vec0fn, static_cast<int>(inPtr[i + k]));
				vecSum0n = _mm_fmadd_ss(vec0fn, vecCoeffn, vecSum0n);
			}
			outPtr[i] = static_cast<uint8_t>(_mm_cvtt_ss2si(vecSum0n) & 0xff);
		}

		inPtr += stride;
		outPtr += stride;
	}
	_mm256_zeroupper();
}


COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
