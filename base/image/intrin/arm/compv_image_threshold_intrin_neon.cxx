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

void CompVImageThresholdOtsuSum_32s32s_Intrin_NEON(COMPV_ALIGNED(NEON) const uint32_t* ptr32uHistogram, COMPV_ALIGNED(NEON) uint32_t* sumA256, uint32_t* sumB1)
{
	COMPV_DEBUG_INFO_CHECK_NEON();

	const uint32x4_t vecIndicesInc = vdupq_n_u32(16);
	static const uint32_t data[4] = { 0, 1, 2, 3 };
	static const uint32x4_t vec4 = vdupq_n_u32(4);
	uint32x4_t vecIndices0 = vld1q_u32(data);
	uint32x4_t vecIndices1 = vaddq_u32(vecIndices0, vec4);
	uint32x4_t vecIndices2 = vaddq_u32(vecIndices1, vec4);
	uint32x4_t vecIndices3 = vaddq_u32(vecIndices2, vec4);
	uint32x4_t vec0, vec1, vec2, vec3, vecSumB0, vecSumB1, vecSumB2, vecSumB3;

	vecSumB0 = vdupq_n_u32(0);
	vecSumB1 = vdupq_n_u32(0);
	vecSumB2 = vdupq_n_u32(0);
	vecSumB3 = vdupq_n_u32(0);

	for (size_t i = 0; i < 256; i += 16) {
		vec0 = vld1q_u32(&ptr32uHistogram[i]);
		vec1 = vld1q_u32(&ptr32uHistogram[i + 4]);
		vec2 = vld1q_u32(&ptr32uHistogram[i + 8]);
		vec3 = vld1q_u32(&ptr32uHistogram[i + 12]);

		vec0 = vmulq_u32(vecIndices0, vec0);
		vec1 = vmulq_u32(vecIndices1, vec1);
		vec2 = vmulq_u32(vecIndices2, vec2);
		vec3 = vmulq_u32(vecIndices3, vec3);

		vecIndices0 = vaddq_u32(vecIndices0, vecIndicesInc);
		vecIndices1 = vaddq_u32(vecIndices1, vecIndicesInc);
		vecIndices2 = vaddq_u32(vecIndices2, vecIndicesInc);
		vecIndices3 = vaddq_u32(vecIndices3, vecIndicesInc);

		vecSumB0 = vaddq_u32(vecSumB0, vec0);
		vecSumB1 = vaddq_u32(vecSumB1, vec1);
		vecSumB2 = vaddq_u32(vecSumB2, vec2);
		vecSumB3 = vaddq_u32(vecSumB3, vec3);

		vst1q_u32(&sumA256[i], vec0);
		vst1q_u32(&sumA256[i + 4], vec1);
		vst1q_u32(&sumA256[i + 8], vec2);
		vst1q_u32(&sumA256[i + 12], vec3);
	}

	vecSumB0 = vaddq_u32(vecSumB0, vecSumB1);
	vecSumB2 = vaddq_u32(vecSumB2, vecSumB3);
	vecSumB0 = vaddq_u32(vecSumB0, vecSumB2);

	uint32x2_t vecSumB0n = vadd_u32(vget_low_u32(vecSumB0), vget_high_u32(vecSumB0));
	*sumB1 = vget_lane_u32(vecSumB0n, 0) + vget_lane_u32(vecSumB0n, 1);
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_ARM && COMPV_INTRINSIC */
