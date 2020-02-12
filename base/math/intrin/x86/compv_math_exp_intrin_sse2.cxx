/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/x86/compv_math_exp_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// Must not require memory alignment (random access from SVM)
void CompVMathExpExp_minpack2_64f64f_Intrin_SSE2(const compv_float64_t* ptrIn, compv_float64_t* ptrOut, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride, const uint64_t* lut64u, const uint64_t* var64u, const compv_float64_t* var64f)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	COMPV_DEBUG_INFO_CODE_TODO("Add ASM implemenation");

	const __m128i vecMask = _mm_set1_epi64x(var64u[0]);
	const __m128i vecCADJ = _mm_set1_epi64x(var64u[1]);

	const __m128d vecB = _mm_set1_pd(var64f[0]);
	const __m128d vecCA = _mm_set1_pd(var64f[1]);
	const __m128d vecCRA = _mm_set1_pd(var64f[2]);
	const __m128d vecC10 = _mm_set1_pd(var64f[3]);
	const __m128d vecC20 = _mm_set1_pd(var64f[4]);
	const __m128d vecC30 = _mm_set1_pd(var64f[5]);
	const __m128d vecMin = _mm_set1_pd(var64f[6]);
	const __m128d vecMax = _mm_set1_pd(var64f[7]);

	const compv_uscalar_t width2 = width & -2;

	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width2; i += 2) {
			__m128d vecX = _mm_min_pd(_mm_loadu_pd(&ptrIn[i]), vecMax);
			vecX = _mm_max_pd(vecX, vecMin);
			__m128d vecDI = _mm_add_pd(_mm_mul_pd(vecX, vecCA), vecB); // TODO(dmi): add FMA implementation
			const __m128d vecT = _mm_sub_pd(_mm_mul_pd(_mm_sub_pd(vecDI, vecB), vecCRA), vecX); // TODO(dmi): add FMA implementation
			__m128i vecU = _mm_slli_epi64(_mm_srli_epi64(_mm_add_epi64(_mm_castpd_si128(vecDI), vecCADJ), 11), 52);
			__m128d vecY = _mm_mul_pd(vecT, vecT);
			vecDI = _mm_castsi128_pd(_mm_and_si128(_mm_castpd_si128(vecDI), vecMask));
			vecY = _mm_mul_pd(vecY,  _mm_sub_pd(vecC30, vecT));
			// TODO(dmi): use gather for avx to compute "vecLUT"
#if COMPV_ARCH_X64
			const int64_t i0 = _mm_cvtsi128_si64(_mm_castpd_si128(vecDI));
			const int64_t i1 = _mm_cvtsi128_si64(_mm_srli_si128(_mm_castpd_si128(vecDI), 8));
#else
			const int32_t i0 = _mm_cvtsi128_si32(_mm_castpd_si128(vecDI));
			const int32_t i1 = _mm_cvtsi128_si32(_mm_srli_si128(_mm_castpd_si128(vecDI), 8));
#endif
#if ((defined(_DEBUG) && _DEBUG != 0) || (defined(DEBUG) && DEBUG != 0))
			COMPV_ASSERT(i0 >= 0 && i0 < 2048 && i1 >= 0 && i1 < 2048);
#endif
			const __m128i vecLUT = _mm_unpacklo_epi64(
				_mm_loadl_epi64(reinterpret_cast<const __m128i*>(&lut64u[i0])), _mm_loadl_epi64(reinterpret_cast<const __m128i*>(&lut64u[i1]))
			);
			vecU = _mm_or_si128(vecU, vecLUT);

			vecY = _mm_sub_pd(_mm_mul_pd(vecY, vecC20), vecT); // TODO(dmi): add FMA implementation
			vecY = _mm_add_pd(vecY, vecC10);
			_mm_storeu_pd(&ptrOut[i], _mm_mul_pd(vecY, _mm_castsi128_pd(vecU)));
		}
		ptrIn += stride;
		ptrOut += stride;
	}

}

// "ptrOut" must be correctly strided as this function will write beyond width and up to stride
void CompVMathExpExp_minpack1_32f32f_Intrin_SSE2(COMPV_ALIGNED(SSE) const compv_float32_t* ptrIn, COMPV_ALIGNED(SSE) compv_float32_t* ptrOut, const compv_uscalar_t width, const compv_uscalar_t height, COMPV_ALIGNED(SSE) const compv_uscalar_t stride, COMPV_ALIGNED(SSE) const uint32_t* lut32u, COMPV_ALIGNED(SSE) const compv_float32_t* var32f)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	COMPV_DEBUG_INFO_CODE_TODO("ASM implemenation faster (AVX + FMA)");

	const __m128 vecMagic = _mm_set1_ps(var32f[0]); // [0]: (1 << 23) + (1 << 22)
	const __m128 vecA0 = _mm_set1_ps(var32f[1]); // [1]: expVar.a[0]
	const __m128 vecB0 = _mm_set1_ps(var32f[2]); // [2]: expVar.b[0]
	const __m128 vecMaxX = _mm_set1_ps(var32f[3]); // [3]: expVar.maxX[0]
	const __m128 vecMinX = _mm_set1_ps(var32f[4]); // [4]: expVar.minX[0]

	const __m128i vec130048 = _mm_set1_epi32(130048);
	const __m128i vec1023 = _mm_set1_epi32(1023);

	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; i += 4) {
			__m128 vecX = _mm_load_ps(&ptrIn[i]);

			vecX = _mm_min_ps(vecX, vecMaxX);
			vecX = _mm_max_ps(vecX, vecMinX);
			__m128 vecFi = _mm_add_ps(_mm_mul_ps(vecX, vecA0), vecMagic); // TODO(dmi): FMA
			__m128 vecT = _mm_sub_ps(vecFi, vecMagic);
			vecT = _mm_sub_ps(vecX, _mm_mul_ps(vecT, vecB0)); // TODO(dmi): FMA instruction "vecT = _mm_fnmadd_ps(vecT, vecB0, vecX)"

			__m128i vecU = _mm_add_epi32(_mm_castps_si128(vecFi), vec130048);
			__m128i vecV = _mm_and_si128(_mm_castps_si128(vecFi), vec1023);
			vecU = _mm_slli_epi32(_mm_srli_epi32(vecU, 10), 23);

			// TODO(dmi): AVX, use Gather instruction
			const uint32_t i0 = _mm_cvtsi128_si32(vecV);
			const uint32_t i1 = _mm_cvtsi128_si32(_mm_srli_si128(vecV, 4));
			const uint32_t i2 = _mm_cvtsi128_si32(_mm_srli_si128(vecV, 8));
			const uint32_t i3 = _mm_cvtsi128_si32(_mm_srli_si128(vecV, 12));
			__m128i vecFi0 = _mm_unpacklo_epi32(_mm_cvtsi32_si128(lut32u[i0]), _mm_cvtsi32_si128(lut32u[i1]));
			__m128i vecFi1 = _mm_unpacklo_epi32(_mm_cvtsi32_si128(lut32u[i2]), _mm_cvtsi32_si128(lut32u[i3]));
			vecFi0 = _mm_unpacklo_epi64(vecFi0, vecFi1);
			vecFi0 = _mm_or_si128(vecFi0, vecU);

			_mm_store_ps(&ptrOut[i], _mm_add_ps(_mm_mul_ps(vecT, _mm_castsi128_ps(vecFi0)), _mm_castsi128_ps(vecFi0))); // TODO(dmi): FMA
		}
		ptrIn += stride;
		ptrOut += stride;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
