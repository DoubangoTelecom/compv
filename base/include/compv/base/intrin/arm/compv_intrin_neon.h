/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_INTRIN_NEON_H_)
#define _COMPV_BASE_INTRIN_NEON_H_

#include "compv/base/compv_config.h"
#if COMPV_ARCH_ARM && COMPV_INTRINSIC
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

#if COMPV_ARCH_ARM64
// vmov r10, r10, q0x
// vmov r11, r11, q0y
// orrs r11, r11, r10
// beq AllZeros
#	define COMPV_ARM_NEON_NEQ_ZERO(vec)	(vgetq_lane_u64(vec, 0) || vgetq_lane_u64(vec, 1))
#	define COMPV_ARM_NEON_EQ_ZERO(vec)	!COMPV_ARM_NEON_NEQ_ZERO(vec)
#else
// vorr q0x, q0x, q0y
// vmov.32	r10, q0x[0]
// vmov.32	r11, q0x[1]
// orrs r11, r11, r10
// beq AllZeros
#	define COMPV_ARM_NEON_NEQ_ZERO(vec) ({ \
	bool __ret; \
	uint8x8_t __vec = vorr_u8(vget_high_u8(vec), vget_low_u8(vec)); \
	__ret = vget_lane_u32(__vec, 0) || vget_lane_u32(__vec, 1); \
	__ret; \
})
#	define COMPV_ARM_NEON_EQ_ZERO(vec)	!COMPV_ARM_NEON_NEQ_ZERO(vec)
#endif

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_ARM && COMPV_INTRINSIC */

#endif /* _COMPV_BASE_INTRIN_NEON_H_ */
