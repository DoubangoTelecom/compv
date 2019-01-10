/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/x86/compv_math_utils_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/intrin/x86/compv_intrin_sse.h"
#include "compv/base/compv_simd_globals.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

void CompVMathUtilsMax_16u_Intrin_SSE2(COMPV_ALIGNED(SSE) const uint16_t* data, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride, uint16_t *max)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Use SSE41 version in ASM code");
	COMPV_DEBUG_INFO_CHECK_SSE2();
	compv_scalar_t widthSigned = static_cast<compv_scalar_t>(width);
	compv_scalar_t i, orphans = (widthSigned & 7); // in short
	__m128i vec0, vec1, vec2, vec3;
	__m128i vecMax = _mm_setzero_si128();
	__m128i vecOrphansSuppress = _mm_setzero_si128(); // not needed, just to stop compiler warnings about unset variable
	const __m128i vecMask = _mm_set1_epi16((int16_t)0x8000);
	if (orphans) {
		compv_scalar_t orphansInBits = ((8 - orphans) << 4); // convert to bits
		COMPV_ALIGN_SSE() uint32_t memOrphans[4];
		_mm_store_si128(reinterpret_cast<__m128i*>(memOrphans), _mm_cmpeq_epi16(vecMax, vecMax)); // 0xffff...fff
		uint32_t* memOrphansPtr = &memOrphans[3];
		while (orphansInBits >= 0) *memOrphansPtr-- = (orphansInBits > 31 ? 0 : (0xffffffff >> orphansInBits)), orphansInBits -= 32;
		vecOrphansSuppress = _mm_load_si128(reinterpret_cast<const __m128i*>(memOrphans));
	}
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (i = 0; i < widthSigned - 31; i += 32) {
			vec0 = _mm_load_si128(reinterpret_cast<const __m128i*>(&data[i + 0]));
			vec1 = _mm_load_si128(reinterpret_cast<const __m128i*>(&data[i + 8]));
			vec2 = _mm_load_si128(reinterpret_cast<const __m128i*>(&data[i + 16]));
			vec3 = _mm_load_si128(reinterpret_cast<const __m128i*>(&data[i + 24]));
			vec0 = _mm_max_epu16_SS2(vec0, vec1, vecMask);
			vec2 = _mm_max_epu16_SS2(vec2, vec3, vecMask);
			vecMax = _mm_max_epu16_SS2(vecMax, vec0, vecMask);
			vecMax = _mm_max_epu16_SS2(vecMax, vec2, vecMask);
		}
		for (; i < widthSigned - 7; i += 8) {
			vec0 = _mm_load_si128(reinterpret_cast<const __m128i*>(&data[i + 0]));
			vecMax = _mm_max_epu16_SS2(vecMax, vec0, vecMask);
		}
		if (orphans) {
			vec0 = _mm_load_si128(reinterpret_cast<const __m128i*>(&data[i + 0]));
			vec0 = _mm_and_si128(vec0, vecOrphansSuppress);
			vecMax = _mm_max_epu16_SS2(vecMax, vec0, vecMask);
		}
		data += stride;
	}

	// Paiwise / hz max
	vec0 = _mm_srli_si128(vecMax, 8);
	vec1 = _mm_srli_si128(vecMax, 4);
	vec2 = _mm_srli_si128(vecMax, 2);
	vec0 = _mm_max_epu16_SS2(vec0, vec1, vecMask);
	vecMax = _mm_max_epu16_SS2(vecMax, vec2, vecMask);
	vecMax = _mm_max_epu16_SS2(vecMax, vec0, vecMask);
	
	*max = static_cast<uint16_t>(_mm_extract_epi16(vecMax, 0));
}

