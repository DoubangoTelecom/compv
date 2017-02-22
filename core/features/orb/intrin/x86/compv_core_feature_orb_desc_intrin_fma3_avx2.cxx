/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/features/orb/intrin/x86/compv_core_feature_orb_desc_intrin_fma3_avx2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/intrin/x86/compv_intrin_avx.h"
#include "compv/base/compv_simd_globals.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx2
#endif
void CompVOrbBrief256_31_32f_Intrin_FMA3_AVX2(
	const uint8_t* img_center, compv_uscalar_t img_stride,
	const compv_float32_t* cos1, const compv_float32_t* sin1,
	COMPV_ALIGNED(AVX) const compv_float32_t* kBrief256Pattern31AX, COMPV_ALIGNED(AVX) const compv_float32_t* kBrief256Pattern31AY,
	COMPV_ALIGNED(AVX) const compv_float32_t* kBrief256Pattern31BX, COMPV_ALIGNED(AVX) const compv_float32_t* kBrief256Pattern31BY,
	void* out
)
{
	COMPV_DEBUG_INFO_CHECK_AVX2();
	COMPV_DEBUG_INFO_CHECK_FMA3();
	
	_mm256_zeroupper();
	COMPV_ALIGN_AVX2() int32_t vecIndex[32];
	COMPV_ALIGN_AVX2() uint8_t vecA[32];
	COMPV_ALIGN_AVX2() uint8_t vecB[32];
	__m256i vecR;
	__m256 vecX[4], vecY[4];

	uint32_t* outPtr = reinterpret_cast<uint32_t*>(out); // uint32_t for AVX

	const __m256i vec128 = _mm256_load_si256(reinterpret_cast<const __m256i*>(k128_u8));
	const __m256 vecStride = _mm256_set1_ps(static_cast<compv_float32_t>(img_stride));
	const __m256 vecCosT = _mm256_set1_ps(*cos1);
	const __m256 vecSinT = _mm256_set1_ps(*sin1);

	for (size_t i = 0; i < 256; i += 32) {
		// xf = (kBrief256Pattern31AX[i] * cosT - kBrief256Pattern31AY[i] * sinT);
		vecX[0] = _mm256_fmsub_ps(_mm256_load_ps(&kBrief256Pattern31AX[i + 0]), vecCosT, _mm256_mul_ps(_mm256_load_ps(&kBrief256Pattern31AY[i + 0]), vecSinT));
		vecX[1] = _mm256_fmsub_ps(_mm256_load_ps(&kBrief256Pattern31AX[i + 8]), vecCosT, _mm256_mul_ps(_mm256_load_ps(&kBrief256Pattern31AY[i + 8]), vecSinT));
		vecX[2] = _mm256_fmsub_ps(_mm256_load_ps(&kBrief256Pattern31AX[i + 16]), vecCosT, _mm256_mul_ps(_mm256_load_ps(&kBrief256Pattern31AY[i + 16]), vecSinT));
		vecX[3] = _mm256_fmsub_ps(_mm256_load_ps(&kBrief256Pattern31AX[i + 24]), vecCosT, _mm256_mul_ps(_mm256_load_ps(&kBrief256Pattern31AY[i + 24]), vecSinT));
		// yf = (kBrief256Pattern31AX[i] * sinT + kBrief256Pattern31AY[i] * cosT);
		vecY[0] = _mm256_fmadd_ps(_mm256_load_ps(&kBrief256Pattern31AX[i + 0]), vecSinT, _mm256_mul_ps(_mm256_load_ps(&kBrief256Pattern31AY[i + 0]), vecCosT));
		vecY[1] = _mm256_fmadd_ps(_mm256_load_ps(&kBrief256Pattern31AX[i + 8]), vecSinT, _mm256_mul_ps(_mm256_load_ps(&kBrief256Pattern31AY[i + 8]), vecCosT));
		vecY[2] = _mm256_fmadd_ps(_mm256_load_ps(&kBrief256Pattern31AX[i + 16]), vecSinT, _mm256_mul_ps(_mm256_load_ps(&kBrief256Pattern31AY[i + 16]), vecCosT));
		vecY[3] = _mm256_fmadd_ps(_mm256_load_ps(&kBrief256Pattern31AX[i + 24]), vecSinT, _mm256_mul_ps(_mm256_load_ps(&kBrief256Pattern31AY[i + 24]), vecCosT));
		// x = COMPV_MATH_ROUNDF_2_INT(xf, int);
		vecX[0] = _mm256_round_ps(vecX[0], 0x8);
		vecX[1] = _mm256_round_ps(vecX[1], 0x8);
		vecX[2] = _mm256_round_ps(vecX[2], 0x8);
		vecX[3] = _mm256_round_ps(vecX[3], 0x8);
		// y = COMPV_MATH_ROUNDF_2_INT(yf, int);
		vecY[0] = _mm256_round_ps(vecY[0], 0x8);
		vecY[1] = _mm256_round_ps(vecY[1], 0x8);
		vecY[2] = _mm256_round_ps(vecY[2], 0x8);
		vecY[3] = _mm256_round_ps(vecY[3], 0x8);
		// a = img_center[(y * img_stride) + x];
		_mm256_store_si256(reinterpret_cast<__m256i*>(&vecIndex[0]), _mm256_cvtps_epi32(_mm256_fmadd_ps(vecY[0], vecStride, vecX[0])));
		_mm256_store_si256(reinterpret_cast<__m256i*>(&vecIndex[8]), _mm256_cvtps_epi32(_mm256_fmadd_ps(vecY[1], vecStride, vecX[1])));
		_mm256_store_si256(reinterpret_cast<__m256i*>(&vecIndex[16]), _mm256_cvtps_epi32(_mm256_fmadd_ps(vecY[2], vecStride, vecX[2])));
		_mm256_store_si256(reinterpret_cast<__m256i*>(&vecIndex[24]), _mm256_cvtps_epi32(_mm256_fmadd_ps(vecY[3], vecStride, vecX[3])));
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
		vecA[16] = img_center[vecIndex[16]];
		vecA[17] = img_center[vecIndex[17]];
		vecA[18] = img_center[vecIndex[18]];
		vecA[19] = img_center[vecIndex[19]];
		vecA[20] = img_center[vecIndex[20]];
		vecA[21] = img_center[vecIndex[21]];
		vecA[22] = img_center[vecIndex[22]];
		vecA[23] = img_center[vecIndex[23]];
		vecA[24] = img_center[vecIndex[24]];
		vecA[25] = img_center[vecIndex[25]];
		vecA[26] = img_center[vecIndex[26]];
		vecA[27] = img_center[vecIndex[27]];
		vecA[28] = img_center[vecIndex[28]];
		vecA[29] = img_center[vecIndex[29]];
		vecA[30] = img_center[vecIndex[30]];
		vecA[31] = img_center[vecIndex[31]];

		// xf = (kBrief256Pattern31BX[i] * cosT - kBrief256Pattern31BY[i] * sinT);
		vecX[0] = _mm256_fmsub_ps(_mm256_load_ps(&kBrief256Pattern31BX[i + 0]), vecCosT, _mm256_mul_ps(_mm256_load_ps(&kBrief256Pattern31BY[i + 0]), vecSinT));
		vecX[1] = _mm256_fmsub_ps(_mm256_load_ps(&kBrief256Pattern31BX[i + 8]), vecCosT, _mm256_mul_ps(_mm256_load_ps(&kBrief256Pattern31BY[i + 8]), vecSinT));
		vecX[2] = _mm256_fmsub_ps(_mm256_load_ps(&kBrief256Pattern31BX[i + 16]), vecCosT, _mm256_mul_ps(_mm256_load_ps(&kBrief256Pattern31BY[i + 16]), vecSinT));
		vecX[3] = _mm256_fmsub_ps(_mm256_load_ps(&kBrief256Pattern31BX[i + 24]), vecCosT, _mm256_mul_ps(_mm256_load_ps(&kBrief256Pattern31BY[i + 24]), vecSinT));
		// yf = (kBrief256Pattern31BX[i] * sinT + kBrief256Pattern31BY[i] * cosT);
		vecY[0] = _mm256_fmadd_ps(_mm256_load_ps(&kBrief256Pattern31BX[i + 0]), vecSinT, _mm256_mul_ps(_mm256_load_ps(&kBrief256Pattern31BY[i + 0]), vecCosT));
		vecY[1] = _mm256_fmadd_ps(_mm256_load_ps(&kBrief256Pattern31BX[i + 8]), vecSinT, _mm256_mul_ps(_mm256_load_ps(&kBrief256Pattern31BY[i + 8]), vecCosT));
		vecY[2] = _mm256_fmadd_ps(_mm256_load_ps(&kBrief256Pattern31BX[i + 16]), vecSinT, _mm256_mul_ps(_mm256_load_ps(&kBrief256Pattern31BY[i + 16]), vecCosT));
		vecY[3] = _mm256_fmadd_ps(_mm256_load_ps(&kBrief256Pattern31BX[i + 24]), vecSinT, _mm256_mul_ps(_mm256_load_ps(&kBrief256Pattern31BY[i + 24]), vecCosT));
		// x = COMPV_MATH_ROUNDF_2_INT(xf, int);
		vecX[0] = _mm256_round_ps(vecX[0], 0x8);
		vecX[1] = _mm256_round_ps(vecX[1], 0x8);
		vecX[2] = _mm256_round_ps(vecX[2], 0x8);
		vecX[3] = _mm256_round_ps(vecX[3], 0x8);
		// y = COMPV_MATH_ROUNDF_2_INT(yf, int);
		vecY[0] = _mm256_round_ps(vecY[0], 0x8);
		vecY[1] = _mm256_round_ps(vecY[1], 0x8);
		vecY[2] = _mm256_round_ps(vecY[2], 0x8);
		vecY[3] = _mm256_round_ps(vecY[3], 0x8);
		// b = img_center[(y * img_stride) + x];
		_mm256_store_si256(reinterpret_cast<__m256i*>(&vecIndex[0]), _mm256_cvtps_epi32(_mm256_fmadd_ps(vecY[0], vecStride, vecX[0])));
		_mm256_store_si256(reinterpret_cast<__m256i*>(&vecIndex[8]), _mm256_cvtps_epi32(_mm256_fmadd_ps(vecY[1], vecStride, vecX[1])));
		_mm256_store_si256(reinterpret_cast<__m256i*>(&vecIndex[16]), _mm256_cvtps_epi32(_mm256_fmadd_ps(vecY[2], vecStride, vecX[2])));
		_mm256_store_si256(reinterpret_cast<__m256i*>(&vecIndex[24]), _mm256_cvtps_epi32(_mm256_fmadd_ps(vecY[3], vecStride, vecX[3])));
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
		vecB[16] = img_center[vecIndex[16]];
		vecB[17] = img_center[vecIndex[17]];
		vecB[18] = img_center[vecIndex[18]];
		vecB[19] = img_center[vecIndex[19]];
		vecB[20] = img_center[vecIndex[20]];
		vecB[21] = img_center[vecIndex[21]];
		vecB[22] = img_center[vecIndex[22]];
		vecB[23] = img_center[vecIndex[23]];
		vecB[24] = img_center[vecIndex[24]];
		vecB[25] = img_center[vecIndex[25]];
		vecB[26] = img_center[vecIndex[26]];
		vecB[27] = img_center[vecIndex[27]];
		vecB[28] = img_center[vecIndex[28]];
		vecB[29] = img_center[vecIndex[29]];
		vecB[30] = img_center[vecIndex[30]];
		vecB[31] = img_center[vecIndex[31]];

		// _out[0] |= (a < b) ? (u64_1 << j) : 0;
		// _mm256_cmplt_epu8 doesn't exist this is why we substract 128
		// _mm256_cmplt_epi8 doesn't exist this is why we swap VecA/VecB
		vecR = _mm256_cmpgt_epi8(_mm256_sub_epi8(_mm256_load_si256(reinterpret_cast<const __m256i*>(vecB)), vec128), _mm256_sub_epi8(_mm256_load_si256(reinterpret_cast<const __m256i*>(vecA)), vec128));
		*outPtr++ = static_cast<uint32_t>(_mm256_movemask_epi8(vecR));
	}

	_mm256_zeroupper();
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
