/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/features/hog/intrin/x86/compv_core_feature_hog_common_norm_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/intrin/x86/compv_intrin_sse.h"
#include "compv/base/compv_simd_globals.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

void CompVHogCommonNormL1_32f_Intrin_SSE2(compv_float32_t* inOutPtr, const compv_float32_t* eps1, const compv_uscalar_t count)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No ASM code");
    if (count == 9) {
        COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("There is a faster version");
    }
	const compv_uscalar_t count8 = count & -8; // count equal #9 is very common -> use count8 instead of count16
	const compv_uscalar_t count4 = count & -4;
	compv_uscalar_t i;
	
	__m128 vec0 = _mm_setzero_ps();
	__m128 vec1 = _mm_setzero_ps();

	for (i = 0; i < count8; i += 8) {
		// no need for abs because hists are always >= 0
		vec0 = _mm_add_ps(vec0, _mm_loadu_ps(&inOutPtr[i]));
		vec1 = _mm_add_ps(vec1, _mm_loadu_ps(&inOutPtr[i + 4]));
	}
	for (; i < count4; i += 4) {
		vec0 = _mm_add_ps(vec0, _mm_loadu_ps(&inOutPtr[i]));
	}
	vec0 = _mm_add_ps(vec0, vec1);
	vec0 = _mm_add_ps(vec0, _mm_shuffle_ps(vec0, vec0, 0x0E));
	vec0 = _mm_add_ps(vec0, _mm_shuffle_ps(vec0, vec0, 0x01));
	for (; i < count; i += 1) {
		vec0 = _mm_add_ss(vec0, _mm_load_ss(&inOutPtr[i]));
	}
	
	vec0 = _mm_add_ss(vec0, _mm_load_ss(eps1));
#if 0 // TODO(dmi): use RCP instead of 1/den
	vec0 = _mm_rcp_ss(vec0);
#else
	static const __m128 vecOne = _mm_set1_ps(1.f);
	vec0 = _mm_div_ss(vecOne, vec0);
#endif
	vec0 = _mm_shuffle_ps(vec0, vec0, 0x00);

	// Compute norm = v * (1 / den)
	for (i = 0; i < count8; i += 8) {
		_mm_storeu_ps(&inOutPtr[i], _mm_mul_ps(vec0, _mm_loadu_ps(&inOutPtr[i])));
		_mm_storeu_ps(&inOutPtr[i + 4], _mm_mul_ps(vec0, _mm_loadu_ps(&inOutPtr[i + 4])));
	}
	for (; i < count4; i += 4) {
		_mm_storeu_ps(&inOutPtr[i], _mm_mul_ps(vec0, _mm_loadu_ps(&inOutPtr[i])));
	}
	for (; i < count; ++i) {
		_mm_store_ss(&inOutPtr[i], _mm_mul_ss(vec0, _mm_load_ss(&inOutPtr[i])));
	}
}

void CompVHogCommonNormL1_9_32f_Intrin_SSE2(compv_float32_t* inOutPtr, const compv_float32_t* eps1, const compv_uscalar_t count)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("ASM code faster");
	const __m128 veca = _mm_loadu_ps(&inOutPtr[0]);
	const __m128 vecb = _mm_loadu_ps(&inOutPtr[4]);
	const __m128 vecc = _mm_load_ss(&inOutPtr[8]);
	__m128 vec0 = _mm_add_ps(veca, vecb);
	vec0 = _mm_add_ps(vec0, _mm_shuffle_ps(vec0, vec0, 0x0E));
	vec0 = _mm_add_ps(vec0, _mm_shuffle_ps(vec0, vec0, 0x01));
	vec0 = _mm_add_ss(vec0, vecc);

	vec0 = _mm_add_ss(vec0, _mm_load_ss(eps1));
#if 0 // TODO(dmi): use RCP instead of 1/den
	vec0 = _mm_rcp_ss(vec0);
#else
	static const __m128 vecOne = _mm_set1_ps(1.f);
	vec0 = _mm_div_ss(vecOne, vec0);
#endif
	vec0 = _mm_shuffle_ps(vec0, vec0, 0x00);
	
	_mm_storeu_ps(&inOutPtr[0], _mm_mul_ps(vec0, veca));
	_mm_storeu_ps(&inOutPtr[4], _mm_mul_ps(vec0, vecb));
	_mm_store_ss(&inOutPtr[8], _mm_mul_ss(vec0, vecc));
}

