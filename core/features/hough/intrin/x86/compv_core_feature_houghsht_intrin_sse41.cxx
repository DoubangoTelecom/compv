/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/features/hough/intrin/x86/compv_core_feature_houghsht_intrin_sse41.h"

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

// 4mpd -> minpack 4 for dwords (int32) - for maxTheta
void CompVHoughShtAccGatherRow_4mpd_Intrin_SSE41(COMPV_ALIGNED(SSE) const int32_t* pCosRho, COMPV_ALIGNED(SSE) const int32_t* pRowTimesSinRho, compv_uscalar_t col, int32_t* pACC, compv_uscalar_t accStride, compv_uscalar_t maxTheta)
{
	COMPV_DEBUG_INFO_CHECK_SSE41();
	const __m128i vecColInt32 = _mm_set1_epi32(static_cast<int32_t>(col));
	const __m128i vecStride = _mm_set1_epi32(static_cast<int32_t>(accStride));
	compv_uscalar_t theta = 0;
	__m128i vec0, vec1, vec2, vec3;
	const __m128i vec4 = _mm_set1_epi32(4);
	__m128i vecTheta = _mm_setr_epi32(0, 1, 2, 3);
#if COMPV_ARCH_X64
	__m128i vecTmp;
#endif

	for (theta = 0; theta < maxTheta - 15; theta += 16) { // maxTheta always > 16
		vec0 = _mm_mullo_epi32(vecColInt32, _mm_load_si128(reinterpret_cast<const __m128i*>(&pCosRho[theta])));
		vec1 = _mm_mullo_epi32(vecColInt32, _mm_load_si128(reinterpret_cast<const __m128i*>(&pCosRho[theta + 4])));
		vec2 = _mm_mullo_epi32(vecColInt32, _mm_load_si128(reinterpret_cast<const __m128i*>(&pCosRho[theta + 8])));
		vec3 = _mm_mullo_epi32(vecColInt32, _mm_load_si128(reinterpret_cast<const __m128i*>(&pCosRho[theta + 12])));
		vec0 = _mm_add_epi32(vec0, _mm_load_si128(reinterpret_cast<const __m128i*>(&pRowTimesSinRho[theta])));
		vec1 = _mm_add_epi32(vec1, _mm_load_si128(reinterpret_cast<const __m128i*>(&pRowTimesSinRho[theta + 4])));
		vec2 = _mm_add_epi32(vec2, _mm_load_si128(reinterpret_cast<const __m128i*>(&pRowTimesSinRho[theta + 8])));
		vec3 = _mm_add_epi32(vec3, _mm_load_si128(reinterpret_cast<const __m128i*>(&pRowTimesSinRho[theta + 12])));
		vec0 = _mm_srai_epi32(vec0, 16);
		vec1 = _mm_srai_epi32(vec1, 16);
		vec2 = _mm_srai_epi32(vec2, 16);
		vec3 = _mm_srai_epi32(vec3, 16);
		vec0 = _mm_mullo_epi32(vec0, vecStride);
		vec1 = _mm_mullo_epi32(vec1, vecStride);
		vec2 = _mm_mullo_epi32(vec2, vecStride);
		vec3 = _mm_mullo_epi32(vec3, vecStride);
		// TODO(asm): use vecTheta0, vecTheta1, vecTheta2 and vecTheta3
		vec0 = _mm_sub_epi32(vecTheta, vec0);
		vecTheta = _mm_add_epi32(vecTheta, vec4);
		vec1 = _mm_sub_epi32(vecTheta, vec1);
		vecTheta = _mm_add_epi32(vecTheta, vec4);
		vec2 = _mm_sub_epi32(vecTheta, vec2);
		vecTheta = _mm_add_epi32(vecTheta, vec4);
		vec3 = _mm_sub_epi32(vecTheta, vec3);
		vecTheta = _mm_add_epi32(vecTheta, vec4);
		AccInc_Intrin_SSE41(vec0);
		AccInc_Intrin_SSE41(vec1);
		AccInc_Intrin_SSE41(vec2);
		AccInc_Intrin_SSE41(vec3);
	}

	for (; theta < maxTheta; theta += 4) { // maxTheta is already 4d-aligned
		vec0 = _mm_mullo_epi32(vecColInt32, _mm_load_si128(reinterpret_cast<const __m128i*>(&pCosRho[theta])));
		vec0 = _mm_add_epi32(vec0, _mm_load_si128(reinterpret_cast<const __m128i*>(&pRowTimesSinRho[theta])));
		vec0 = _mm_srai_epi32(vec0, 16);
		vec0 = _mm_mullo_epi32(vec0, vecStride);
		vec0 = _mm_sub_epi32(vecTheta, vec0);
		vecTheta = _mm_add_epi32(vecTheta, vec4);
		AccInc_Intrin_SSE41(vec0);
	}
}

// pSinRho and rowTimesSinRhoPtr must be strided and SSE-aligned -> reading beyond count
// count must be >= 16
void CompVHoughShtRowTimesSinRho_Intrin_SSE41(COMPV_ALIGNED(SSE) const int32_t* pSinRho, COMPV_ALIGNED(SSE) compv_uscalar_t row, COMPV_ALIGNED(SSE) int32_t* rowTimesSinRhoPtr, compv_uscalar_t count)
{
	const __m128i vecRowInt32 = _mm_set1_epi32(static_cast<int32_t>(row));
	compv_uscalar_t i;
	for (i = 0; i < count - 15; i += 16) {
		_mm_store_si128(reinterpret_cast<__m128i*>(&rowTimesSinRhoPtr[i]),
			_mm_mullo_epi32(vecRowInt32, _mm_load_si128(reinterpret_cast<const __m128i*>(&pSinRho[i]))));
		_mm_store_si128(reinterpret_cast<__m128i*>(&rowTimesSinRhoPtr[i + 4]),
			_mm_mullo_epi32(vecRowInt32, _mm_load_si128(reinterpret_cast<const __m128i*>(&pSinRho[i + 4]))));
		_mm_store_si128(reinterpret_cast<__m128i*>(&rowTimesSinRhoPtr[i + 8]),
			_mm_mullo_epi32(vecRowInt32, _mm_load_si128(reinterpret_cast<const __m128i*>(&pSinRho[i + 8]))));
		_mm_store_si128(reinterpret_cast<__m128i*>(&rowTimesSinRhoPtr[i + 12]),
			_mm_mullo_epi32(vecRowInt32, _mm_load_si128(reinterpret_cast<const __m128i*>(&pSinRho[i + 12]))));
	}
	for (; i < count; i += 4) {
		_mm_store_si128(reinterpret_cast<__m128i*>(&rowTimesSinRhoPtr[i]),
			_mm_mullo_epi32(vecRowInt32, _mm_load_si128(reinterpret_cast<const __m128i*>(&pSinRho[i]))));
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
