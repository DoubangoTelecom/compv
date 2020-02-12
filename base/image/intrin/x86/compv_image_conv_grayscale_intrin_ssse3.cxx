/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/intrin/x86/compv_image_conv_grayscale_intrin_ssse3.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/intrin/x86/compv_intrin_sse.h"
#include "compv/base/image/compv_image_conv_common.h"
#include "compv/base/compv_simd_globals.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_debug.h"


COMPV_NAMESPACE_BEGIN()

#define yuv422family_to_y_SSSE3(xmmMask) \
	compv_uscalar_t i, j, maxI = ((width + 15) & -16), padY = (stride - maxI), padYUV = padY << 1; \
	for (j = 0; j < height; ++j) { \
		for (i = 0; i < width; i += 16) { \
			_mm_store_si128(reinterpret_cast<__m128i*>(outYPtr), \
				_mm_unpacklo_epi64( \
					_mm_shuffle_epi8(_mm_load_si128(reinterpret_cast<const __m128i*>(yuv422Ptr + 0)), xmmMask), \
					_mm_shuffle_epi8(_mm_load_si128(reinterpret_cast<const __m128i*>(yuv422Ptr + 16)), xmmMask) \
				) \
			); \
			outYPtr += 16; \
			yuv422Ptr += 32; \
		} \
		outYPtr += padY; \
		yuv422Ptr += padYUV; \
	}

void CompVImageConvYuyv422_to_y_Intrin_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* yuv422Ptr, COMPV_ALIGNED(SSE) uint8_t* outYPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_SSSE3();
	const __m128i xmmMask = _mm_load_si128(reinterpret_cast<const __m128i*>(kShuffleEpi8_Yuyv422ToYuv_i32)); // Packed yuyv422 -> planar Y(8 samples), U(4 samples), V(4 samples)
	yuv422family_to_y_SSSE3(xmmMask);
}

void CompVImageConvUyvy422_to_y_Intrin_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* yuv422Ptr, COMPV_ALIGNED(SSE) uint8_t* outYPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_SSSE3();
	const __m128i xmmMask = _mm_load_si128(reinterpret_cast<const __m128i*>(kShuffleEpi8_Uyvy422ToYuv_i32)); // Packed uyvy422 -> planar Y(8 samples), U(4 samples), V(4 samples)
	yuv422family_to_y_SSSE3(xmmMask);
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