void CompVMathUtilsSum_8u32u_Intrin_SSE2(COMPV_ALIGNED(SSE) const uint8_t* data, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride, uint32_t *sum1)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	__m128i vecSuml = _mm_setzero_si128(), vecSumh = _mm_setzero_si128(), vec0, vec1, vec2, vec3, vecOrphansSuppress;
	const __m128i vecZero = _mm_setzero_si128();
	compv_scalar_t widthSigned = static_cast<compv_scalar_t>(width), i;
	compv_scalar_t orphans = (widthSigned & 15); // in bytes
	if (orphans) {
		compv_scalar_t orphansInBits = ((16 - orphans) << 3); // convert to bits
		COMPV_ALIGN_SSE() uint32_t memOrphans[4];
		_mm_store_si128(reinterpret_cast<__m128i*>(memOrphans), _mm_cmpeq_epi16(vecZero, vecZero)); // 0xffff...fff
		uint32_t* memOrphansPtr = &memOrphans[3];
		while (orphansInBits >= 0) *memOrphansPtr-- = (orphansInBits > 31 ? 0 : (0xffffffff >> orphansInBits)), orphansInBits -= 32;
		vecOrphansSuppress = _mm_load_si128(reinterpret_cast<const __m128i*>(memOrphans));
	}
	for (compv_uscalar_t j = 0; j < height; ++j) {
		// conversion from "uint8" to "uint16" using unpack will lose sign -> doesn't work with "signed int8"
		// SSE4.1 "_mm_cvtepi8_epi16" would work
		// Same remark apply to conversion from epi16 to epi32
		for (i = 0; i < widthSigned - 63; i += 64) {
			vec0 = _mm_load_si128(reinterpret_cast<const __m128i*>(&data[i]));
			vec1 = _mm_load_si128(reinterpret_cast<const __m128i*>(&data[i + 16]));
			vec2 = _mm_load_si128(reinterpret_cast<const __m128i*>(&data[i + 32]));
			vec3 = _mm_load_si128(reinterpret_cast<const __m128i*>(&data[i + 48]));
			vec0 = _mm_add_epi16(_mm_unpacklo_epi8(vec0, vecZero), _mm_unpackhi_epi8(vec0, vecZero));
			vec1 = _mm_add_epi16(_mm_unpacklo_epi8(vec1, vecZero), _mm_unpackhi_epi8(vec1, vecZero));
			vec2 = _mm_add_epi16(_mm_unpacklo_epi8(vec2, vecZero), _mm_unpackhi_epi8(vec2, vecZero));
			vec3 = _mm_add_epi16(_mm_unpacklo_epi8(vec3, vecZero), _mm_unpackhi_epi8(vec3, vecZero));
			vec0 = _mm_add_epi16(vec0, vec1);
			vec2 = _mm_add_epi16(vec2, vec3);
			vec0 = _mm_add_epi16(vec0, vec2);
			// use vecSuml and vecSumh to break dependency
			vecSuml = _mm_add_epi32(vecSuml, _mm_unpacklo_epi16(vec0, vecZero));
			vecSumh = _mm_add_epi32(vecSumh, _mm_unpackhi_epi16(vec0, vecZero));
		}
		for (; i < widthSigned - 15; i += 16) {
			vec0 = _mm_load_si128(reinterpret_cast<const __m128i*>(&data[i]));
			vec0 = _mm_add_epi16(_mm_unpacklo_epi8(vec0, vecZero), _mm_unpackhi_epi8(vec0, vecZero));
			vecSuml = _mm_add_epi32(vecSuml, _mm_unpacklo_epi16(vec0, vecZero));
			vecSumh = _mm_add_epi32(vecSumh, _mm_unpackhi_epi16(vec0, vecZero));
		}
		if (orphans) {
			vec0 = _mm_load_si128(reinterpret_cast<const __m128i*>(&data[i]));
			vec0 = _mm_and_si128(vec0, vecOrphansSuppress);
			vec0 = _mm_add_epi16(_mm_unpacklo_epi8(vec0, vecZero), _mm_unpackhi_epi8(vec0, vecZero));
			vecSuml = _mm_add_epi32(vecSuml, _mm_unpacklo_epi16(vec0, vecZero));
			vecSumh = _mm_add_epi32(vecSumh, _mm_unpackhi_epi16(vec0, vecZero));
		}
		data += stride;
	}

	vecSuml = _mm_add_epi32(vecSuml, vecSumh);

	// Paiwise / hz add
	vecSuml = _mm_add_epi32(_mm_srli_si128(vecSuml, 8), vecSuml);
	vecSuml = _mm_add_epi32(_mm_srli_si128(vecSuml, 4), vecSuml);

	*sum1 = static_cast<uint32_t>(_mm_cvtsi128_si32(vecSuml));
}

