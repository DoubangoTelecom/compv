/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/features/hough/intrin/x86/compv_core_feature_houghkht_intrin_avx.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/intrin/x86/compv_intrin_sse.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// up to the caller to set padding bytes to zeros (otherwise max will be invalid)
// 4mpq -> minpack 4 for qwords (float64) - for count
#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx
#endif
void CompVHoughKhtKernelHeight_4mpq_Intrin_AVX(
	COMPV_ALIGNED(AVX) const double* M_Eq14_r0, COMPV_ALIGNED(AVX) const double* M_Eq14_0, COMPV_ALIGNED(AVX) const double* M_Eq14_2, COMPV_ALIGNED(AVX) const double* n_scale,
	COMPV_ALIGNED(AVX) double* sigma_rho_square, COMPV_ALIGNED(AVX) double* sigma_rho_times_theta, COMPV_ALIGNED(AVX) double* m2, COMPV_ALIGNED(AVX) double* sigma_theta_square,
	COMPV_ALIGNED(AVX) double* height, COMPV_ALIGNED(AVX) double* heightMax1, COMPV_ALIGNED(AVX) compv_uscalar_t count)
{
	COMPV_DEBUG_INFO_CHECK_AVX();

	_mm256_zeroupper();

	static const __m256d vecTwoPi = _mm256_set1_pd(6.2831853071795862);
	static const __m256d vecOne = _mm256_set1_pd(1.0);
	static const __m256d vecFour = _mm256_set1_pd(4.0);
	static const __m256d vecZeroDotOne = _mm256_set1_pd(0.1);
	static const __m256d vecZero = _mm256_set1_pd(0.0);
	__m256d vecheightMax1, vecR0, vecM_Eq14_0, vecM_Eq14_2;
	__m256d vecSigma_rho_square, vecSigma_rho_times_sigma_theta, vecSigma_rho_times_theta, vecSigma_theta_square, vecM2;
	__m256d vecOne_minus_r_square, vecHeight;
	__m256d vecMaskEqZero;

	vecheightMax1 = _mm256_broadcast_sd(heightMax1);

	for (compv_uscalar_t i = 0; i < count; i += 4) {
		vecR0 = _mm256_load_pd(&M_Eq14_r0[i]);
		vecR0 = _mm256_div_pd(vecOne, vecR0);
		vecM_Eq14_0 = _mm256_load_pd(&M_Eq14_0[i]);
		vecM_Eq14_2 = _mm256_load_pd(&M_Eq14_2[i]);
		vecSigma_rho_times_theta = _mm256_mul_pd(vecM_Eq14_0, vecR0);
		vecSigma_theta_square = _mm256_mul_pd(vecM_Eq14_2, vecR0);
		vecSigma_rho_square = _mm256_add_pd(_mm256_mul_pd(vecSigma_rho_times_theta, vecM_Eq14_0), _mm256_load_pd(&n_scale[i]));
		vecSigma_rho_times_theta = _mm256_mul_pd(vecSigma_rho_times_theta, vecM_Eq14_2);
		vecM2 = _mm256_mul_pd(vecSigma_theta_square, vecM_Eq14_0);
		vecSigma_theta_square = _mm256_mul_pd(vecSigma_theta_square, vecM_Eq14_2);
		vecMaskEqZero = _mm256_cmp_pd(vecSigma_theta_square, vecZero, _CMP_EQ_OQ);
		vecSigma_theta_square = _mm256_or_pd(_mm256_and_pd(vecMaskEqZero, vecZeroDotOne), _mm256_andnot_pd(vecMaskEqZero, vecSigma_theta_square));
		vecSigma_rho_square = _mm256_mul_pd(vecSigma_rho_square, vecFour);
		vecSigma_theta_square = _mm256_mul_pd(vecSigma_theta_square, vecFour);
		vecSigma_rho_times_sigma_theta = _mm256_mul_pd(_mm256_sqrt_pd(vecSigma_rho_square), _mm256_sqrt_pd(vecSigma_theta_square));
		vecOne_minus_r_square = _mm256_div_pd(vecSigma_rho_times_theta, vecSigma_rho_times_sigma_theta);
		vecOne_minus_r_square = _mm256_sub_pd(vecOne, _mm256_mul_pd(vecOne_minus_r_square, vecOne_minus_r_square));
		vecOne_minus_r_square = _mm256_sqrt_pd(vecOne_minus_r_square);
		vecOne_minus_r_square = _mm256_mul_pd(vecOne_minus_r_square, vecSigma_rho_times_sigma_theta);
		vecOne_minus_r_square = _mm256_mul_pd(vecOne_minus_r_square, vecTwoPi);
		vecHeight = _mm256_div_pd(vecOne, vecOne_minus_r_square);
		
		_mm256_store_pd(&sigma_rho_square[i], vecSigma_rho_square);
		_mm256_store_pd(&sigma_rho_times_theta[i], vecSigma_rho_times_theta);
		_mm256_store_pd(&m2[i], vecM2);
		_mm256_store_pd(&sigma_theta_square[i], vecSigma_theta_square);
		_mm256_store_pd(&height[i], vecHeight);
		vecheightMax1 = _mm256_max_pd(vecheightMax1, vecHeight);
	}

	// SSE code, requires building the code with /AVX flags otherwise AVX/SSE transition issues
	__m128d vecMax = _mm_max_pd(_mm256_castpd256_pd128(vecheightMax1), _mm256_extractf128_pd(vecheightMax1, 0x1));
	vecMax = _mm_max_sd(vecMax, _mm_shuffle_pd(vecMax, vecMax, 0x11));
	_mm_store_sd(heightMax1, vecMax);

	_mm256_zeroupper();
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
