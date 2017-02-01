/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/x86/compv_math_convlt_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/math/compv_math_utils.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// no arithmetic overflow check
void CompVMathConvlt1VtHz_8u32f8u_Intrin_SSE2(COMPV_ALIGNED(SSE) const uint8_t* inPtr, uint8_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const compv_float32_t* vthzKernPtr, compv_uscalar_t kernSize)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	compv_uscalar_t i, j, k, row, stride = width + pad;
	__m128i vecInPtr, vec0i, vec1i, vec2i, vec3i;
	__m128 vecCoeff, vecSum0, vecSum1, vecSum2, vecSum3, vec0f, vec1f, vec2f, vec3f;
	const __m128i vecZero = _mm_setzero_si128();

	for (j = 0; j < height; ++j) {
		/* Per #16 bytes */
		for (i = 0; i < width - 15; i += 16) {
			vecSum0 = _mm_setzero_ps();
			vecSum1 = _mm_setzero_ps();
			vecSum2 = _mm_setzero_ps();
			vecSum3 = _mm_setzero_ps();
			for (row = 0, k = 0; row < kernSize; ++row, k += step) {
				vecInPtr = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&inPtr[i + k]));
				vecCoeff = _mm_set1_ps(vthzKernPtr[row]);
				vec2i = _mm_unpacklo_epi8(vecInPtr, vecZero); // epi8 -> epi16
				vec3i = _mm_unpackhi_epi8(vecInPtr, vecZero); // epi8 -> epi16
				vec0i = _mm_unpacklo_epi16(vec2i, vecZero); // epi16 -> epi32
				vec1i = _mm_unpackhi_epi16(vec2i, vecZero); // epi16 -> epi32
				vec2i = _mm_unpacklo_epi16(vec3i, vecZero); // epi16 -> epi32
				vec3i = _mm_unpackhi_epi16(vec3i, vecZero); // epi16 -> epi32
				vec0f = _mm_cvtepi32_ps(vec0i);
				vec1f = _mm_cvtepi32_ps(vec1i);
				vec2f = _mm_cvtepi32_ps(vec2i);
				vec3f = _mm_cvtepi32_ps(vec3i);
				// TODO(dmi): AVX use FMA if available
				vec0f = _mm_mul_ps(vec0f, vecCoeff);
				vec1f = _mm_mul_ps(vec1f, vecCoeff);
				vec2f = _mm_mul_ps(vec2f, vecCoeff);
				vec3f = _mm_mul_ps(vec3f, vecCoeff);
				vecSum0 = _mm_add_ps(vecSum0, vec0f);
				vecSum1 = _mm_add_ps(vecSum1, vec1f);
				vecSum2 = _mm_add_ps(vecSum2, vec2f);
				vecSum3 = _mm_add_ps(vecSum3, vec3f);
			}
			vec0i = _mm_cvttps_epi32(vecSum0);
			vec1i = _mm_cvttps_epi32(vecSum1);
			vec2i = _mm_cvttps_epi32(vecSum2);
			vec3i = _mm_cvttps_epi32(vecSum3);
			vec0i = _mm_packs_epi32(vec0i, vec1i); // _mm_packus_epi32 is SSE4.1
			vec2i = _mm_packs_epi32(vec2i, vec3i);
			vec0i = _mm_packus_epi16(vec0i, vec2i);
			_mm_storeu_si128(reinterpret_cast<__m128i*>(&outPtr[i]), vec0i);
		}
		
		/* Per #4 bytes */
		for (; i < width - 3; i += 4) {
			vecSum0 = _mm_setzero_ps();
			for (row = 0, k = 0; row < kernSize; ++row, k += step) {
				vecInPtr = _mm_cvtsi32_si128(*reinterpret_cast<const uint32_t*>(&inPtr[i + k]));
				vecCoeff = _mm_set1_ps(vthzKernPtr[row]);
				vec0f = _mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_unpacklo_epi8(vecInPtr, vecZero), vecZero));
				vec0f = _mm_mul_ps(vec0f, vecCoeff);
				vecSum0 = _mm_add_ps(vecSum0, vec0f);
			}
			vec0i = _mm_cvttps_epi32(vecSum0);
			vec0i = _mm_packs_epi32(vec0i, vec0i);
			vec0i = _mm_packus_epi16(vec0i, vec0i);
			*reinterpret_cast<uint32_t*>(&outPtr[i]) = static_cast<uint32_t>(_mm_cvtsi128_si32(vec0i));
		}
		
		/* Per #1 bytes */
		for (; i < width; i += 1) {
			vecSum0 = _mm_setzero_ps();
			for (row = 0, k = 0; row < kernSize; ++row, k += step) {
				vecCoeff = _mm_load_ss(&vthzKernPtr[row]);
				vec0f = _mm_cvtsi32_ss(vec0f, static_cast<int>(inPtr[i + k]));
				vec0f = _mm_mul_ss(vec0f, vecCoeff);
				vecSum0 = _mm_add_ss(vecSum0, vec0f);
			}
			outPtr[i] = static_cast<uint8_t>(_mm_cvtt_ss2si(vecSum0) & 0xff);
		}

		inPtr += stride;
		outPtr += stride;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */