/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/x86/compv_math_trig_intrin_avx.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/intrin/x86/compv_intrin_avx.h"
#include "compv/base/compv_simd_globals.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx
#endif
void CompVMathTrigFastAtan2_32f_Intrin_AVX(COMPV_ALIGNED(AVX) const compv_float32_t* y, COMPV_ALIGNED(AVX) const compv_float32_t* x, COMPV_ALIGNED(AVX) compv_float32_t* r, const compv_float32_t* scale1, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(AVX) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_AVX();
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("ASM AVX-FMA3 version is faster");
	_mm256_zeroupper();
	static const __m256 vecAtan2_eps = _mm256_load_ps(kAtan2Eps_32f);
	static const __m256 vecAtan2_p1 = _mm256_load_ps(kAtan2P1_32f);
	static const __m256 vecAtan2_p3 = _mm256_load_ps(kAtan2P3_32f);
	static const __m256 vecAtan2_p5 = _mm256_load_ps(kAtan2P5_32f);
	static const __m256 vecAtan2_p7 = _mm256_load_ps(kAtan2P7_32f);
	static const __m256 vecAtan2_zero = _mm256_setzero_ps();
	static const __m256 vecAtan2_plus90 = _mm256_load_ps(k90_32f);
	static const __m256 vecAtan2_plus180 = _mm256_load_ps(k180_32f);
	static const __m256 vecAtan2_plus360 = _mm256_load_ps(k360_32f);
	static const __m256 vecAtan2_sign = _mm256_castsi256_ps(_mm256_set1_epi32(0x7fffffff)); // used to compute fabs, not needed for ARM NEON
	const __m256 vecAtan2_scale = _mm256_set1_ps(*scale1);
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; i += 8) {
			// ax = std::abs(x[i]), ay = std::abs(y[i]);
			__m256 vecAx = _mm256_and_ps(_mm256_load_ps(&x[i]), vecAtan2_sign);
			__m256 vecAy = _mm256_and_ps(_mm256_load_ps(&y[i]), vecAtan2_sign);

			// if (ax >= ay) vec1 = ay, vec2 = ax;
			// else vec1 = ax, vec2 = ay;
			__m256 vecMask = _mm256_cmp_ps(vecAx, vecAy, _CMP_GE_OQ);
			__m256 vec1 = _mm256_and_ps(vecMask, vecAy);
			__m256 vec2 = _mm256_and_ps(vecMask, vecAx);
			vec1 = _mm256_or_ps(vec1, _mm256_andnot_ps(vecMask, vecAx));
			vec2 = _mm256_or_ps(vec2, _mm256_andnot_ps(vecMask, vecAy));

			// c = vec1 / (vec2 + atan2_eps)
			// c2 = c*c
			const __m256 vecC = _mm256_div_ps(vec1, _mm256_add_ps(vec2, vecAtan2_eps));
			const __m256 vecC2 = _mm256_mul_ps(vecC, vecC);

			// a = (((atan2_p7*c2 + atan2_p5)*c2 + atan2_p3)*c2 + atan2_p1)*c
			__m256 vec0 = _mm256_add_ps(_mm256_mul_ps(vecAtan2_p7, vecC2), vecAtan2_p5); // TODO(dmi): AVX/NEON: Use fusedMulAdd
			vec0 = _mm256_add_ps(_mm256_mul_ps(vec0, vecC2), vecAtan2_p3); // TODO(dmi): AVX/NEON: Use fusedMulAdd
			vec0 = _mm256_add_ps(_mm256_mul_ps(vec0, vecC2), vecAtan2_p1); // TODO(dmi): AVX/NEON: Use fusedMulAdd
			vec0 = _mm256_mul_ps(vec0, vecC);
			
			// if (!(ax >= ay)) a = 90 - a
			vec1 = _mm256_andnot_ps(vecMask, _mm256_sub_ps(vecAtan2_plus90, vec0));
			vec0 = _mm256_or_ps(_mm256_and_ps(vecMask, vec0), vec1);

			// if (x[i] < 0) a = 180.f - a
			vecMask = _mm256_cmp_ps(_mm256_load_ps(&x[i]), vecAtan2_zero, _CMP_LT_OQ);
			vec1 = _mm256_and_ps(vecMask, _mm256_sub_ps(vecAtan2_plus180, vec0));
			vec0 = _mm256_or_ps(_mm256_andnot_ps(vecMask, vec0), vec1);

			// if (y[i] < 0) a = 360.f - a
			vecMask = _mm256_cmp_ps(_mm256_load_ps(&y[i]), vecAtan2_zero, _CMP_LT_OQ);
			vec1 = _mm256_and_ps(vecMask, _mm256_sub_ps(vecAtan2_plus360, vec0));
			vec0 = _mm256_or_ps(_mm256_andnot_ps(vecMask, vec0), vec1);

			// r[i] = a * scale
			_mm256_store_ps(&r[i], _mm256_mul_ps(vec0, vecAtan2_scale));
		}
		y += stride;
		x += stride;
		r += stride;
	}
	_mm256_zeroupper();
}

#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx
#endif
void CompVMathTrigHypotNaive_32f_Intrin_AVX(COMPV_ALIGNED(AVX) const compv_float32_t* x, COMPV_ALIGNED(AVX) const compv_float32_t* y, COMPV_ALIGNED(AVX) compv_float32_t* r, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(AVX) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_AVX();
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("ASM AVX-FMA3 version is faster");
	_mm256_zeroupper();
	const compv_uscalar_t width32 = width & -32;
	compv_uscalar_t i;
	__m256 vec0, vec1, vec2, vec3, vec4, vec5, vec6, vec7;

	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (i = 0; i < width32; i += 32) {
			vec0 = _mm256_load_ps(&x[i]);
			vec1 = _mm256_load_ps(&x[i + 8]);
			vec2 = _mm256_load_ps(&x[i + 16]);
			vec3 = _mm256_load_ps(&x[i + 24]);
			vec4 = _mm256_load_ps(&y[i]);
			vec5 = _mm256_load_ps(&y[i + 8]);
			vec6 = _mm256_load_ps(&y[i + 16]);
			vec7 = _mm256_load_ps(&y[i + 24]);
			_mm256_store_ps(&r[i], _mm256_sqrt_ps(_mm256_add_ps(_mm256_mul_ps(vec0, vec0), _mm256_mul_ps(vec4, vec4)))); // TODO(dmi): Add support for FMA3 (see ASM code)
			_mm256_store_ps(&r[i + 8], _mm256_sqrt_ps(_mm256_add_ps(_mm256_mul_ps(vec1, vec1), _mm256_mul_ps(vec5, vec5)))); // TODO(dmi): Add support for FMA3 (see ASM code)
			_mm256_store_ps(&r[i + 16], _mm256_sqrt_ps(_mm256_add_ps(_mm256_mul_ps(vec2, vec2), _mm256_mul_ps(vec6, vec6)))); // TODO(dmi): Add support for FMA3 (see ASM code)
			_mm256_store_ps(&r[i + 24], _mm256_sqrt_ps(_mm256_add_ps(_mm256_mul_ps(vec3, vec3), _mm256_mul_ps(vec7, vec7)))); // TODO(dmi): Add support for FMA3 (see ASM code)
		}
		for (; i < width; i += 8) {
			vec0 = _mm256_load_ps(&x[i]);
			vec4 = _mm256_load_ps(&y[i]);
			_mm256_store_ps(&r[i], _mm256_sqrt_ps(_mm256_add_ps(_mm256_mul_ps(vec0, vec0), _mm256_mul_ps(vec4, vec4)))); // TODO(dmi): Add support for FMA3 (see ASM code)
		}
		y += stride;
		x += stride;
		r += stride;
	}
	_mm256_zeroupper();
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
