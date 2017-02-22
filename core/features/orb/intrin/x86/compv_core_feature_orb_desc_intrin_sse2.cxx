/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/features/orb/intrin/x86/compv_core_feature_orb_desc_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/intrin/x86/compv_intrin_sse.h"
#include "compv/base/compv_simd_globals.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// TODO(dmi): in ASM add SSE4.1 version which supports "_mm_mullo_epi32"
void CompVOrbBrief256_31_32f_Intrin_SSE2(
	const uint8_t* img_center, compv_uscalar_t img_stride,
	const compv_float32_t* cos1, const compv_float32_t* sin1,
	COMPV_ALIGNED(SSE) const compv_float32_t* kBrief256Pattern31AX, COMPV_ALIGNED(SSE) const compv_float32_t* kBrief256Pattern31AY,
	COMPV_ALIGNED(SSE) const compv_float32_t* kBrief256Pattern31BX, COMPV_ALIGNED(SSE) const compv_float32_t* kBrief256Pattern31BY,
	void* out
)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("in ASM add SSE4.1 version which supports '_mm_mullo_epi32'"); // SSE41, ASM
	COMPV_ALIGN_SSE() int32_t vecIndex[16];
	COMPV_ALIGN_SSE() uint8_t vecA[16];
	COMPV_ALIGN_SSE() uint8_t vecB[16];
	__m128i vecX[4], vecY[4], vecR;

	uint16_t* outPtr = reinterpret_cast<uint16_t*>(out); // uint32_t for AVX

	const __m128i vec128 = _mm_load_si128(reinterpret_cast<const __m128i*>(k128_u8));
	const __m128i vecStride = _mm_set1_epi32(static_cast<int>(img_stride));
	const __m128 vecCosT = _mm_set1_ps(*cos1);
	const __m128 vecSinT = _mm_set1_ps(*sin1);

	// _mm_mullo_epi32 is SSE4.1
	// use _mm_extract_epi32 and _mm_insert_epi32 in SSE4.1 instead of vecIndex[4]

	for (size_t i = 0; i < 256; i += 16) {
		// xf = (kBrief256Pattern31AX[i] * cosT - kBrief256Pattern31AY[i] * sinT);
		vecX[0] = _mm_castps_si128(_mm_sub_ps(_mm_mul_ps(_mm_load_ps(&kBrief256Pattern31AX[i + 0]), vecCosT), _mm_mul_ps(_mm_load_ps(&kBrief256Pattern31AY[i + 0]), vecSinT)));
		vecX[1] = _mm_castps_si128(_mm_sub_ps(_mm_mul_ps(_mm_load_ps(&kBrief256Pattern31AX[i + 4]), vecCosT), _mm_mul_ps(_mm_load_ps(&kBrief256Pattern31AY[i + 4]), vecSinT)));
		vecX[2] = _mm_castps_si128(_mm_sub_ps(_mm_mul_ps(_mm_load_ps(&kBrief256Pattern31AX[i + 8]), vecCosT), _mm_mul_ps(_mm_load_ps(&kBrief256Pattern31AY[i + 8]), vecSinT)));
		vecX[3] = _mm_castps_si128(_mm_sub_ps(_mm_mul_ps(_mm_load_ps(&kBrief256Pattern31AX[i + 12]), vecCosT), _mm_mul_ps(_mm_load_ps(&kBrief256Pattern31AY[i + 12]), vecSinT)));
		// yf = (kBrief256Pattern31AX[i] * sinT + kBrief256Pattern31AY[i] * cosT);
		vecY[0] = _mm_castps_si128(_mm_add_ps(_mm_mul_ps(_mm_load_ps(&kBrief256Pattern31AX[i + 0]), vecSinT), _mm_mul_ps(_mm_load_ps(&kBrief256Pattern31AY[i + 0]), vecCosT)));
		vecY[1] = _mm_castps_si128(_mm_add_ps(_mm_mul_ps(_mm_load_ps(&kBrief256Pattern31AX[i + 4]), vecSinT), _mm_mul_ps(_mm_load_ps(&kBrief256Pattern31AY[i + 4]), vecCosT)));
		vecY[2] = _mm_castps_si128(_mm_add_ps(_mm_mul_ps(_mm_load_ps(&kBrief256Pattern31AX[i + 8]), vecSinT), _mm_mul_ps(_mm_load_ps(&kBrief256Pattern31AY[i + 8]), vecCosT)));
		vecY[3] = _mm_castps_si128(_mm_add_ps(_mm_mul_ps(_mm_load_ps(&kBrief256Pattern31AX[i + 12]), vecSinT), _mm_mul_ps(_mm_load_ps(&kBrief256Pattern31AY[i + 12]), vecCosT)));
		// x = COMPV_MATH_ROUNDF_2_NEAREST_INT(xf, int);
		vecX[0] = _mm_cvtps_epi32(_mm_castsi128_ps(vecX[0]));
		vecX[1] = _mm_cvtps_epi32(_mm_castsi128_ps(vecX[1]));
		vecX[2] = _mm_cvtps_epi32(_mm_castsi128_ps(vecX[2]));
		vecX[3] = _mm_cvtps_epi32(_mm_castsi128_ps(vecX[3]));
		// y = COMPV_MATH_ROUNDF_2_NEAREST_INT(yf, int);
		vecY[0] = _mm_cvtps_epi32(_mm_castsi128_ps(vecY[0]));
		vecY[1] = _mm_cvtps_epi32(_mm_castsi128_ps(vecY[1]));
		vecY[2] = _mm_cvtps_epi32(_mm_castsi128_ps(vecY[2]));
		vecY[3] = _mm_cvtps_epi32(_mm_castsi128_ps(vecY[3]));
		// a = img_center[(y * img_stride) + x];
		_mm_store_si128(reinterpret_cast<__m128i*>(&vecIndex[0]), _mm_add_epi32(_mm_mullo_epi32_SSE2(vecY[0], vecStride), vecX[0]));
		_mm_store_si128(reinterpret_cast<__m128i*>(&vecIndex[4]), _mm_add_epi32(_mm_mullo_epi32_SSE2(vecY[1], vecStride), vecX[1]));
		_mm_store_si128(reinterpret_cast<__m128i*>(&vecIndex[8]), _mm_add_epi32(_mm_mullo_epi32_SSE2(vecY[2], vecStride), vecX[2]));
		_mm_store_si128(reinterpret_cast<__m128i*>(&vecIndex[12]), _mm_add_epi32(_mm_mullo_epi32_SSE2(vecY[3], vecStride), vecX[3]));
		vecA[0] = img_center[vecIndex[0]];
		vecA[1] = img_center[vecIndex[1]];
		vecA[2] = img_center[vecIndex[2]];
		vecA[3] = img_center[vecIndex[3]];
		vecA[4] = img_center[vecIndex[4]];
		vecA[5] = img_center[vecIndex[5]];
		vecA[6] = img_center[vecIndex[6]];
		vecA[7] = img_center[vecIndex[7]];
		vecA[8] = img_center[vecIndex[8]];
		vecA[9] = img_center[vecIndex[9]];
		vecA[10] = img_center[vecIndex[10]];
		vecA[11] = img_center[vecIndex[11]];
		vecA[12] = img_center[vecIndex[12]];
		vecA[13] = img_center[vecIndex[13]];
		vecA[14] = img_center[vecIndex[14]];
		vecA[15] = img_center[vecIndex[15]];

		// xf = (kBrief256Pattern31BX[i] * cosT - kBrief256Pattern31BY[i] * sinT);
		vecX[0] = _mm_castps_si128(_mm_sub_ps(_mm_mul_ps(_mm_load_ps(&kBrief256Pattern31BX[i + 0]), vecCosT), _mm_mul_ps(_mm_load_ps(&kBrief256Pattern31BY[i + 0]), vecSinT)));
		vecX[1] = _mm_castps_si128(_mm_sub_ps(_mm_mul_ps(_mm_load_ps(&kBrief256Pattern31BX[i + 4]), vecCosT), _mm_mul_ps(_mm_load_ps(&kBrief256Pattern31BY[i + 4]), vecSinT)));
		vecX[2] = _mm_castps_si128(_mm_sub_ps(_mm_mul_ps(_mm_load_ps(&kBrief256Pattern31BX[i + 8]), vecCosT), _mm_mul_ps(_mm_load_ps(&kBrief256Pattern31BY[i + 8]), vecSinT)));
		vecX[3] = _mm_castps_si128(_mm_sub_ps(_mm_mul_ps(_mm_load_ps(&kBrief256Pattern31BX[i + 12]), vecCosT), _mm_mul_ps(_mm_load_ps(&kBrief256Pattern31BY[i + 12]), vecSinT)));
		// yf = (kBrief256Pattern31BX[i] * sinT + kBrief256Pattern31BY[i] * cosT);
		vecY[0] = _mm_castps_si128(_mm_add_ps(_mm_mul_ps(_mm_load_ps(&kBrief256Pattern31BX[i + 0]), vecSinT), _mm_mul_ps(_mm_load_ps(&kBrief256Pattern31BY[i + 0]), vecCosT)));
		vecY[1] = _mm_castps_si128(_mm_add_ps(_mm_mul_ps(_mm_load_ps(&kBrief256Pattern31BX[i + 4]), vecSinT), _mm_mul_ps(_mm_load_ps(&kBrief256Pattern31BY[i + 4]), vecCosT)));
		vecY[2] = _mm_castps_si128(_mm_add_ps(_mm_mul_ps(_mm_load_ps(&kBrief256Pattern31BX[i + 8]), vecSinT), _mm_mul_ps(_mm_load_ps(&kBrief256Pattern31BY[i + 8]), vecCosT)));
		vecY[3] = _mm_castps_si128(_mm_add_ps(_mm_mul_ps(_mm_load_ps(&kBrief256Pattern31BX[i + 12]), vecSinT), _mm_mul_ps(_mm_load_ps(&kBrief256Pattern31BY[i + 12]), vecCosT)));
		// x = COMPV_MATH_ROUNDF_2_NEAREST_INT(xf, int);
		vecX[0] = _mm_cvtps_epi32(_mm_castsi128_ps(vecX[0]));
		vecX[1] = _mm_cvtps_epi32(_mm_castsi128_ps(vecX[1]));
		vecX[2] = _mm_cvtps_epi32(_mm_castsi128_ps(vecX[2]));
		vecX[3] = _mm_cvtps_epi32(_mm_castsi128_ps(vecX[3]));
		// y = COMPV_MATH_ROUNDF_2_NEAREST_INT(yf, int);
		vecY[0] = _mm_cvtps_epi32(_mm_castsi128_ps(vecY[0]));
		vecY[1] = _mm_cvtps_epi32(_mm_castsi128_ps(vecY[1]));
		vecY[2] = _mm_cvtps_epi32(_mm_castsi128_ps(vecY[2]));
		vecY[3] = _mm_cvtps_epi32(_mm_castsi128_ps(vecY[3]));
		// b = img_center[(y * img_stride) + x];
		_mm_store_si128(reinterpret_cast<__m128i*>(&vecIndex[0]), _mm_add_epi32(_mm_mullo_epi32_SSE2(vecY[0], vecStride), vecX[0]));
		_mm_store_si128(reinterpret_cast<__m128i*>(&vecIndex[4]), _mm_add_epi32(_mm_mullo_epi32_SSE2(vecY[1], vecStride), vecX[1]));
		_mm_store_si128(reinterpret_cast<__m128i*>(&vecIndex[8]), _mm_add_epi32(_mm_mullo_epi32_SSE2(vecY[2], vecStride), vecX[2]));
		_mm_store_si128(reinterpret_cast<__m128i*>(&vecIndex[12]), _mm_add_epi32(_mm_mullo_epi32_SSE2(vecY[3], vecStride), vecX[3]));
		vecB[0] = img_center[vecIndex[0]];
		vecB[1] = img_center[vecIndex[1]];
		vecB[2] = img_center[vecIndex[2]];
		vecB[3] = img_center[vecIndex[3]];
		vecB[4] = img_center[vecIndex[4]];
		vecB[5] = img_center[vecIndex[5]];
		vecB[6] = img_center[vecIndex[6]];
		vecB[7] = img_center[vecIndex[7]];
		vecB[8] = img_center[vecIndex[8]];
		vecB[9] = img_center[vecIndex[9]];
		vecB[10] = img_center[vecIndex[10]];
		vecB[11] = img_center[vecIndex[11]];
		vecB[12] = img_center[vecIndex[12]];
		vecB[13] = img_center[vecIndex[13]];
		vecB[14] = img_center[vecIndex[14]];
		vecB[15] = img_center[vecIndex[15]];

		// _out[0] |= (a < b) ? (u64_1 << j) : 0;
		// _mm_cmplt_epu8 doesn't exist this is why we substract 128
		vecR = _mm_cmplt_epi8(_mm_sub_epi8(_mm_load_si128(reinterpret_cast<const __m128i*>(vecA)), vec128), _mm_sub_epi8(_mm_load_si128(reinterpret_cast<const __m128i*>(vecB)), vec128));
		*outPtr++ = _mm_movemask_epi8(vecR);
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