void CompVHogCommonNormL1Sqrt_32f_Intrin_SSE2(compv_float32_t* inOutPtr, const compv_float32_t* eps1, const compv_uscalar_t count)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No ASM code");
    if (count == 9) {
        COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("There is a faster version");
    }
	CompVHogCommonNormL1_32f_Intrin_SSE2(inOutPtr, eps1, count);
	const compv_uscalar_t count8 = count & -8; // count equal #9 is very common -> use count8 instead of count16
	const compv_uscalar_t count4 = count & -4;
	compv_uscalar_t i;
	for (i = 0; i < count8; i += 8) {
		_mm_storeu_ps(&inOutPtr[i], _mm_sqrt_ps(_mm_loadu_ps(&inOutPtr[i])));
		_mm_storeu_ps(&inOutPtr[i + 4], _mm_sqrt_ps(_mm_loadu_ps(&inOutPtr[i + 4])));
	}
	for (; i < count4; i += 4) {
		_mm_storeu_ps(&inOutPtr[i], _mm_sqrt_ps(_mm_loadu_ps(&inOutPtr[i])));
	}
	for (; i < count; i += 1) {
		_mm_store_ss(&inOutPtr[i], _mm_sqrt_ss(_mm_load_ss(&inOutPtr[i])));
	}
}

void CompVHogCommonNormL1Sqrt_9_32f_Intrin_SSE2(compv_float32_t* inOutPtr, const compv_float32_t* eps1, const compv_uscalar_t count)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("ASM code faster");
	CompVHogCommonNormL1_9_32f_Intrin_SSE2(inOutPtr, eps1, count);
	_mm_storeu_ps(&inOutPtr[0], _mm_sqrt_ps(_mm_loadu_ps(&inOutPtr[0])));
	_mm_storeu_ps(&inOutPtr[4], _mm_sqrt_ps(_mm_loadu_ps(&inOutPtr[4])));
	_mm_store_ss(&inOutPtr[8], _mm_sqrt_ss(_mm_load_ss(&inOutPtr[8])));
}

void CompVHogCommonNormL2_32f_Intrin_SSE2(compv_float32_t* inOutPtr, const compv_float32_t* eps_square1, const compv_uscalar_t count)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No ASM code");
    if (count == 9) {
        COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("There is a faster version");
    }
	const compv_uscalar_t count8 = count & -8; // count equal #9 is very common -> use count8 instead of count16
	const compv_uscalar_t count4 = count & -4;
	compv_uscalar_t i;

	__m128 vec0 = _mm_setzero_ps();
	__m128 vec1 = _mm_setzero_ps();

	for (i = 0; i < count8; i += 8) {
		const __m128 vec2 = _mm_loadu_ps(&inOutPtr[i]);
		const __m128 vec3 = _mm_loadu_ps(&inOutPtr[i + 4]);
		vec0 = _mm_add_ps(vec0, _mm_mul_ps(vec2, vec2));
		vec1 = _mm_add_ps(vec1, _mm_mul_ps(vec3, vec3));
	}
	for (; i < count4; i += 4) {
		const __m128 vec2 = _mm_loadu_ps(&inOutPtr[i]);
		vec0 = _mm_add_ps(vec0, _mm_mul_ps(vec2, vec2));
	}
	vec0 = _mm_add_ps(vec0, vec1);
	vec0 = _mm_add_ps(vec0, _mm_shuffle_ps(vec0, vec0, 0x0E));
	vec0 = _mm_add_ps(vec0, _mm_shuffle_ps(vec0, vec0, 0x01));
	for (; i < count; i += 1) {
		const __m128 vec2 = _mm_load_ss(&inOutPtr[i]);
		vec0 = _mm_add_ss(vec0, _mm_mul_ss(vec2, vec2));
	}

	vec0 = _mm_add_ss(vec0, _mm_load_ss(eps_square1));
#if 0 // TODO(dmi): use RSQRT instead of SQRT followed by DIV
	vec0 = _mm_rsqrt_ss(vec0);
#else
	static const __m128 vecOne = _mm_set1_ps(1.f);
	vec0 = _mm_sqrt_ss(vec0);
	vec0 = _mm_div_ss(vecOne, vec0);
#endif
	vec0 = _mm_shuffle_ps(vec0, vec0, 0x00);

	for (i = 0; i < count8; i += 8) {
		_mm_storeu_ps(&inOutPtr[i], _mm_mul_ps(vec0, _mm_loadu_ps(&inOutPtr[i])));
		_mm_storeu_ps(&inOutPtr[i + 4], _mm_mul_ps(vec0, _mm_loadu_ps(&inOutPtr[i + 4])));
	}
	for (; i < count4; i += 4) {
		_mm_storeu_ps(&inOutPtr[i], _mm_mul_ps(vec0, _mm_loadu_ps(&inOutPtr[i])));
	}
	for (; i < count; i += 1) {
		_mm_store_ss(&inOutPtr[i], _mm_mul_ss(vec0, _mm_load_ss(&inOutPtr[i])));
	}
}

