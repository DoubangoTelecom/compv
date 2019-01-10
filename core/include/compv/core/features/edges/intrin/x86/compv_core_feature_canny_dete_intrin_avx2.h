/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_CORE_FEATURE_CANNY_DETE_INTRIN_AVX2_H_)
#define _COMPV_CORE_FEATURE_CANNY_DETE_INTRIN_AVX2_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

#if COMPV_ARCH_X86 && COMPV_INTRINSIC

COMPV_NAMESPACE_BEGIN()

void CompVCannyHysteresisRow_16mpw_Intrin_AVX2(size_t row, size_t colStart, size_t width, size_t height, size_t stride, uint16_t tLow, uint16_t tHigh, const uint16_t* grad, const uint16_t* g0, uint8_t* e, uint8_t* e0);

void CompVCannyNMSApply_Intrin_AVX2(COMPV_ALIGNED(AVX) uint16_t* grad, COMPV_ALIGNED(AVX) uint8_t* nms, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(AVX) compv_uscalar_t stride);

void CompVCannyNMSGatherRow_16mpw_Intrin_AVX2(uint8_t* nms, const uint16_t* g, const int16_t* gx, const int16_t* gy, const uint16_t* tLow1, compv_uscalar_t width, compv_uscalar_t stride);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */

#endif /* _COMPV_CORE_FEATURE_CANNY_DETE_INTRIN_AVX2_H_ */
