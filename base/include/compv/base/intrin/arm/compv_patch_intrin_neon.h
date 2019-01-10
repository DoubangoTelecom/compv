/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_PATCH_INTRIN_NEON_H_)
#define _COMPV_BASE_PATCH_INTRIN_NEON_H_

#include "compv/base/compv_config.h"
#if COMPV_ARCH_ARM && COMPV_INTRINSIC
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

void CompVPatchMoments0110_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* top, COMPV_ALIGNED(NEON) const uint8_t* bottom, COMPV_ALIGNED(NEON) const int16_t* x, COMPV_ALIGNED(NEON) const int16_t* y, compv_uscalar_t count, compv_scalar_t* s01, compv_scalar_t* s10);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_ARM && COMPV_INTRINSIC */

#endif /* _COMPV_BASE_PATCH_INTRIN_NEON_H_ */