#define __CompVMathUtilsSum2_16x1_32s32s_Intrin_SSE2(i) \
	_mm_store_si128(reinterpret_cast<__m128i*>(&s[i]), _mm_add_epi32(_mm_load_si128(reinterpret_cast<const __m128i*>(&a[i])), _mm_load_si128(reinterpret_cast<const __m128i*>(&b[i])))); \
	_mm_store_si128(reinterpret_cast<__m128i*>(&s[i + 4]), _mm_add_epi32(_mm_load_si128(reinterpret_cast<const __m128i*>(&a[i + 4])), _mm_load_si128(reinterpret_cast<const __m128i*>(&b[i + 4])))); \
	_mm_store_si128(reinterpret_cast<__m128i*>(&s[i + 8]), _mm_add_epi32(_mm_load_si128(reinterpret_cast<const __m128i*>(&a[i + 8])), _mm_load_si128(reinterpret_cast<const __m128i*>(&b[i + 8])))); \
	_mm_store_si128(reinterpret_cast<__m128i*>(&s[i + 12]), _mm_add_epi32(_mm_load_si128(reinterpret_cast<const __m128i*>(&a[i + 12])), _mm_load_si128(reinterpret_cast<const __m128i*>(&b[i + 12]))))


void CompVMathUtilsSum2_32s32s_Intrin_SSE2(COMPV_ALIGNED(SSE) const int32_t* a, COMPV_ALIGNED(SSE) const int32_t* b, COMPV_ALIGNED(SSE) int32_t* s, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride)
{
    COMPV_DEBUG_INFO_CHECK_SSE2();
    compv_uscalar_t j;
    compv_scalar_t i;
    compv_scalar_t width_ = static_cast<compv_scalar_t>(width);

    for (j = 0; j < height; ++j) {
        for (i = 0; i < width_ - 15; i += 16) {
			__CompVMathUtilsSum2_16x1_32s32s_Intrin_SSE2(i);
        }
        for (; i < width_; i += 4) {
            _mm_store_si128(reinterpret_cast<__m128i*>(&s[i]), _mm_add_epi32(_mm_load_si128(reinterpret_cast<const __m128i*>(&a[i])), _mm_load_si128(reinterpret_cast<const __m128i*>(&b[i]))));
        }
        a += stride;
        b += stride;
        s += stride;
    }
}

// width = 256, height = 1: common size (histogram 8u)
void CompVMathUtilsSum2_32s32s_256x1_Intrin_SSE2(COMPV_ALIGNED(SSE) const int32_t* a, COMPV_ALIGNED(SSE) const int32_t* b, COMPV_ALIGNED(SSE) int32_t* s, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride)
{
	__CompVMathUtilsSum2_16x1_32s32s_Intrin_SSE2(0); __CompVMathUtilsSum2_16x1_32s32s_Intrin_SSE2(16); __CompVMathUtilsSum2_16x1_32s32s_Intrin_SSE2(32); __CompVMathUtilsSum2_16x1_32s32s_Intrin_SSE2(48);
	__CompVMathUtilsSum2_16x1_32s32s_Intrin_SSE2(64); __CompVMathUtilsSum2_16x1_32s32s_Intrin_SSE2(80); __CompVMathUtilsSum2_16x1_32s32s_Intrin_SSE2(96); __CompVMathUtilsSum2_16x1_32s32s_Intrin_SSE2(112);
	__CompVMathUtilsSum2_16x1_32s32s_Intrin_SSE2(128); __CompVMathUtilsSum2_16x1_32s32s_Intrin_SSE2(144); __CompVMathUtilsSum2_16x1_32s32s_Intrin_SSE2(160); __CompVMathUtilsSum2_16x1_32s32s_Intrin_SSE2(176);
	__CompVMathUtilsSum2_16x1_32s32s_Intrin_SSE2(192); __CompVMathUtilsSum2_16x1_32s32s_Intrin_SSE2(208); __CompVMathUtilsSum2_16x1_32s32s_Intrin_SSE2(224); __CompVMathUtilsSum2_16x1_32s32s_Intrin_SSE2(240);
}

