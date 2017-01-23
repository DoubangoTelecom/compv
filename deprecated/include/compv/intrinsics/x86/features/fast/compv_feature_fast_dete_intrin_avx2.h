/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_FEATURES_FAST_DETE_INTRIN_AVX2_H_)
#define _COMPV_FEATURES_FAST_DETE_INTRIN_AVX2_H_

#include "compv/compv_config.h"

#if defined(COMPV_ARCH_X86) && defined(COMPV_INTRINSIC)
#include "compv/compv_common.h"
#include "compv/image/compv_image.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

void FastData32Row_Intrin_AVX2(
    const uint8_t* IP,
    const uint8_t* IPprev,
    compv_scalar_t width,
    const compv_scalar_t(&pixels16)[16],
    compv_scalar_t N,
    compv_scalar_t threshold,
    uint8_t* strengths,
    compv_scalar_t* me);
void FastStrengths32_Intrin_AVX2(compv_scalar_t rbrighters, compv_scalar_t rdarkers, COMPV_ALIGNED(AVX) const uint8_t* dbrighters16x32, COMPV_ALIGNED(AVX) const uint8_t* ddarkers16x32, const compv_scalar_t(*fbrighters16)[16], const compv_scalar_t(*fdarkers16)[16], uint8_t* strengths32, compv_scalar_t N);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 */

#endif /* _COMPV_FEATURES_FAST_DETE_INTRIN_AVX2_H_ */
