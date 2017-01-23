/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/intrin/arm/compv_image_conv_grayscale_intrin_neon.h"

#if COMPV_ARCH_ARM && COMPV_INTRINSIC
#include "compv/base/intrin/arm/compv_intrin_neon.h"
#include "compv/base/image/compv_image_conv_common.h"
#include "compv/base/compv_simd_globals.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

#define yuv422family_to_y_NEON(vecMask) \
	compv_uscalar_t i, j, maxI = ((width + 15) & -16), padY = (stride - maxI), padYUV = padY << 1; \
	uint8x8x2_t vec0, vec1; \
	for (j = 0; j < height; ++j) { \
		for (i = 0; i < width; i += 16) { \
			vec0 = { { vld1_u8(yuv422Ptr + 0), vld1_u8(yuv422Ptr + 8) } }; \
			vec1 = { { vld1_u8(yuv422Ptr + 16), vld1_u8(yuv422Ptr + 24) } }; \
			vst1q_u8(outYPtr, vcombine_u8(vtbx2_u8(vec0.val[0], vec0, vecMask), vtbx2_u8(vec1.val[0], vec1, vecMask))); \
			outYPtr += 16; \
			yuv422Ptr += 32; \
		} \
		outYPtr += padY; \
		yuv422Ptr += padYUV; \
	}

	void CompVImageConvYuyv422_to_y_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* yuv422Ptr, COMPV_ALIGNED(NEON) uint8_t* outYPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	const uint8x8_t vecMask = vld1_u8(reinterpret_cast<const uint8_t*>(kShuffleEpi8_Yuyv422ToYuv_i32)); // indexes for the Y planar only (8 first)
	yuv422family_to_y_NEON(vecMask);
}

void CompVImageConvUyvy422_to_y_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* yuv422Ptr, COMPV_ALIGNED(NEON) uint8_t* outYPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	const uint8x8_t vecMask = vld1_u8(reinterpret_cast<const uint8_t*>(kShuffleEpi8_Uyvy422ToYuv_i32)); // indexes for the Y planar only (8 first)
	yuv422family_to_y_NEON(vecMask);
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_ARM && COMPV_INTRINSIC */
