/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/intrinsics/x86/features/orb/compv_feature_orb_desc_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/intrinsics/x86/compv_intrin_sse.h"
#include "compv/compv_simd_globals.h"
#include "compv/math/compv_math_utils.h"
#include "compv/compv_bits.h"
#include "compv/compv_cpu.h"

COMPV_EXTERNC const COMPV_ALIGN_DEFAULT() float kBrief256Pattern31AX[256];
COMPV_EXTERNC const COMPV_ALIGN_DEFAULT() float kBrief256Pattern31AY[256];
COMPV_EXTERNC const COMPV_ALIGN_DEFAULT() float kBrief256Pattern31BX[256];
COMPV_EXTERNC const COMPV_ALIGN_DEFAULT() float kBrief256Pattern31BY[256];
#if 0 // COMPV_FEATURE_DESC_ORB_FXP_DESC
COMPV_EXTERNC const COMPV_ALIGN_DEFAULT() int16_t kBrief256Pattern31AXFxp[256];
COMPV_EXTERNC const COMPV_ALIGN_DEFAULT() int16_t kBrief256Pattern31AYFxp[256];
COMPV_EXTERNC const COMPV_ALIGN_DEFAULT() int16_t kBrief256Pattern31BXFxp[256];
COMPV_EXTERNC const COMPV_ALIGN_DEFAULT() int16_t kBrief256Pattern31BYFxp[256];
#endif

COMPV_NAMESPACE_BEGIN()

