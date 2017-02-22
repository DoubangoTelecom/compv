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
//mov r27, v21.d[0]
//mov r28, v21.d[1]
//orr r27, r27, r28 // orrs not avail on Aarch64
//cbz r27, AllZeros
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

#if COMPV_ARCH_ARM64
#   define COMPV_ARM_N_FMAQ(a, b, c) vfmaq_n_f32(a, b, c)
#   define COMPV_ARM_N_FMA(a, b, c) vfma_n_f32(a, b, c)
#else
#   define COMPV_ARM_N_FMAQ(a, b, c) vfmaq_f32(a, b, (float32x4_t) {c, c, c, c})
#   define COMPV_ARM_N_FMA(a, b, c) vfma_f32(a, b, (float32x2_t) {c, c})
#endif

// static_cast<inttype>((f) >= 0.0 ? ((f) + 0.5) : ((f) - 0.5))
// const float32x4_t vecHalf = vdupq_n_f32(0.5f)
#if COMPV_ARCH_ARM64
#	define COMPV_ARM_NEON_MATH_ROUNDF_2_NEAREST_INT(vec) vcvtaq_s32_f32(vec) /* in two instruction: vcvtq_s32_f32(vrndaq_f32(v)) */
#else
#	define COMPV_ARM_NEON_MATH_ROUNDF_2_NEAREST_INT(vec)({ \
		const float32x4_t vecSign = vcvtq_f32_u32((vshrq_n_u32(vec, 31))); \
		int32x4_t __ret = vcvtq_s32_f32(vsubq_f32(vaddq_f32(vec, vecHalf), vecSign)); \
		__ret; \
	})
#endif

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_ARM && COMPV_INTRINSIC */

#endif /* _COMPV_BASE_INTRIN_NEON_H_ */
