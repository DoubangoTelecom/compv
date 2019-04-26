/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_CORE_FEATURE_HOG_COMMON_NORM_INTRIN_NEON_H_)
#define _COMPV_CORE_FEATURE_HOG_COMMON_NORM_INTRIN_NEON_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

#if COMPV_ARCH_ARM && COMPV_INTRINSIC

COMPV_NAMESPACE_BEGIN()

void CompVHogCommonNormL1_32f_Intrin_NEON(compv_float32_t* inOutPtr, const compv_float32_t* eps1, const compv_uscalar_t count);
void CompVHogCommonNormL1_9_32f_Intrin_NEON(compv_float32_t* inOutPtr, const compv_float32_t* eps1, const compv_uscalar_t count);
void CompVHogCommonNormL1Sqrt_32f_Intrin_NEON(compv_float32_t* inOutPtr, const compv_float32_t* eps1, const compv_uscalar_t count);
void CompVHogCommonNormL1Sqrt_9_32f_Intrin_NEON(compv_float32_t* inOutPtr, const compv_float32_t* eps1, const compv_uscalar_t count);
void CompVHogCommonNormL2_32f_Intrin_NEON(compv_float32_t* inOutPtr, const compv_float32_t* eps_square1, const compv_uscalar_t count);
void CompVHogCommonNormL2_9_32f_Intrin_NEON(compv_float32_t* inOutPtr, const compv_float32_t* eps_square1, const compv_uscalar_t count);
void CompVHogCommonNormL2Hys_32f_Intrin_NEON(compv_float32_t* inOutPtr, const compv_float32_t* eps_square1, const compv_uscalar_t count);
void CompVHogCommonNormL2Hys_9_32f_Intrin_NEON(compv_float32_t* inOutPtr, const compv_float32_t* eps_square1, const compv_uscalar_t count);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_ARM && COMPV_INTRINSIC */

#endif /* _COMPV_CORE_FEATURE_HOG_COMMON_NORM_INTRIN_NEON_H_ */
