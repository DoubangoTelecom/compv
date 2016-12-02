/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/intrinsics/x86/image/conv/compv_imageconv_rgba_rgb_intrin_ssse3.h"

#if defined(COMPV_ARCH_X86) && defined(COMPV_INTRINSIC)
#include "compv/image/conv/compv_imageconv_common.h"
#include "compv/compv_simd_globals.h"

COMPV_NAMESPACE_BEGIN()

void rgbToRgbaKernel31_Intrin_Aligned_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* rgb, COMPV_ALIGNED(SSE) uint8_t* rgba, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride)
{
    COMPV_DEBUG_INFO_CHECK_SSSE3();
    __m128i xmm0, xmm1, xmmMaskRgbToRgba, xmmAlpha;
    compv_scalar_t i, j, maxI = ((width + 15) & -16), pad = (stride - maxI), padRGB = pad * 3, padRGBA = pad << 2;

    _mm_store_si128(&xmmAlpha, _mm_load_si128((__m128i*)k_0_0_0_255_u8)); // alpha add to the 4th bytes - if rga is an intermediate format then do not care
    _mm_store_si128(&xmmMaskRgbToRgba, _mm_load_si128((__m128i*)kShuffleEpi8_RgbToRgba_i32));

    for (j = 0; j < height; ++j) {
        for (i = 0; i < width; i += 16) {
            /**  convert from RGB to RGBA **/
            // RGBA0 = Convert(RGB0) -> 4RGBAs which means we used 4RGBs = 12bytes and lost 4bytes from RGB0
            // RGBA1 = Convert(ALIGN(RGB0, RGB1, 12)) -> we used 4bytes from RGB0 and 8bytes from RGB1 = 12bytes RGB = 16bytes RGBA -> lost 12bytes from RGB1
            // RGBA2 = Convert(ALIGN(RGB1, RGB2, 8)) -> we used 8bytes from RGB1 and 4bytes from RGB2 = 12bytes RGB = 16bytes RGBA -> lost 12bytes from RGB2
            // RGBA3 = Convert(ALIGN(RGB2, RGB2, 4)) -> used 12bytes from RGB2 = 12bytes RGB = 16bytes RGBA
            _mm_store_si128(&xmm0, _mm_load_si128((__m128i*)rgb)); // load first 16 samples
            _mm_store_si128(&xmm1, _mm_load_si128((__m128i*)(rgb + 16))); // load next 16 samples
            _mm_store_si128((__m128i*)(rgba + 0), _mm_add_epi8(_mm_shuffle_epi8(xmm0, xmmMaskRgbToRgba), xmmAlpha));
            _mm_store_si128((__m128i*)(rgba + 16), _mm_add_epi8(_mm_shuffle_epi8(_mm_alignr_epi8(xmm1, xmm0, 12), xmmMaskRgbToRgba), xmmAlpha));
            _mm_store_si128(&xmm0, _mm_load_si128((__m128i*)(rgb + 32))); // load next 16 samples
            _mm_store_si128((__m128i*)(rgba + 32), _mm_add_epi8(_mm_shuffle_epi8(_mm_alignr_epi8(xmm0, xmm1, 8), xmmMaskRgbToRgba), xmmAlpha));
            _mm_store_si128((__m128i*)(rgba + 48), _mm_add_epi8(_mm_shuffle_epi8(_mm_alignr_epi8(xmm0, xmm0, 4), xmmMaskRgbToRgba), xmmAlpha));

            rgb += 48;
            rgba += 64;
        }
        rgb += padRGB;
        rgba += padRGBA;
    }
}

void bgrToBgraKernel31_Intrin_Aligned_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* bgr, COMPV_ALIGNED(SSE) uint8_t* bgra, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride)
{
    // the alpha channel is at the same index as rgb->rgba which means we can use the same function
    rgbToRgbaKernel31_Intrin_Aligned_SSSE3(bgr, bgra, height, width, stride);
}

COMPV_NAMESPACE_END()

#endif /* defined(COMPV_ARCH_X86) && defined(COMPV_INTRINSIC) */
