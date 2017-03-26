/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/features/hough/intrin/x86/compv_core_feature_houghstd_intrin_avx2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

#if COMPV_ARCH_X64
// This code avoid using "movsxd" to transfer memory indexes
#	define AccInc_Intrin_SSE41(vecIndices) \
		vecTmp = _mm_cvtepi32_epi64(vecIndices); \
		vecIndices = _mm_cvtepi32_epi64(_mm_srli_si128(vecIndices, 8)); \
		pACC[_mm_cvtsi128_si64(vecTmp)]++; \
		pACC[_mm_extract_epi64(vecTmp, 1)]++; \
		pACC[_mm_cvtsi128_si64(vecIndices)]++; \
		pACC[_mm_extract_epi64(vecIndices, 1)]++
#else
#	define AccInc_Intrin_SSE41(vecIndices) \
		pACC[_mm_cvtsi128_si32(vecIndices)]++; \
		pACC[_mm_extract_epi32(vecIndices, 1)]++; \
		pACC[_mm_extract_epi32(vecIndices, 2)]++; \
		pACC[_mm_extract_epi32(vecIndices, 3)]++
#endif

#define AccInc_Intrin_AVX2(vecIndices) \
	vecSSE = _mm256_castsi256_si128(vecIndices); \
	AccInc_Intrin_SSE41(vecSSE); \
	vecSSE = _mm256_extracti128_si256(vecIndices, 1); \
	AccInc_Intrin_SSE41(vecSSE)

#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx2
#endif
// 8mpd -> minpack 8 for dwords (int32) - for maxTheta
void CompVHoughStdAccGatherRow_8mpd_Intrin_AVX2(COMPV_ALIGNED(AVX) const int32_t* pCosRho, COMPV_ALIGNED(AVX) const int32_t* pRowTimesSinRho, compv_uscalar_t col, int32_t* pACC, compv_uscalar_t accStride, compv_uscalar_t maxTheta)
{
	COMPV_DEBUG_INFO_CHECK_AVX2();
	_mm256_zeroupper();
	const __m256i vecColInt32 = _mm256_set1_epi32(static_cast<int32_t>(col));
	const __m256i vecStride = _mm256_set1_epi32(static_cast<int32_t>(accStride));
	compv_uscalar_t theta = 0;
	__m256i vec0, vec1, vec2, vec3;
	const __m256i vec8 = _mm256_set1_epi32(8);
	__m256i vecTheta = _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7);
	__m128i vecSSE;
#if COMPV_ARCH_X64
	__m128i vecTmp;
#endif

	for (theta = 0; theta < maxTheta - 31; theta += 32) { // maxTheta always > 16
		vec0 = _mm256_mullo_epi32(vecColInt32, _mm256_load_si256(reinterpret_cast<const __m256i*>(&pCosRho[theta])));
		vec1 = _mm256_mullo_epi32(vecColInt32, _mm256_load_si256(reinterpret_cast<const __m256i*>(&pCosRho[theta + 8])));
		vec2 = _mm256_mullo_epi32(vecColInt32, _mm256_load_si256(reinterpret_cast<const __m256i*>(&pCosRho[theta + 16])));
		vec3 = _mm256_mullo_epi32(vecColInt32, _mm256_load_si256(reinterpret_cast<const __m256i*>(&pCosRho[theta + 24])));
		vec0 = _mm256_add_epi32(vec0, _mm256_load_si256(reinterpret_cast<const __m256i*>(&pRowTimesSinRho[theta])));
		vec1 = _mm256_add_epi32(vec1, _mm256_load_si256(reinterpret_cast<const __m256i*>(&pRowTimesSinRho[theta + 8])));
		vec2 = _mm256_add_epi32(vec2, _mm256_load_si256(reinterpret_cast<const __m256i*>(&pRowTimesSinRho[theta + 16])));
		vec3 = _mm256_add_epi32(vec3, _mm256_load_si256(reinterpret_cast<const __m256i*>(&pRowTimesSinRho[theta + 24])));
		vec0 = _mm256_srai_epi32(vec0, 16);
		vec1 = _mm256_srai_epi32(vec1, 16);
		vec2 = _mm256_srai_epi32(vec2, 16);
		vec3 = _mm256_srai_epi32(vec3, 16);
		vec0 = _mm256_mullo_epi32(vec0, vecStride);
		vec1 = _mm256_mullo_epi32(vec1, vecStride);
		vec2 = _mm256_mullo_epi32(vec2, vecStride);
		vec3 = _mm256_mullo_epi32(vec3, vecStride);
		// TODO(asm): use vecTheta0, vecTheta1, vecTheta2 and vecTheta3
		vec0 = _mm256_sub_epi32(vecTheta, vec0);
		vecTheta = _mm256_add_epi32(vecTheta, vec8);
		vec1 = _mm256_sub_epi32(vecTheta, vec1);
		vecTheta = _mm256_add_epi32(vecTheta, vec8);
		vec2 = _mm256_sub_epi32(vecTheta, vec2);
		vecTheta = _mm256_add_epi32(vecTheta, vec8);
		vec3 = _mm256_sub_epi32(vecTheta, vec3);
		vecTheta = _mm256_add_epi32(vecTheta, vec8);
		AccInc_Intrin_AVX2(vec0);
		AccInc_Intrin_AVX2(vec1);
		AccInc_Intrin_AVX2(vec2);
		AccInc_Intrin_AVX2(vec3);
	}
	
	for (; theta < maxTheta; theta += 8) { // maxTheta is already 8d-aligned
		vec0 = _mm256_mullo_epi32(vecColInt32, _mm256_load_si256(reinterpret_cast<const __m256i*>(&pCosRho[theta])));
		vec0 = _mm256_add_epi32(vec0, _mm256_load_si256(reinterpret_cast<const __m256i*>(&pRowTimesSinRho[theta])));
		vec0 = _mm256_srai_epi32(vec0, 16);
		vec0 = _mm256_mullo_epi32(vec0, vecStride);
		vec0 = _mm256_sub_epi32(vecTheta, vec0);
		vecTheta = _mm256_add_epi32(vecTheta, vec8);
		AccInc_Intrin_AVX2(vec0);
	}
	_mm256_zeroupper();
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
