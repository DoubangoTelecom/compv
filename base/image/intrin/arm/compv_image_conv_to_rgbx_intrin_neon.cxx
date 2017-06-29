/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/intrin/arm/compv_image_conv_to_rgbx_intrin_neon.h"

#if COMPV_ARCH_ARM && COMPV_INTRINSIC
#include "compv/base/intrin/arm/compv_intrin_neon.h"
#include "compv/base/image/compv_image_conv_common.h"
#include "compv/base/compv_simd_globals.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

static const uint8x8_t vec16n = vdup_n_u8(16); // half-vector
static const uint8x8_t vec127n = vdup_n_u8(127); // half-vector

static const int16x8_t vec37 = vdupq_n_s16(37);
static const int16x8_t vec51 = vdupq_n_s16(51);
static const int16x8_t vec65 = vdupq_n_s16(65);
static const int16x8_t vec13 = vdupq_n_s16(13);
static const int16x8_t vec26 = vdupq_n_s16(26);

// TODO(dmi): Optiz issues. ASM code is by far faster than this (iPhone5/single thread: 1960.ms vs 3354.ms)
void CompVImageConvYuv420_to_Rgb24_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* yPtr, COMPV_ALIGNED(NEON) const uint8_t* uPtr, COMPV_ALIGNED(NEON) const uint8_t* vPtr, COMPV_ALIGNED(NEON) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	
	compv_uscalar_t i, j, k, l;
    const compv_uscalar_t strideUV = (stride >> 1); // no need for "((stride + 1) >> 1)" because stride is even (aligned on #16 bytes)
	const compv_uscalar_t strideRGB = (stride << 1) + stride;
	int16x8_t vecYlow, vecYhigh, vecU, vecV, vec0, vec1;
	uint8x16x3_t vecRGB;
	uint8x8_t vecUn, vecVn;
	int16x8x2_t vec2;

	for (j = 0; j < height; ++j) {
		for (i = 0, k = 0, l = 0; i < width; i += 16, k += 48, l += 8) {
			/* Load samples */
			vecYlow = vld1q_u8(&yPtr[i]); // #16 Y samples
			vecUn = vld1_u8(&uPtr[l]); // #8 U samples, low mem
			vecVn = vld1_u8(&vPtr[l]); // #8 V samples, low mem

			/* Compute Y', U', V': substract and convert to I16 */
			vecYhigh = vsubl_u8(vget_high_u8(vecYlow), vec16n);
			vecYlow = vsubl_u8(vget_low_u8(vecYlow), vec16n);
			vecU = vsubl_u8(vecUn, vec127n);
			vecV = vsubl_u8(vecVn, vec127n);

			/* Compute (37Y') */
			vecYlow = vmulq_s16(vecYlow, vec37);
			vecYhigh = vmulq_s16(vecYhigh, vec37);

			/* Compute R = (37Y' + 0U' + 51V') >> 5, Instead of (#2 'vmlaq_s16' + #2 'vzipq_s16') use (#1 'vmulq_s16' + #1 'vzipq_s16' and #2 'vaddq_s16') */
            vec0 = vmulq_s16(vecV, vec51);
			vec2 = vzipq_s16(vec0, vec0); // UV sampled 1/2 -> duplicate to have same size as Y
			vecRGB.val[0] = vcombine_u8(
				vqshrun_n_s16(vaddq_s16(vecYlow, vec2.val[0]), 5),
				vqshrun_n_s16(vaddq_s16(vecYhigh, vec2.val[1]), 5)
			);

			/* B = (37Y' + 65U' + 0V') >> 5, Instead of (#2 'vmlaq_s16' + #2 'vzipq_s16') use (#1 'vmulq_s16' + #1 'vzipq_s16' and #2 'vaddq_s16') */
            vec1 = vmulq_s16(vecU, vec65);
			vec2 = vzipq_s16(vec1, vec1); // UV sampled 1/2 -> duplicate to have same size as Y
			vecRGB.val[2] = vcombine_u8( //!\\ Notice the indice: #2 (B)
				vqshrun_n_s16(vaddq_s16(vecYlow, vec2.val[0]), 5),
				vqshrun_n_s16(vaddq_s16(vecYhigh, vec2.val[1]), 5)
			);

			/* Compute G = (37Y' - 13U' - 26V') >> 5 = (37Y' - (13U' + 26V')) >> 5 */
			vec0 = vmulq_s16(vecU, vec13);
			vec1 = vmlaq_s16(vec0, vecV, vec26);
			vec2 = vzipq_s16(vec1, vec1);
			vecRGB.val[1] = vcombine_u8(
				vqshrun_n_s16(vsubq_s16(vecYlow, vec2.val[0]), 5),
				vqshrun_n_s16(vsubq_s16(vecYhigh, vec2.val[1]), 5)
			);
			
			/* Store result */
			vst3q_u8(&rgbPtr[k], vecRGB);

		} // End_Of for (i = 0; i < width; i += 16)
		yPtr += stride;
		rgbPtr += strideRGB;
		if (j & 1) {
			uPtr += strideUV;
			vPtr += strideUV;
		} // End_Of for (j = 0; j < height; ++j)
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_ARM && COMPV_INTRINSIC */
