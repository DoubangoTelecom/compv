/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/intrin/arm/compv_image_remap_intrin_neon.h"

#if COMPV_ARCH_ARM && COMPV_INTRINSIC
#include "compv/base/intrin/arm/compv_intrin_neon.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// TODO(dmi): add ASM code
void CompVImageRemapBilinear_8u32f_Intrin_NEON(
	COMPV_ALIGNED(NEON) const compv_float32_t* mapXPtr, COMPV_ALIGNED(NEON) const compv_float32_t* mapYPtr,
	const uint8_t* inputPtr, compv_float32_t* outputPtr,
	COMPV_ALIGNED(NEON) const compv_float32_t* roi, COMPV_ALIGNED(NEON) const int32_t* size,
	const compv_float32_t* defaultPixelValue1,
	COMPV_ALIGNED(NEON) const compv_uscalar_t count
)
{
	COMPV_DEBUG_INFO_CHECK_NEON();

	const float32x4_t roi_left = vld1q_dup_f32(&roi[0]); // 32f
	const float32x4_t roi_right = vld1q_dup_f32(&roi[1]); // 32f
	const float32x4_t roi_top = vld1q_dup_f32(&roi[2]); // 32f
	const float32x4_t roi_bottom = vld1q_dup_f32(&roi[3]); // 32f

	const float32x4_t one = vdupq_n_f32(1.f);

	const int32x4_t inWidthMinus1 = vld1q_dup_s32(&size[0]); // 32s
	const int32x4_t inHeightMinus1 = vld1q_dup_s32(&size[1]); // 32s
	const int32x4_t stride = vld1q_dup_s32(&size[2]); // 32s
	const int32x4_t maxIndex = vdupq_n_s32((size[2] * (size[1] + 1)) - 1); // (stride * inHeight) - 1

	const int32x4_t zero = vdupq_n_s32(0);

	const float32x4_t defaultPixelValue = vld1q_dup_f32(defaultPixelValue1); // 32f

	COMPV_ALIGN_NEON() int32_t y1x1_mem[4];
	COMPV_ALIGN_NEON() int32_t y1x2_mem[4];
	COMPV_ALIGN_NEON() int32_t y2x1_mem[4];
	COMPV_ALIGN_NEON() int32_t y2x2_mem[4];

	for (compv_uscalar_t i = 0; i < count; i += 4) {
		const float32x4_t x = vld1q_f32(&mapXPtr[i]);
		const float32x4_t y = vld1q_f32(&mapYPtr[i]);

		const float32x4_t cmp = (float32x4_t)vandq_u32(
			vandq_u32(vcgeq_f32(x, roi_left), vcleq_f32(x, roi_right)),
			vandq_u32(vcgeq_f32(y, roi_top), vcleq_f32(y, roi_bottom))
		);
		
		if (COMPV_ARM_NEON_NEQ_ZEROQ(cmp)) {
			const int32x4_t x1 = vcvtq_s32_f32(x); // truncation to emulate "static_cast<int32_t>()"
			const int32x4_t x2 = vminq_s32(vcvtq_s32_f32(vaddq_f32(x, one)), inWidthMinus1);
			const float32x4_t xfractpart = vsubq_f32(x, vcvtq_f32_s32(x1));
			int32x4_t y1 = vcvtq_s32_f32(y); // truncation to emulate "static_cast<int32_t>()"
			int32x4_t y2 = vminq_s32(vcvtq_s32_f32(vaddq_f32(y, one)), inHeightMinus1);
			const float32x4_t yfractpart = vsubq_f32(y, vcvtq_f32_s32(y1));
			const float32x4_t xyfractpart = vmulq_f32(xfractpart, yfractpart);
			y1 = vmulq_s32(y1, stride);
			y2 = vmulq_s32(y2, stride);
			const int32x4_t y1x1 = vmaxq_s32(vminq_s32(maxIndex, vaddq_s32(y1, x1)), zero); // clip to avoid reading beyong accessible memory address. "mask" is useless here.
			const int32x4_t y1x2 = vmaxq_s32(vminq_s32(maxIndex, vaddq_s32(y1, x2)), zero);
			const int32x4_t y2x1 = vmaxq_s32(vminq_s32(maxIndex, vaddq_s32(y2, x1)), zero);
			const int32x4_t y2x2 = vmaxq_s32(vminq_s32(maxIndex, vaddq_s32(y2, x2)), zero);
			vst1q_s32(y1x1_mem, y1x1);
			vst1q_s32(y1x2_mem, y1x2);
			vst1q_s32(y2x1_mem, y2x1);
			vst1q_s32(y2x2_mem, y2x2);
			// "A = (1 - xfractpart - yfractpart + xyfractpart)"
			const float32x4_t A = vaddq_f32(vsubq_f32(vsubq_f32(one, xfractpart), yfractpart), xyfractpart);
			// "B = (xfractpart - xyfractpart)"
			const float32x4_t B = vsubq_f32(xfractpart, xyfractpart);
			// "C = (yfractpart - xyfractpart)"
			const float32x4_t C = vsubq_f32(yfractpart, xyfractpart);

			y1x1_mem[0] = inputPtr[y1x1_mem[0]];
			y1x1_mem[1] = inputPtr[y1x1_mem[1]];
			y1x1_mem[2] = inputPtr[y1x1_mem[2]];
			y1x1_mem[3] = inputPtr[y1x1_mem[3]];
			y1x2_mem[0] = inputPtr[y1x2_mem[0]];
			y1x2_mem[1] = inputPtr[y1x2_mem[1]];
			y1x2_mem[2] = inputPtr[y1x2_mem[2]];
			y1x2_mem[3] = inputPtr[y1x2_mem[3]];
			y2x1_mem[0] = inputPtr[y2x1_mem[0]];
			y2x1_mem[1] = inputPtr[y2x1_mem[1]];
			y2x1_mem[2] = inputPtr[y2x1_mem[2]];
			y2x1_mem[3] = inputPtr[y2x1_mem[3]];
			y2x2_mem[0] = inputPtr[y2x2_mem[0]];
			y2x2_mem[1] = inputPtr[y2x2_mem[1]];
			y2x2_mem[2] = inputPtr[y2x2_mem[2]];
			y2x2_mem[3] = inputPtr[y2x2_mem[3]];
			
			const float32x4_t y1x1_vec = vcvtq_f32_s32(vld1q_s32(y1x1_mem));
			const float32x4_t y1x2_vec = vcvtq_f32_s32(vld1q_s32(y1x2_mem));
			const float32x4_t y2x1_vec = vcvtq_f32_s32(vld1q_s32(y2x1_mem));
			const float32x4_t y2x2_vec = vcvtq_f32_s32(vld1q_s32(y2x2_mem));
			
			float32x4_t pixel = vaddq_f32(vaddq_f32(vaddq_f32(vmulq_f32(y1x1_vec, A), vmulq_f32(y1x2_vec, B)), vmulq_f32(y2x1_vec, C)), vmulq_f32(y2x2_vec, xyfractpart));
			pixel = (float32x4_t)vorrq_s32(vandq_s32((int32x4_t)pixel, (int32x4_t)cmp), vbicq_s32((int32x4_t)defaultPixelValue, (int32x4_t)cmp));
			
			vst1q_f32(&outputPtr[i], pixel);
		}
		else {
			vst1q_f32(&outputPtr[i], defaultPixelValue);
		}
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_ARM && COMPV_INTRINSIC */
