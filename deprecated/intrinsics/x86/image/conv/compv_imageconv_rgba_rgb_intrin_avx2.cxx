/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/intrinsics/x86/image/conv/compv_imageconv_rgba_rgb_intrin_avx2.h"

#if defined(COMPV_ARCH_X86) && defined(COMPV_INTRINSIC)
#include "compv/intrinsics/x86/compv_intrin_avx.h"
#include "compv/image/conv/compv_imageconv_common.h"
#include "compv/compv_simd_globals.h"

COMPV_NAMESPACE_BEGIN()

#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx
#endif
void rgbToRgbaKernel31_Intrin_Aligned_AVX2(COMPV_ALIGNED(AVX2) const uint8_t* rgb, COMPV_ALIGNED(AVX2) uint8_t* rgba, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride)
{
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // Use ASM
    COMPV_DEBUG_INFO_CHECK_AVX2();

    _mm256_zeroupper();
    __m256i ymm0, ymm1, ymmABCDDEFG, ymmCDEFFGHX, ymmMaskRgbToRgba, ymmXXABBCDE, ymmLost, ymmAlpha;
    compv_scalar_t i, j, maxI = ((width + 31) & -32), pad = (stride - maxI), padRGB = pad * 3, padRGBA = pad << 2;

    _mm256_store_si256(&ymmAlpha, _mm256_load_si256((__m256i*)k_0_0_0_255_u8)); // alpha add to the 4th bytes - if rga is an intermediate format then do not care
    _mm256_store_si256(&ymmMaskRgbToRgba, _mm256_load_si256((__m256i*)kShuffleEpi8_RgbToRgba_i32));
    _mm256_store_si256(&ymmABCDDEFG, _mm256_load_si256((__m256i*)kAVXPermutevar8x32_ABCDDEFG_i32));
    _mm256_store_si256(&ymmCDEFFGHX, _mm256_load_si256((__m256i*)kAVXPermutevar8x32_CDEFFGHX_i32));
    _mm256_store_si256(&ymmXXABBCDE, _mm256_load_si256((__m256i*)kAVXPermutevar8x32_XXABBCDE_i32));

    // Y = (((33 * R) + (65 * G) + (13 * B))) >> 7 + 16
    for (j = 0; j < height; ++j) {
        for (i = 0; i < width; i += 32) {
            ///////////// Line-0 /////////////
            _mm256_store_si256(&ymm0, _mm256_load_si256((__m256i*)(rgb + 0))); // load first 32 samples
            _mm256_store_si256(&ymm1, _mm256_permutevar8x32_epi32(ymm0, ymmABCDDEFG)); // move the last 4bytes in the first 128-lane to the second 128-lane
            _mm256_store_si256((__m256i*)(rgba + 0), _mm256_add_epi8(_mm256_shuffle_epi8(ymm1, ymmMaskRgbToRgba), ymmAlpha)); // RGB -> RGBA

            ///////////// Line-1 /////////////
            _mm256_store_si256(&ymm1, _mm256_load_si256((__m256i*)(rgb + 32))); // load next 32 samples
            _mm256_store_si256(&ymm0, _mm256_permute4x64_epi64(ymm0, COMPV_MM_SHUFFLE(3, 3, 3, 3))); // duplicate lost0
            _mm256_store_si256(&ymmLost, _mm256_broadcastsi128_si256(_mm256_extractf128_si256(ymm1, 1))); // high-128 = low-lost = lost0 || lost1
            _mm256_store_si256(&ymm1, _mm256_permutevar8x32_epi32(ymm1, ymmXXABBCDE));
            _mm256_store_si256(&ymm1, _mm256_blend_epi32(ymm1, ymm0, 0x03)); // ymm0(64bits)||ymm1(192bits)
            _mm256_store_si256((__m256i*)(rgba + 32), _mm256_add_epi8(_mm256_shuffle_epi8(ymm1, ymmMaskRgbToRgba), ymmAlpha)); // RGB -> RGBA

            ///////////// Line-2 /////////////
            _mm256_store_si256(&ymm0, _mm256_load_si256((__m256i*)(rgb + 64))); // load next 32 samples
            _mm256_store_si256(&ymm1, _mm256_permutevar8x32_epi32(ymm0, ymmCDEFFGHX)); // lost0 || lost1 || lost2 || garbage
            _mm256_store_si256(&ymmLost, _mm256_inserti128_si256(ymmLost, _mm256_extractf128_si256(ymm0, 0), 1)); // lost0 || lost1 || 0 || 1
            _mm256_store_si256(&ymm0, _mm256_permutevar8x32_epi32(ymmLost, ymmABCDDEFG));
            _mm256_store_si256((__m256i*)(rgba + 64), _mm256_add_epi8(_mm256_shuffle_epi8(ymm0, ymmMaskRgbToRgba), ymmAlpha)); // RGB -> RGBA

            ///////////// Line-3 /////////////
            _mm256_store_si256((__m256i*)(rgba + 96), _mm256_add_epi8(_mm256_shuffle_epi8(ymm1, ymmMaskRgbToRgba), ymmAlpha)); // RGB -> RGBA

            rgb += 96;
            rgba += 128;
        }
        rgb += padRGB;
        rgba += padRGBA;
    }
    _mm256_zeroupper();
}

#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx
#endif
void bgrToBgraKernel31_Intrin_Aligned_AVX2(COMPV_ALIGNED(AVX2) const uint8_t* bgr, COMPV_ALIGNED(AVX2) uint8_t* bgra, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride)
{
    // the alpha channel is at the same index as rgb->rgba which means we can use the same function
    rgbToRgbaKernel31_Intrin_Aligned_AVX2(bgr, bgra, height, width, stride);
}

COMPV_NAMESPACE_END()

#endif /* defined(COMPV_ARCH_X86) && defined(COMPV_INTRINSIC) */
