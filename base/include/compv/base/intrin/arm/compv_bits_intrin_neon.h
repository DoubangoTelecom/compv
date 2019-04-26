/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_BITS_INTRIN_NEON_H_)
#define _COMPV_BASE_BITS_INTRIN_NEON_H_

#include "compv/base/compv_config.h"
#if COMPV_ARCH_ARM && COMPV_INTRINSIC
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

void CompVBitsLogicalAnd_8u_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* Aptr, COMPV_ALIGNED(NEON) const uint8_t* Bptr, uint8_t* Rptr, compv_uscalar_t width, COMPV_ALIGNED(NEON) compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t Astride, COMPV_ALIGNED(NEON) compv_uscalar_t Bstride, COMPV_ALIGNED(NEON) compv_uscalar_t Rstride);
void CompVBitsLogicalNotAnd_8u_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* Aptr, COMPV_ALIGNED(NEON) const uint8_t* Bptr, uint8_t* Rptr, compv_uscalar_t width, COMPV_ALIGNED(NEON) compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t Astride, COMPV_ALIGNED(NEON) compv_uscalar_t Bstride, COMPV_ALIGNED(NEON) compv_uscalar_t Rstride);
void CompVBitsLogicalNot_8u_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* Aptr, COMPV_ALIGNED(NEON) uint8_t* Rptr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t Astride, COMPV_ALIGNED(NEON) compv_uscalar_t Rstride);
void CompVBitsLogicalXorVt_8u_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* Aptr, COMPV_ALIGNED(NEON) const uint8_t* A_Minus1_ptr, COMPV_ALIGNED(NEON) uint8_t* Rptr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t Astride, COMPV_ALIGNED(NEON) compv_uscalar_t Rstride);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_ARM && COMPV_INTRINSIC */

#endif /* _COMPV_BASE_BITS_INTRIN_NEON_H_ */
