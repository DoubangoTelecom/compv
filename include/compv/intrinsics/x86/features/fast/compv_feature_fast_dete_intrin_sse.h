/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_FEATURES_FAST_DETE_INTRIN_SSE_H_)
#define _COMPV_FEATURES_FAST_DETE_INTRIN_SSE_H_

#include "compv/compv_config.h"
#include "compv/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

#if defined(COMPV_ARCH_X86) && defined(COMPV_INTRINSIC)

COMPV_NAMESPACE_BEGIN()

void FastData16Row_Intrin_SSE2(
    const uint8_t* IP,
    const uint8_t* IPprev,
    compv_scalar_t width,
    const compv_scalar_t(&pixels16)[16],
    compv_scalar_t N,
    compv_scalar_t threshold,
    uint8_t* strengths,
    compv_scalar_t* me);

void FastStrengths16_Intrin_SSE2(compv_scalar_t rbrighters, compv_scalar_t rdarkers, COMPV_ALIGNED(SSE) const uint8_t* dbrighters16x16, COMPV_ALIGNED(SSE) const uint8_t* ddarkers16x16, const compv_scalar_t(*fbrighters16)[16], const compv_scalar_t(*fdarkers16)[16], uint8_t* strengths16, compv_scalar_t N);
void FastStrengths16_Intrin_SSE41(compv_scalar_t rbrighters, compv_scalar_t rdarkers, COMPV_ALIGNED(SSE) const uint8_t* dbrighters16x16, COMPV_ALIGNED(SSE) const uint8_t* ddarkers16x16, const compv_scalar_t(*fbrighters16)[16], const compv_scalar_t(*fdarkers16)[16], uint8_t* strengths16, compv_scalar_t N);
void FastStrengths32_Intrin_SSE41(compv_scalar_t rbrighters, compv_scalar_t rdarkers, COMPV_ALIGNED(SSE) const uint8_t* dbrighters16x32, COMPV_ALIGNED(SSE) const uint8_t* ddarkers16x32, const compv_scalar_t(*fbrighters16)[16], const compv_scalar_t(*fdarkers16)[16], uint8_t* strengths32, compv_scalar_t N);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */

#endif /* _COMPV_FEATURES_FAST_DETE_INTRIN_SSE_H_ */