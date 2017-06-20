/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/intrin/x86/compv_image_conv_hsv_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/intrin/x86/compv_intrin_sse.h"
#include "compv/base/image/compv_image_conv_common.h"
#include "compv/base/compv_simd_globals.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// width must be >= 16
void CompVImageConvRgba32ToHsv_Intrin_SSE2(COMPV_ALIGNED(SSE) const uint8_t* rgba32Ptr, COMPV_ALIGNED(SSE) uint8_t* hsvPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();

	compv_uscalar_t i, j;
	const compv_uscalar_t strideInBytes = (stride << 2);
	__m128i vecR, vecG, vecB, vecA;
	__m128i vec0;

	width <<= 2; // from samples to bytes
	stride <<= 2; // from samples to bytes

	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 64) { // 64 = (16 * 4)
			/* Load samples (TODO(dmi): for ARM neon use vld4.u8) */
			vecR = _mm_load_si128(reinterpret_cast<const __m128i*>(&rgba32Ptr[i + 0]));
			vecG = _mm_load_si128(reinterpret_cast<const __m128i*>(&rgba32Ptr[i + 16]));
			vecB = _mm_load_si128(reinterpret_cast<const __m128i*>(&rgba32Ptr[i + 32]));
			vecA = _mm_load_si128(reinterpret_cast<const __m128i*>(&rgba32Ptr[i + 48]));
			
			// Transpose
			COMPV_TRANSPOSE_I8_4X16_SSE2(vecR, vecG, vecB, vecA, vec0);

			
		} // End_Of for (i = 0; i < width; i += 64)
		rgba32Ptr += stride;
		hsvPtr += stride;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
