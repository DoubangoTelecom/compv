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
			const __m256i vecLUT = _mm256_i64gather_epi64(reinterpret_cast<const long long*>(lut64u), _mm256_castpd_si256(vecDI), /*sizeof(int64_t)*/8);
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

#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx2
#endif
void CompVMathExpExp_minpack1_32f32f_Intrin_AVX2(COMPV_ALIGNED(AVX) const compv_float32_t* ptrIn, COMPV_ALIGNED(AVX) compv_float32_t* ptrOut, const compv_uscalar_t width, const compv_uscalar_t height, COMPV_ALIGNED(AVX) const compv_uscalar_t stride, COMPV_ALIGNED(AVX) const uint32_t* lut32u, COMPV_ALIGNED(AVX) const compv_float32_t* var32f)
{
	COMPV_DEBUG_INFO_CHECK_AVX2();
	COMPV_DEBUG_INFO_CODE_TODO("ASM implemenation faster (AVX + FMA)");

	_mm256_zeroupper();

	const __m256 vecMagic = _mm256_set1_ps(var32f[0]); // [0]: (1 << 23) + (1 << 22)
	const __m256 vecA0 = _mm256_set1_ps(var32f[1]); // [1]: expVar.a[0]
	const __m256 vecB0 = _mm256_set1_ps(var32f[2]); // [2]: expVar.b[0]
	const __m256 vecMaxX = _mm256_set1_ps(var32f[3]); // [3]: expVar.maxX[0]
	const __m256 vecMinX = _mm256_set1_ps(var32f[4]); // [4]: expVar.minX[0]

	const __m256i vec130048 = _mm256_set1_epi32(130048);
	const __m256i vec1023 = _mm256_set1_epi32(1023);

	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; i += 8) {
			__m256 vecX = _mm256_load_ps(&ptrIn[i]);

			vecX = _mm256_min_ps(vecX, vecMaxX);
			vecX = _mm256_max_ps(vecX, vecMinX);
			__m256 vecFi = _mm256_add_ps(_mm256_mul_ps(vecX, vecA0), vecMagic); // TODO(dmi): FMA
			__m256 vecT = _mm256_sub_ps(vecFi, vecMagic);
			vecT = _mm256_sub_ps(vecX, _mm256_mul_ps(vecT, vecB0)); // TODO(dmi): FMA instruction "vecT = _mm256_fnmadd_ps(vecT, vecB0, vecX)"

			__m256i vecU = _mm256_add_epi32(_mm256_castps_si256(vecFi), vec130048);
			__m256i vecV = _mm256_and_si256(_mm256_castps_si256(vecFi), vec1023);
			vecU = _mm256_slli_epi32(_mm256_srli_epi32(vecU, 10), 23);

			__m256i vecFi0 = _mm256_i32gather_epi32(reinterpret_cast<const int32_t *>(lut32u), vecV, 4); // 4 = sizeof(int32_t)
			vecFi0 = _mm256_or_si256(vecFi0, vecU);

			_mm256_store_ps(&ptrOut[i], _mm256_add_ps(_mm256_mul_ps(vecT, _mm256_castsi256_ps(vecFi0)), _mm256_castsi256_ps(vecFi0))); // TODO(dmi): FMA
		}
		ptrIn += stride;
		ptrOut += stride;
	}

	_mm256_zeroupper();
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