void CompVMathUtilsScaleAndClipPixel8_16u32f_Intrin_SSE2(COMPV_ALIGNED(SSE) const uint16_t* in, const compv_float32_t* scale1, COMPV_ALIGNED(SSE) uint8_t* out, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	compv_scalar_t i, widthSigned = static_cast<compv_scalar_t>(width);
	__m128i vec0, vec1, vec2, vec3;
	__m128 vec0f, vec1f, vec2f, vec3f, vec4f, vec5f, vec6f, vec7f;
	const __m128 vecScale = _mm_load1_ps(scale1);
	const __m128i vecZero = _mm_setzero_si128();

	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (i = 0; i < widthSigned - 31; i += 32) {
			vec0 = _mm_load_si128(reinterpret_cast<const __m128i*>(&in[i + 0]));
			vec1 = _mm_load_si128(reinterpret_cast<const __m128i*>(&in[i + 8]));
			vec2 = _mm_load_si128(reinterpret_cast<const __m128i*>(&in[i + 16]));
			vec3 = _mm_load_si128(reinterpret_cast<const __m128i*>(&in[i + 24]));
			vec0f = _mm_cvtepi32_ps(_mm_unpacklo_epi16(vec0, vecZero));
			vec1f = _mm_cvtepi32_ps(_mm_unpackhi_epi16(vec0, vecZero));
			vec2f = _mm_cvtepi32_ps(_mm_unpacklo_epi16(vec1, vecZero));
			vec3f = _mm_cvtepi32_ps(_mm_unpackhi_epi16(vec1, vecZero));
			vec4f = _mm_cvtepi32_ps(_mm_unpacklo_epi16(vec2, vecZero));
			vec5f = _mm_cvtepi32_ps(_mm_unpackhi_epi16(vec2, vecZero));
			vec6f = _mm_cvtepi32_ps(_mm_unpacklo_epi16(vec3, vecZero));
			vec7f = _mm_cvtepi32_ps(_mm_unpackhi_epi16(vec3, vecZero));
			vec0f = _mm_mul_ps(vec0f, vecScale);
			vec1f = _mm_mul_ps(vec1f, vecScale);
			vec2f = _mm_mul_ps(vec2f, vecScale);
			vec3f = _mm_mul_ps(vec3f, vecScale);
			vec4f = _mm_mul_ps(vec4f, vecScale);
			vec5f = _mm_mul_ps(vec5f, vecScale);
			vec6f = _mm_mul_ps(vec6f, vecScale);
			vec7f = _mm_mul_ps(vec7f, vecScale);
			vec0 = _mm_packs_epi32(_mm_cvttps_epi32(vec0f), _mm_cvttps_epi32(vec1f));
			vec1 = _mm_packs_epi32(_mm_cvttps_epi32(vec2f), _mm_cvttps_epi32(vec3f));
			vec2 = _mm_packs_epi32(_mm_cvttps_epi32(vec4f), _mm_cvttps_epi32(vec5f));
			vec3 = _mm_packs_epi32(_mm_cvttps_epi32(vec6f), _mm_cvttps_epi32(vec7f));
			vec0 = _mm_packus_epi16(vec0, vec1);
			vec1 = _mm_packus_epi16(vec2, vec3);
			_mm_store_si128(reinterpret_cast<__m128i*>(&out[i + 0]), vec0);
			_mm_store_si128(reinterpret_cast<__m128i*>(&out[i + 16]), vec1);
		}
		for (; i < widthSigned; i += 8) { // reading beyond width which means in and out must be strided
			vec0 = _mm_load_si128(reinterpret_cast<const __m128i*>(&in[i + 0]));
			vec0f = _mm_cvtepi32_ps(_mm_unpacklo_epi16(vec0, vecZero));
			vec1f = _mm_cvtepi32_ps(_mm_unpackhi_epi16(vec0, vecZero));
			vec0f = _mm_mul_ps(vec0f, vecScale);
			vec1f = _mm_mul_ps(vec1f, vecScale);
			vec0 = _mm_packs_epi32(_mm_cvttps_epi32(vec0f), _mm_cvttps_epi32(vec1f));
			vec0 = _mm_packus_epi16(vec0, vec0);
			_mm_storel_epi64(reinterpret_cast<__m128i*>(&out[i + 0]), vec0);
		}
		out += stride;
		in += stride;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