// TODO(dmi): add ASM version
void Brief256_31_Intrin_SSE2(const uint8_t* img_center, compv_scalar_t img_stride, const float* cos1, const float* sin1, COMPV_ALIGNED(SSE) void* out)
{
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // SSE41, ASM
    int i, u8_index;
    COMPV_ALIGN_SSE() int32_t xmmIndex[4];
    COMPV_ALIGN_SSE() uint8_t xmmA[16];
    COMPV_ALIGN_SSE() uint8_t xmmB[16];
    __m128i xmmX, xmmY, xmmStride, xmmR, xmm128;

    __m128 xmmCosT, xmmSinT, xmmXF, xmmYF;

    uint16_t* outPtr = (uint16_t*)out;

    const float* Brief256Pattern31AX = &kBrief256Pattern31AX[0];
    const float* Brief256Pattern31AY = &kBrief256Pattern31AY[0];
    const float* Brief256Pattern31BX = &kBrief256Pattern31BX[0];
    const float* Brief256Pattern31BY = &kBrief256Pattern31BY[0];

    _mm_store_si128(&xmm128, _mm_load_si128((__m128i*)k128_u8));
    _mm_store_si128(&xmmStride, _mm_set1_epi32((int)img_stride));
    _mm_store_ps((float*)&xmmCosT, _mm_set1_ps(*cos1));
    _mm_store_ps((float*)&xmmSinT, _mm_set1_ps(*sin1));

    u8_index = 0;

    for (i = 0; i < 256; i += 4) {
        // xf = (kBrief256Pattern31AX[i] * cosT - kBrief256Pattern31AY[i] * sinT);
        _mm_store_ps((float*)&xmmXF, _mm_sub_ps(_mm_mul_ps(_mm_load_ps(Brief256Pattern31AX), xmmCosT), _mm_mul_ps(_mm_load_ps(Brief256Pattern31AY), xmmSinT)));
        // yf = (kBrief256Pattern31AX[i] * sinT + kBrief256Pattern31AY[i] * cosT);
        _mm_store_ps((float*)&xmmYF, _mm_add_ps(_mm_mul_ps(_mm_load_ps(Brief256Pattern31AX), xmmSinT), _mm_mul_ps(_mm_load_ps(Brief256Pattern31AY), xmmCosT)));
        // x = COMPV_MATH_ROUNDF_2_INT(xf, int);
        _mm_store_si128(&xmmX, _mm_cvtps_epi32(xmmXF));
        // y = COMPV_MATH_ROUNDF_2_INT(yf, int);
        _mm_store_si128(&xmmY, _mm_cvtps_epi32(xmmYF));
        // a = img_center[(y * img_stride) + x];
        _mm_store_si128((__m128i*)xmmIndex, _mm_add_epi32(_mm_mullo_epi32_SSE2(xmmY, xmmStride), xmmX)); // _mm_mullo_epi32 is SSE4.1
        xmmA[u8_index + 0] = img_center[xmmIndex[0]];
        xmmA[u8_index + 1] = img_center[xmmIndex[1]];
        xmmA[u8_index + 2] = img_center[xmmIndex[2]];
        xmmA[u8_index + 3] = img_center[xmmIndex[3]];

        // xf = (kBrief256Pattern31BX[i] * cosT - kBrief256Pattern31BY[i] * sinT);
        _mm_store_ps((float*)&xmmXF, _mm_sub_ps(_mm_mul_ps(_mm_load_ps(Brief256Pattern31BX), xmmCosT), _mm_mul_ps(_mm_load_ps(Brief256Pattern31BY), xmmSinT)));
        // yf = (kBrief256Pattern31BX[i] * sinT + kBrief256Pattern31BY[i] * cosT);
        _mm_store_ps((float*)&xmmYF, _mm_add_ps(_mm_mul_ps(_mm_load_ps(Brief256Pattern31BX), xmmSinT), _mm_mul_ps(_mm_load_ps(Brief256Pattern31BY), xmmCosT)));
        // x = COMPV_MATH_ROUNDF_2_INT(xf, int);
        _mm_store_si128(&xmmX, _mm_cvtps_epi32(xmmXF));
        // y = COMPV_MATH_ROUNDF_2_INT(yf, int);
        _mm_store_si128(&xmmY, _mm_cvtps_epi32(xmmYF));
        // b = img_center[(y * img_stride) + x];
        _mm_store_si128((__m128i*)xmmIndex, _mm_add_epi32(_mm_mullo_epi32_SSE2(xmmY, xmmStride), xmmX)); // _mm_mullo_epi32 is SSE4.1
        xmmB[u8_index + 0] = img_center[xmmIndex[0]];
        xmmB[u8_index + 1] = img_center[xmmIndex[1]];
        xmmB[u8_index + 2] = img_center[xmmIndex[2]];
        xmmB[u8_index + 3] = img_center[xmmIndex[3]];

        if ((u8_index += 4) == 16) {
            // _out[0] |= (a < b) ? (u64_1 << j) : 0;
            _mm_store_si128(&xmmR, _mm_cmplt_epi8(_mm_sub_epi8(_mm_load_si128((__m128i*)xmmA), xmm128), _mm_sub_epi8(_mm_load_si128((__m128i*)xmmB), xmm128))); // _mm_cmplt_epu8 doesn't exist
            *outPtr = _mm_movemask_epi8(xmmR);

            u8_index = 0;
            outPtr += 1;
        }

        Brief256Pattern31AX += 4;
        Brief256Pattern31AY += 4;
        Brief256Pattern31BX += 4;
        Brief256Pattern31BY += 4;
    }
}

