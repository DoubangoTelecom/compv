/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/x86/compv_math_exp_intrin_avx2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

//#define __FMA3__

// Must not require memory alignment (random access from SVM)
#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx2
#endif
void CompVMathExpExp_minpack4_64f64f_Intrin_AVX2(const compv_float64_t* ptrIn, compv_float64_t* ptrOut, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride, const uint64_t* lut64u, const uint64_t* var64u, const compv_float64_t* var64f)
{
	COMPV_DEBUG_INFO_CHECK_AVX2();

	_mm256_zeroupper();

	const __m256i vecMask = _mm256_set1_epi64x(var64u[0]);
	const __m256i vecCADJ = _mm256_set1_epi64x(var64u[1]);

	const __m256d vecB = _mm256_broadcast_sd(&var64f[0]);
	const __m256d vecCA = _mm256_broadcast_sd(&var64f[1]);
	const __m256d vecCRA = _mm256_broadcast_sd(&var64f[2]);
	const __m256d vecC10 = _mm256_broadcast_sd(&var64f[3]);
	const __m256d vecC20 = _mm256_broadcast_sd(&var64f[4]);
	const __m256d vecC30 = _mm256_broadcast_sd(&var64f[5]);
	const __m256d vecMin = _mm256_broadcast_sd(&var64f[6]);
	const __m256d vecMax = _mm256_broadcast_sd(&var64f[7]);

	const compv_uscalar_t width4 = width & -4;

	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width4; i += 4) {
			__m256d vecX = _mm256_min_pd(vecMax, _mm256_loadu_pd(&ptrIn[i]));
			vecX = _mm256_max_pd(vecX, vecMin);
#if defined(__FMA3__)
			__m256d vecDI = _mm256_fmadd_pd(vecX, vecCA, vecB);
			const __m256d vecT = _mm256_fmsub_pd(_mm256_sub_pd(vecDI, vecB), vecCRA, vecX);
#else
			__m256d vecDI = _mm256_add_pd(_mm256_mul_pd(vecX, vecCA), vecB); // TODO(dmi): add FMA implementation
			const __m256d vecT = _mm256_sub_pd(_mm256_mul_pd(_mm256_sub_pd(vecDI, vecB), vecCRA), vecX); // TODO(dmi): add FMA implementation
#endif
			__m256i vecU = _mm256_slli_epi64(_mm256_srli_epi64(_mm256_add_epi64(_mm256_castpd_si256(vecDI), vecCADJ), 11), 52);
			__m256d vecY = _mm256_mul_pd(vecT, vecT);
			vecDI = _mm256_castsi256_pd(_mm256_and_si256(_mm256_castpd_si256(vecDI), vecMask));
			const __m256i vecLUT = _mm256_i64gather_epi64(reinterpret_cast<const int64_t*>(lut64u), _mm256_castpd_si256(vecDI), /*sizeof(int64_t)*/8);
			vecY = _mm256_mul_pd(vecY, _mm256_sub_pd(vecC30, vecT));
			vecU = _mm256_or_si256(vecU, vecLUT);
#if defined(__FMA3__)
			vecY = _mm256_fmsub_pd(vecY, vecC20, vecT);
#else
			vecY = _mm256_sub_pd(_mm256_mul_pd(vecY, vecC20), vecT);
#endif
			vecY = _mm256_add_pd(vecY, vecC10);
			_mm256_storeu_pd(&ptrOut[i], _mm256_mul_pd(vecY, _mm256_castsi256_pd(vecU)));
		}
		ptrIn += stride;
		ptrOut += stride;
	}

	_mm256_zeroupper();
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
