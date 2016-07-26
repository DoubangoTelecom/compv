/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/intrinsics/x86/features/hough/compv_feature_houghstd_intrin_sse41.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/features/hough/compv_feature_houghstd.h"
#include "compv/intrinsics/x86/compv_intrin_sse.h"
#include "compv/compv_simd_globals.h"

COMPV_NAMESPACE_BEGIN()

// TODO(dmi): add ASM
void HoughStdAccGatherRow_Intrin_SSE41(int32_t* pACC, int32_t accStride, const uint8_t* pixels, int32_t maxCols, int32_t maxThetaCount, int32_t row, COMPV_ALIGNED(SSE) const int32_t* pCosRho, COMPV_ALIGNED(SSE) const int32_t* pSinRho)
{
	COMPV_DEBUG_INFO_CHECK_SSE41();

	int32_t col, theta, rhoInt32;
	const int32_t maxThetaCountSSE = maxThetaCount - 3;
	__m128i xmmCol, xmmRho, xmmTheta, xmm0;
	const __m128i xmmRow = _mm_set1_epi32(row);
	const __m128i xmmStride = _mm_set1_epi32(accStride);
	static const __m128i xmm4 = _mm_set1_epi32(4);
	for (col = 0; col < maxCols; ++col) {
		if (pixels[col]) {
			xmmCol = _mm_set1_epi32(col);
			xmmTheta = _mm_setr_epi32(0, 1, 2, 3);
			for (theta = 0; theta < maxThetaCountSSE; theta += 4) {
				xmmRho = _mm_srai_epi32(_mm_add_epi32(_mm_mullo_epi32(xmmCol, _mm_load_si128(reinterpret_cast<const __m128i*>(&pCosRho[theta]))),
					_mm_mullo_epi32(xmmRow, _mm_load_si128(reinterpret_cast<const __m128i*>(&pSinRho[theta])))), 16);
				xmm0 = _mm_sub_epi32(xmmTheta, _mm_mullo_epi32(xmmRho, xmmStride));
				pACC[_mm_extract_epi32(xmm0, 0)]++;
				pACC[_mm_extract_epi32(xmm0, 1)]++;
				pACC[_mm_extract_epi32(xmm0, 2)]++;
				pACC[_mm_extract_epi32(xmm0, 3)]++;
				xmmTheta = _mm_add_epi32(xmmTheta, xmm4);
			}
			for (; theta < maxThetaCount; ++theta) {
				rhoInt32 = (col * pCosRho[theta] + row * pSinRho[theta]) >> 16;
				pACC[theta - (rhoInt32 * accStride)]++;
			}
		}
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
