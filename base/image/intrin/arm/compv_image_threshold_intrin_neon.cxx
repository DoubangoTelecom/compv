/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/intrin/arm/compv_image_threshold_intrin_neon.h"

#if COMPV_ARCH_ARM && COMPV_INTRINSIC
#include "compv/base/intrin/arm/compv_intrin_neon.h"
#include "compv/base/compv_simd_globals.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// TODO(dmi): optiz issues on ARM64 (MediaPad 2) when used with otsu (histogram time also included) - image size=1282x720= loops= 1k
//		- asm: 2774.ms, intrin: 3539
void CompVImageThresholdGlobal_8u8u_Intrin_NEON(
	COMPV_ALIGNED(NEON) const uint8_t* inPtr,
	COMPV_ALIGNED(NEON) uint8_t* outPtr,
	compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride,
	compv_uscalar_t threshold
)
{
	COMPV_DEBUG_INFO_CHECK_NEON();

	const uint8x16_t vecThreshold = vdupq_n_u8(static_cast<uint8_t>(threshold));
	const compv_uscalar_t width1 = width & -64;

	compv_uscalar_t i, j;
	for (j = 0; j < height; ++j) {
		for (i = 0; i < width1; i += 64) {
			vst1q_u8(&outPtr[i], vcgtq_u8(vld1q_u8(&inPtr[i]), vecThreshold));
			vst1q_u8(&outPtr[i + 16], vcgtq_u8(vld1q_u8(&inPtr[i + 16]), vecThreshold));
			vst1q_u8(&outPtr[i + 32], vcgtq_u8(vld1q_u8(&inPtr[i + 32]), vecThreshold));
			vst1q_u8(&outPtr[i + 48], vcgtq_u8(vld1q_u8(&inPtr[i + 48]), vecThreshold));
		}
		for (; i < width; i += 16) {
			vst1q_u8(&outPtr[i], vcgtq_u8(vld1q_u8(&inPtr[i]), vecThreshold));
		}
		inPtr += stride;
		outPtr += stride;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_ARM && COMPV_INTRINSIC */
