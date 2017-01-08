/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_CORE_FEATURES_FAST_DETE_INTRIN_SSE2_H_)
#define _COMPV_CORE_FEATURES_FAST_DETE_INTRIN_SSE2_H_

#include "compv/base/compv_config.h"
#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

void CompVFastDataRow16_Intrin_SSE2(const uint8_t* IP, COMPV_ALIGNED(SSE) compv_uscalar_t width, COMPV_ALIGNED(SSE) const compv_scalar_t *pixels16, compv_uscalar_t N, compv_uscalar_t threshold, uint8_t* strengths);
void CompVFastNmsGather_Intrin_SSE2(const uint8_t* pcStrengthsMap, uint8_t* pNMS, const compv_uscalar_t width, compv_uscalar_t heigth, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
void CompVFastNmsApply_Intrin_SSE2(COMPV_ALIGNED(SSE) uint8_t* pcStrengthsMap, COMPV_ALIGNED(SSE) uint8_t* pNMS, compv_uscalar_t width, compv_uscalar_t heigth, COMPV_ALIGNED(SSE) compv_uscalar_t stride);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */

#endif /* _COMPV_CORE_FEATURES_FAST_DETE_INTRIN_SSE2_H_ */
