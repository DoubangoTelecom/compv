/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MATH_UTILS_INTRIN_SSSE3_H_)
#define _COMPV_BASE_MATH_UTILS_INTRIN_SSSE3_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

#if COMPV_ARCH_X86 && COMPV_INTRINSIC

COMPV_NAMESPACE_BEGIN()

void MathUtilsSumAbs_16i16u_Intrin_SSSE3(const COMPV_ALIGNED(SSE) int16_t* a, const COMPV_ALIGNED(SSE) int16_t* b, COMPV_ALIGNED(SSE) uint16_t* r, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
void MathUtilsSum_8u32u_Intrin_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* data, compv_uscalar_t count, uint32_t *sum1);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */

#endif /* _COMPV_BASE_MATH_UTILS_INTRIN_SSSE3_H_ */