// This function not used yet and not fully tested
#if 0 // COMPV_FEATURE_DESC_ORB_FXP_DESC
void Brief256_31_Fxpq16_Intrin_SSE2(const uint8_t* img_center, compv_scalar_t img_stride, const int16_t* cos1, const int16_t* sin1, COMPV_ALIGNED(SSE) void* out)
{
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // ASM, FMA3, SSE41

    int i;
    __m128i xmmCosT, xmmSinT, xmmStride, xmmR, xmm128, xmmX0, xmmY0, xmmTemp, xmmZero;
    COMPV_ALIGN_SSE() int32_t xmmIndex[4];
    COMPV_ALIGN_SSE() uint8_t xmmA[16];
    COMPV_ALIGN_SSE() uint8_t xmmB[16];
    __m128i xmmX[2], xmmY[2];
    uint16_t* outPtr = (uint16_t*)out;
	
    xmm128 = _mm_load_si128((__m128i*)k128_u8);
    xmmCosT = _mm_set1_epi16(*cos1);
    xmmSinT = _mm_set1_epi16(*sin1);
    xmmStride = _mm_set1_epi32((int)img_stride);

    const int16_t* Brief256Pattern31AX = &kBrief256Pattern31AXFxp[0];
    const int16_t* Brief256Pattern31AY = &kBrief256Pattern31AYFxp[0];
    const int16_t* Brief256Pattern31BX = &kBrief256Pattern31BXFxp[0];
    const int16_t* Brief256Pattern31BY = &kBrief256Pattern31BYFxp[0];

    // TODO: ASM, use "_mm_cvtepi16_epi32", SSE4.1

    for (i = 0; i < 256; i += 16) {
        /********* A-[0 - 7] ************/
        xmmX0 = _mm_load_si128((const __m128i*)(Brief256Pattern31AX + i));
        xmmY0 = _mm_load_si128((const __m128i*)(Brief256Pattern31AY + i));

        // "cosT" within [-0x7fff, 0x7fff]
        // "Brief256Pattern31" within [-30, 30]
        // -> "((Brief256Pattern31 * cosT) >> 16)" +- "((Brief256Pattern31 * cosT) >> 16)" whithin [-0x7fff, 0x7fff]
        // -> Do not convert from I16 to I32 for the sub/add

        xmmTemp = _mm_sub_epi16(_mm_mulhi_epi16(xmmX0, xmmCosT), _mm_mulhi_epi16(xmmY0, xmmSinT));
        xmmX[0] = _mm_cvtepi16_epi32_low_SSE2(xmmTemp);
        xmmX[1] = _mm_cvtepi16_epi32_hi_SSE2(xmmTemp);
        xmmTemp = _mm_add_epi16(_mm_mulhi_epi16(xmmX0, xmmSinT), _mm_mulhi_epi16(xmmY0, xmmCosT));
        xmmY[0] = _mm_cvtepi16_epi32_low_SSE2(xmmTemp);
        xmmY[1] = _mm_cvtepi16_epi32_hi_SSE2(xmmTemp);
        _mm_store_si128((__m128i*)xmmIndex, _mm_add_epi32(_mm_mullo_epi32_SSE2(xmmY[0], xmmStride), xmmX[0]));
        xmmA[0] = img_center[xmmIndex[0]];
        xmmA[1] = img_center[xmmIndex[1]];
        xmmA[2] = img_center[xmmIndex[2]];
        xmmA[3] = img_center[xmmIndex[3]];
        _mm_store_si128((__m128i*)xmmIndex, _mm_add_epi32(_mm_mullo_epi32_SSE2(xmmY[1], xmmStride), xmmX[1]));
        xmmA[4] = img_center[xmmIndex[0]];
        xmmA[5] = img_center[xmmIndex[1]];
        xmmA[6] = img_center[xmmIndex[2]];
        xmmA[7] = img_center[xmmIndex[3]];

        /********* A-[8 - 15] ************/
        xmmX0 = _mm_load_si128((const __m128i*)(Brief256Pattern31AX + i + 8));
        xmmY0 = _mm_load_si128((const __m128i*)(Brief256Pattern31AY + i + 8));

        xmmTemp = _mm_sub_epi16(_mm_mulhi_epi16(xmmX0, xmmCosT), _mm_mulhi_epi16(xmmY0, xmmSinT));
        xmmX[0] = _mm_cvtepi16_epi32_low_SSE2(xmmTemp);
        xmmX[1] = _mm_cvtepi16_epi32_hi_SSE2(xmmTemp);
        xmmTemp = _mm_add_epi16(_mm_mulhi_epi16(xmmX0, xmmSinT), _mm_mulhi_epi16(xmmY0, xmmCosT));
        xmmY[0] = _mm_cvtepi16_epi32_low_SSE2(xmmTemp);
        xmmY[1] = _mm_cvtepi16_epi32_hi_SSE2(xmmTemp);
        _mm_store_si128((__m128i*)xmmIndex, _mm_add_epi32(_mm_mullo_epi32_SSE2(xmmY[0], xmmStride), xmmX[0]));
        xmmA[8] = img_center[xmmIndex[0]];
        xmmA[9] = img_center[xmmIndex[1]];
        xmmA[10] = img_center[xmmIndex[2]];
        xmmA[11] = img_center[xmmIndex[3]];
        _mm_store_si128((__m128i*)xmmIndex, _mm_add_epi32(_mm_mullo_epi32_SSE2(xmmY[1], xmmStride), xmmX[1]));
        xmmA[12] = img_center[xmmIndex[0]];
        xmmA[13] = img_center[xmmIndex[1]];
        xmmA[14] = img_center[xmmIndex[2]];
        xmmA[15] = img_center[xmmIndex[3]];

        /********* B-[0 - 7] ************/
        xmmX0 = _mm_load_si128((const __m128i*)(Brief256Pattern31BX + i));
        xmmY0 = _mm_load_si128((const __m128i*)(Brief256Pattern31BY + i));

        xmmTemp = _mm_sub_epi16(_mm_mulhi_epi16(xmmX0, xmmCosT), _mm_mulhi_epi16(xmmY0, xmmSinT));
        xmmX[0] = _mm_cvtepi16_epi32_low_SSE2(xmmTemp);
        xmmX[1] = _mm_cvtepi16_epi32_hi_SSE2(xmmTemp);
        xmmTemp = _mm_add_epi16(_mm_mulhi_epi16(xmmX0, xmmSinT), _mm_mulhi_epi16(xmmY0, xmmCosT));
        xmmY[0] = _mm_cvtepi16_epi32_low_SSE2(xmmTemp);
        xmmY[1] = _mm_cvtepi16_epi32_hi_SSE2(xmmTemp);
        _mm_store_si128((__m128i*)xmmIndex, _mm_add_epi32(_mm_mullo_epi32_SSE2(xmmY[0], xmmStride), xmmX[0]));
        xmmB[0] = img_center[xmmIndex[0]];
        xmmB[1] = img_center[xmmIndex[1]];
        xmmB[2] = img_center[xmmIndex[2]];
        xmmB[3] = img_center[xmmIndex[3]];
        _mm_store_si128((__m128i*)xmmIndex, _mm_add_epi32(_mm_mullo_epi32_SSE2(xmmY[1], xmmStride), xmmX[1]));
        xmmB[4] = img_center[xmmIndex[0]];
        xmmB[5] = img_center[xmmIndex[1]];
        xmmB[6] = img_center[xmmIndex[2]];
        xmmB[7] = img_center[xmmIndex[3]];

        /********* B-[8 - 15] ************/
        xmmX0 = _mm_load_si128((const __m128i*)(Brief256Pattern31BX + i + 8));
        xmmY0 = _mm_load_si128((const __m128i*)(Brief256Pattern31BY + i + 8));

        xmmTemp = _mm_sub_epi16(_mm_mulhi_epi16(xmmX0, xmmCosT), _mm_mulhi_epi16(xmmY0, xmmSinT));
        xmmX[0] = _mm_cvtepi16_epi32_low_SSE2(xmmTemp);
        xmmX[1] = _mm_cvtepi16_epi32_hi_SSE2(xmmTemp);
        xmmTemp = _mm_add_epi16(_mm_mulhi_epi16(xmmX0, xmmSinT), _mm_mulhi_epi16(xmmY0, xmmCosT));
        xmmY[0] = _mm_cvtepi16_epi32_low_SSE2(xmmTemp);
        xmmY[1] = _mm_cvtepi16_epi32_hi_SSE2(xmmTemp);
        _mm_store_si128((__m128i*)xmmIndex, _mm_add_epi32(_mm_mullo_epi32_SSE2(xmmY[0], xmmStride), xmmX[0]));
        xmmB[8] = img_center[xmmIndex[0]];
        xmmB[9] = img_center[xmmIndex[1]];
        xmmB[10] = img_center[xmmIndex[2]];
        xmmB[11] = img_center[xmmIndex[3]];
        _mm_store_si128((__m128i*)xmmIndex, _mm_add_epi32(_mm_mullo_epi32_SSE2(xmmY[1], xmmStride), xmmX[1]));
        xmmB[12] = img_center[xmmIndex[0]];
        xmmB[13] = img_center[xmmIndex[1]];
        xmmB[14] = img_center[xmmIndex[2]];
        xmmB[15] = img_center[xmmIndex[3]];

        /********* Result ************/
        _mm_store_si128(&xmmR, _mm_cmplt_epi8(_mm_sub_epi8(_mm_load_si128((__m128i*)xmmA), xmm128), _mm_sub_epi8(_mm_load_si128((__m128i*)xmmB), xmm128))); // _mm_cmplt_epu8 doesn't exist
        *outPtr++ = (uint16_t)_mm_movemask_epi8(xmmR);
    }
}
#endif

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
