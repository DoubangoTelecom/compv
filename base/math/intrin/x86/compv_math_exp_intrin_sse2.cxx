/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
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
			_mm_stream_pd(&ptrOut[i], _mm_mul_pd(vecY, _mm_castsi128_pd(vecU)));
		}
		ptrIn += stride;
		ptrOut += stride;
	}

#if 0
	const compv_uscalar_t width16 = width & -16;
	const compv_uscalar_t width2 = width & -2;
	compv_uscalar_t i;
	__m128d vecSum0 = _mm_setzero_pd();
	__m128d vecSum1 = _mm_setzero_pd();

	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (i = 0; i < width16; i += 16) { // test "width16, width16"
											// TODO(dmi): Add FMA implementation
			__m128d vec0 = _mm_mul_pd(_mm_loadu_pd(&ptrA[i]), _mm_loadu_pd(&ptrB[i]));
			__m128d vec1 = _mm_mul_pd(_mm_loadu_pd(&ptrA[i + 2]), _mm_loadu_pd(&ptrB[i + 2]));
			__m128d vec2 = _mm_mul_pd(_mm_loadu_pd(&ptrA[i + 4]), _mm_loadu_pd(&ptrB[i + 4]));
			__m128d vec3 = _mm_mul_pd(_mm_loadu_pd(&ptrA[i + 6]), _mm_loadu_pd(&ptrB[i + 6]));
			__m128d vec4 = _mm_mul_pd(_mm_loadu_pd(&ptrA[i + 8]), _mm_loadu_pd(&ptrB[i + 8]));
			__m128d vec5 = _mm_mul_pd(_mm_loadu_pd(&ptrA[i + 10]), _mm_loadu_pd(&ptrB[i + 10]));
			__m128d vec6 = _mm_mul_pd(_mm_loadu_pd(&ptrA[i + 12]), _mm_loadu_pd(&ptrB[i + 12]));
			__m128d vec7 = _mm_mul_pd(_mm_loadu_pd(&ptrA[i + 14]), _mm_loadu_pd(&ptrB[i + 14]));
			vec0 = _mm_add_pd(vec0, vec2);
			vec4 = _mm_add_pd(vec4, vec6);
			vec1 = _mm_add_pd(vec1, vec3);
			vec5 = _mm_add_pd(vec5, vec7);
			vec0 = _mm_add_pd(vec0, vec4);
			vec1 = _mm_add_pd(vec1, vec5);
			vecSum0 = _mm_add_pd(vecSum0, vec0);
			vecSum1 = _mm_add_pd(vecSum1, vec1);
		}
		for (; i < width2; i += 2) { // not "test width2, width2" but "cmp i, width2"
			__m128d vec0 = _mm_mul_pd(_mm_loadu_pd(&ptrA[i]), _mm_loadu_pd(&ptrB[i]));
			vecSum0 = _mm_add_pd(vecSum0, vec0);
		}
		for (; i < width; i += 1) {
			__m128d vec0 = _mm_mul_sd(_mm_load_sd(&ptrA[i]), _mm_load_sd(&ptrB[i]));
			vecSum0 = _mm_add_sd(vecSum0, vec0);
		}
		ptrA += strideA;
		ptrB += strideB;
	}

	vecSum0 = _mm_add_pd(vecSum0, vecSum1);
	vecSum0 = _mm_add_sd(vecSum0, _mm_shuffle_pd(vecSum0, vecSum0, 0xff));
	_mm_store_sd(ret, vecSum0);
#endif
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
