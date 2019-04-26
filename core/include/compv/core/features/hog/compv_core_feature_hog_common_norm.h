/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_CORE_FEATURES_HOG_COMMON_NORM_H_)
#define _COMPV_CORE_FEATURES_HOG_COMMON_NORM_H_

#include "compv/core/compv_core_config.h"
#include "compv/core/compv_core_common.h"
#include "compv/base/compv_debug.h"

#include <cmath>

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

static void CompVHogCommonNormL1_32f_C(compv_float32_t* inOutPtr, const compv_float32_t* eps1, const compv_uscalar_t count)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");

	// Compute den = sum(vector) + eps
	// Using "vecDen" to emulate SIMD code in order to have same MD5 result
	const compv_uscalar_t count8 = count & -8; // count equal #9 is very common -> use count8 instead of count16
	const compv_uscalar_t count4 = count & -4;
	compv_float32_t vecDen[8] = { 0 };
	compv_uscalar_t i;
	// vector contains small values -> no need to use double for accumulation
	for (i = 0; i < count8; i += 8) {
		// no need for abs because hist are always >= 0
		vecDen[0] += inOutPtr[i + 0];
		vecDen[1] += inOutPtr[i + 1];
		vecDen[2] += inOutPtr[i + 2];
		vecDen[3] += inOutPtr[i + 3];
		vecDen[4] += inOutPtr[i + 4];
		vecDen[5] += inOutPtr[i + 5];
		vecDen[6] += inOutPtr[i + 6];
		vecDen[7] += inOutPtr[i + 7];
	}
	for (; i < count4; i += 4) {
		vecDen[0] += inOutPtr[i + 0];
		vecDen[1] += inOutPtr[i + 1];
		vecDen[2] += inOutPtr[i + 2];
		vecDen[3] += inOutPtr[i + 3];
	}
	vecDen[0] += vecDen[4];
	vecDen[1] += vecDen[5];
	vecDen[2] += vecDen[6];
	vecDen[3] += vecDen[7];
	vecDen[0] += vecDen[2];
	vecDen[1] += vecDen[3];
	vecDen[0] += vecDen[1];

	compv_float32_t& den = vecDen[0];
	for (; i < count; i += 1) {
		den += inOutPtr[i];
	}

	den = 1 / (den + *eps1);
	// Compute norm = v * (1 / den)
	for (compv_uscalar_t i = 0; i < count; ++i) {
		inOutPtr[i] *= den;
	}
}

static void CompVHogCommonNormL1Sqrt_32f_C(compv_float32_t* inOutPtr, const compv_float32_t* eps1, const compv_uscalar_t count)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");

	CompVHogCommonNormL1_32f_C(inOutPtr, eps1, count);
	for (compv_uscalar_t i = 0; i < count; ++i) {
		inOutPtr[i] = std::sqrt(inOutPtr[i]);
	}
}

static void CompVHogCommonNormL2_32f_C(compv_float32_t* inOutPtr, const compv_float32_t* eps_square1, const compv_uscalar_t count)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");

	// Compute den = L2(vector)^2 + eps^2
	// Using "vecDen" to emulate SIMD code in order to have same MD5 result
	const compv_uscalar_t count8 = count & -8; // count equal #9 is very common -> use count8 instead of count16
	const compv_uscalar_t count4 = count & -4;
	compv_float32_t vecDen[8] = { 0 };
	compv_uscalar_t i;
	// vector contains small values -> no need to use double for accumulation
	for (i = 0; i < count8; i += 8) {
		// no need for abs because hist are always >= 0
		vecDen[0] += inOutPtr[i + 0] * inOutPtr[i + 0];
		vecDen[1] += inOutPtr[i + 1] * inOutPtr[i + 1];
		vecDen[2] += inOutPtr[i + 2] * inOutPtr[i + 2];
		vecDen[3] += inOutPtr[i + 3] * inOutPtr[i + 3];
		vecDen[4] += inOutPtr[i + 4] * inOutPtr[i + 4];
		vecDen[5] += inOutPtr[i + 5] * inOutPtr[i + 5];
		vecDen[6] += inOutPtr[i + 6] * inOutPtr[i + 6];
		vecDen[7] += inOutPtr[i + 7] * inOutPtr[i + 7];
	}
	for (; i < count4; i += 4) {
		vecDen[0] += inOutPtr[i + 0] * inOutPtr[i + 0];
		vecDen[1] += inOutPtr[i + 1] * inOutPtr[i + 1];
		vecDen[2] += inOutPtr[i + 2] * inOutPtr[i + 2];
		vecDen[3] += inOutPtr[i + 3] * inOutPtr[i + 3];
	}
	vecDen[0] += vecDen[4];
	vecDen[1] += vecDen[5];
	vecDen[2] += vecDen[6];
	vecDen[3] += vecDen[7];
	vecDen[0] += vecDen[2];
	vecDen[1] += vecDen[3];
	vecDen[0] += vecDen[1];

	compv_float32_t& den = vecDen[0];
	for (; i < count; i += 1) {
		den += inOutPtr[i] * inOutPtr[i];
	}

	den = 1 / std::sqrt(den + *eps_square1);
	// Compute norm = v * (1 / den)
	for (i = 0; i < count; ++i) {
		inOutPtr[i] *= den;
	}
}

static void CompVHogCommonNormL2Hys_32f_C(compv_float32_t* inOutPtr, const compv_float32_t* eps_square1, const compv_uscalar_t count)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");

	CompVHogCommonNormL2_32f_C(inOutPtr, eps_square1, count);
	for (compv_uscalar_t i = 0; i < count; ++i) {
#if 1
		inOutPtr[i] = std::min(inOutPtr[i], 0.2f);
#else
		if (inOutPtr[i] > 0.2f) {
			inOutPtr[i] = 0.2f;
		}
#endif
	}
	CompVHogCommonNormL2_32f_C(inOutPtr, eps_square1, count);
}

COMPV_NAMESPACE_END()

#endif /* _COMPV_CORE_FEATURES_HOG_COMMON_NORM_H_ */