void CompVHogCommonNormL2_9_32f_Intrin_SSE2(compv_float32_t* inOutPtr, const compv_float32_t* eps_square1, const compv_uscalar_t count)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("ASM code faster");
	const __m128 veca = _mm_loadu_ps(&inOutPtr[0]);
	const __m128 vecb = _mm_loadu_ps(&inOutPtr[4]);
	const __m128 vecc = _mm_load_ss(&inOutPtr[8]);
	__m128 vec0 = _mm_mul_ps(veca, veca);
	__m128 vec1 = _mm_mul_ps(vecb, vecb);
	vec0 = _mm_add_ps(vec0, vec1);
	vec0 = _mm_add_ps(vec0, _mm_shuffle_ps(vec0, vec0, 0x0E));
	vec0 = _mm_add_ps(vec0, _mm_shuffle_ps(vec0, vec0, 0x01));
	vec0 = _mm_add_ss(vec0, _mm_mul_ss(vecc, vecc));

	vec0 = _mm_add_ss(vec0, _mm_load_ss(eps_square1));
#if 0 // TODO(dmi): use RSQRT instead of SQRT followed by DIV
	vec0 = _mm_rsqrt_ss(vec0);
#else
	static const __m128 vecOne = _mm_set1_ps(1.f);
	vec0 = _mm_sqrt_ss(vec0);
	vec0 = _mm_div_ss(vecOne, vec0);
#endif
	vec0 = _mm_shuffle_ps(vec0, vec0, 0x00);

	_mm_storeu_ps(&inOutPtr[0], _mm_mul_ps(vec0, veca));
	_mm_storeu_ps(&inOutPtr[4], _mm_mul_ps(vec0, vecb));
	_mm_store_ss(&inOutPtr[8], _mm_mul_ss(vec0, vecc));
}

void CompVHogCommonNormL2Hys_32f_Intrin_SSE2(compv_float32_t* inOutPtr, const compv_float32_t* eps_square1, const compv_uscalar_t count)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No ASM code");
    if (count == 9) {
        COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("There is a faster version");
    }
	static const __m128 vecMax = _mm_set1_ps(0.2f);
	CompVHogCommonNormL2_32f_Intrin_SSE2(inOutPtr, eps_square1, count);
	const compv_uscalar_t count8 = count & -8; // count equal #9 is very common -> use count8 instead of count16
	const compv_uscalar_t count4 = count & -4;
	compv_uscalar_t i;
	for (i = 0; i < count8; i += 8) {
		_mm_storeu_ps(&inOutPtr[i], _mm_min_ps(_mm_loadu_ps(&inOutPtr[i]), vecMax));
		_mm_storeu_ps(&inOutPtr[i + 4], _mm_min_ps(_mm_loadu_ps(&inOutPtr[i + 4]), vecMax));
	}
	for (; i < count4; i += 4) {
		_mm_storeu_ps(&inOutPtr[i], _mm_min_ps(_mm_loadu_ps(&inOutPtr[i]), vecMax));
	}
	for (i; i < count; ++i) {
		_mm_store_ss(&inOutPtr[i], _mm_min_ss(_mm_load_ss(&inOutPtr[i]), vecMax));
	}
	CompVHogCommonNormL2_32f_Intrin_SSE2(inOutPtr, eps_square1, count);
}

void CompVHogCommonNormL2Hys_9_32f_Intrin_SSE2(compv_float32_t* inOutPtr, const compv_float32_t* eps_square1, const compv_uscalar_t count)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("ASM code faster");
	static const __m128 vecMax = _mm_set1_ps(0.2f);
	CompVHogCommonNormL2_9_32f_Intrin_SSE2(inOutPtr, eps_square1, count);	
	_mm_storeu_ps(&inOutPtr[0], _mm_min_ps(_mm_loadu_ps(&inOutPtr[0]), vecMax));
	_mm_storeu_ps(&inOutPtr[4], _mm_min_ps(_mm_loadu_ps(&inOutPtr[4]), vecMax));
	_mm_store_ss(&inOutPtr[8], _mm_min_ss(_mm_load_ss(&inOutPtr[8]), vecMax));
	CompVHogCommonNormL2_9_32f_Intrin_SSE2(inOutPtr, eps_square1, count);
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
