/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/intrin/x86/compv_image_threshold_intrin_sse41.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/intrin/x86/compv_intrin_sse.h"
#include "compv/base/compv_simd_globals.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

void CompVImageThresholdOtsuSum_32s32s_Intrin_SSE41(COMPV_ALIGNED(SSE) const uint32_t* ptr32uHistogram, COMPV_ALIGNED(SSE) uint32_t* sumA256, uint32_t* sumB1)
{
	COMPV_DEBUG_INFO_CHECK_SSE41();

	const __m128i vecIndicesInc = _mm_set1_epi32(16);
	__m128i vecIndices0 = _mm_setr_epi32(0, 1, 2, 3);
	__m128i vecIndices1 = _mm_setr_epi32(4, 5, 6, 7);
	__m128i vecIndices2 = _mm_setr_epi32(8, 9, 10, 11);
	__m128i vecIndices3 = _mm_setr_epi32(12, 13, 14, 15);
	__m128i vec0, vec1, vec2, vec3, vecSumB0, vecSumB1, vecSumB2, vecSumB3;

	vecSumB0 = _mm_setzero_si128();
	vecSumB1 = _mm_setzero_si128();
	vecSumB2 = _mm_setzero_si128();
	vecSumB3 = _mm_setzero_si128();
	
	for (size_t i = 0; i < 256; i += 16) {
		vec0 = _mm_load_si128(reinterpret_cast<const __m128i*>(&ptr32uHistogram[i]));
		vec1 = _mm_load_si128(reinterpret_cast<const __m128i*>(&ptr32uHistogram[i + 4]));
		vec2 = _mm_load_si128(reinterpret_cast<const __m128i*>(&ptr32uHistogram[i + 8]));
		vec3 = _mm_load_si128(reinterpret_cast<const __m128i*>(&ptr32uHistogram[i + 12]));
		
		// We have a macro to call SSE2 version (_mm_mullo_epi32_SSE2) but doesn't worth it
		vec0 = _mm_mullo_epi32(vecIndices0, vec0);
		vec1 = _mm_mullo_epi32(vecIndices1, vec1);
		vec2 = _mm_mullo_epi32(vecIndices2, vec2);
		vec3 = _mm_mullo_epi32(vecIndices3, vec3);

		vecIndices0 = _mm_add_epi32(vecIndices0, vecIndicesInc);
		vecIndices1 = _mm_add_epi32(vecIndices1, vecIndicesInc);
		vecIndices2 = _mm_add_epi32(vecIndices2, vecIndicesInc);
		vecIndices3 = _mm_add_epi32(vecIndices3, vecIndicesInc);

		vecSumB0 = _mm_add_epi32(vecSumB0, vec0);
		vecSumB1 = _mm_add_epi32(vecSumB1, vec1);
		vecSumB2 = _mm_add_epi32(vecSumB2, vec2);
		vecSumB3 = _mm_add_epi32(vecSumB3, vec3);

		_mm_store_si128(reinterpret_cast<__m128i*>(&sumA256[i]), vec0);
		_mm_store_si128(reinterpret_cast<__m128i*>(&sumA256[i + 4]), vec1);
		_mm_store_si128(reinterpret_cast<__m128i*>(&sumA256[i + 8]), vec2);
		_mm_store_si128(reinterpret_cast<__m128i*>(&sumA256[i + 12]), vec3);
	}

	vecSumB0 = _mm_add_epi32(vecSumB0, vecSumB1);
	vecSumB2 = _mm_add_epi32(vecSumB2, vecSumB3);
	vecSumB0 = _mm_add_epi32(vecSumB0, vecSumB2);
	vecSumB0 = _mm_add_epi32(vecSumB0, _mm_shuffle_epi32(vecSumB0, 0x0E));
	vecSumB0 = _mm_add_epi32(vecSumB0, _mm_shuffle_epi32(vecSumB0, 0x01));
	*sumB1 = static_cast<uint32_t>(_mm_cvtsi128_si32(vecSumB0));
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
