/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/intrin/x86/compv_image_conv_to_rgbx_intrin_avx2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/intrin/x86/compv_intrin_avx.h"
#include "compv/base/intrin/x86/compv_intrin_sse.h"
#include "compv/base/image/compv_image_conv_common.h"
#include "compv/base/compv_simd_globals.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx2
#endif
void CompVImageConvYuv420p_to_Rgba32_Intrin_AVX2(COMPV_ALIGNED(AVX) const uint8_t* yPtr, COMPV_ALIGNED(AVX) const uint8_t* uPtr, COMPV_ALIGNED(AVX) const uint8_t* vPtr, COMPV_ALIGNED(AVX) uint8_t* rgbaPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(AVX) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_AVX2();
	_mm256_zeroupper();

	compv_uscalar_t i, j, k, l;
	const compv_uscalar_t strideUV = (stride >> 1); // no need for "((stride + 1) >> 1)" because stride is even (aligned on #32 bytes)
	const compv_uscalar_t strideRGBA = (stride << 2);
	__m256i vecYlow, vecYhigh, vecU, vecV, vecR, vecG, vecB;
	__m256i vec0, vec1;
	__m128i vecUn, vecVn;
	// To avoid AVX transition issues keep the next static variables local
	static const __m256i vecZero = _mm256_setzero_si256();
	static const __m256i vec16 = _mm256_set1_epi16(16);
	static const __m256i vec37 = _mm256_set1_epi16(37);
	static const __m256i vec51 = _mm256_set1_epi16(51);
	static const __m256i vec65 = _mm256_set1_epi16(65);
	static const __m256i vec127 = _mm256_set1_epi16(127);
	static const __m256i vec13_26 = _mm256_set1_epi32(0x001a000d); // 13, 26, 13, 26 ...
	static const __m256i vecA = _mm256_cmpeq_epi8(vec127, vec127); // 255, 255, 255, 255

	// ASM code do not create variables for k and l: k = [(i<<2)] and l = [i>>1]

	for (j = 0; j < height; ++j) {
		for (i = 0, k = 0, l = 0; i < width; i += 32, k += 128, l += 16) {
			/* Load samples */
			vecYlow = _mm256_load_si256(reinterpret_cast<const __m256i*>(&yPtr[i])); // #16 Y samples
			vecUn = _mm_load_si128(reinterpret_cast<const __m128i*>(&uPtr[l])); // #8 U samples, low mem
			vecVn = _mm_load_si128(reinterpret_cast<const __m128i*>(&vPtr[l])); // #8 V samples, low mem

			/* == Staring this line we're just converting from Y,U,V to R,G,B == */

			/* Convert to I16 */
			vecYhigh = _mm256_unpackhi_epi8(vecYlow, vecZero);
			vecYlow = _mm256_unpacklo_epi8(vecYlow, vecZero);
			vecU = _mm256_cvtepu8_epi16(vecUn);
			vecV = _mm256_cvtepu8_epi16(vecVn);

			/* Compute Y', U', V' */
			vecYlow = _mm256_sub_epi16(vecYlow, vec16);
			vecYhigh = _mm256_sub_epi16(vecYhigh, vec16);
			vecU = _mm256_sub_epi16(vecU, vec127);
			vecV = _mm256_sub_epi16(vecV, vec127);

			/* Compute (37Y'), (51V') and (65U') */
			vecYlow = _mm256_mullo_epi16(vec37, vecYlow);
			vecYhigh = _mm256_mullo_epi16(vec37, vecYhigh);
			vec0 = _mm256_mullo_epi16(vec51, vecV);
			vec1 = _mm256_mullo_epi16(vec65, vecU);

			/* Compute R = (37Y' + 0U' + 51V') >> 5 */
			vecR = _mm256_packus_epi16(
				_mm256_srai_epi16(_mm256_add_epi16(vecYlow, _mm256_unpacklo_epi16(vec0, vec0)), 5),
				_mm256_srai_epi16(_mm256_add_epi16(vecYhigh, _mm256_unpackhi_epi16(vec0, vec0)), 5)
			);

			/* B = (37Y' + 65U' + 0V') >> 5 */
			vecB = _mm256_packus_epi16(
				_mm256_srai_epi16(_mm256_add_epi16(vecYlow, _mm256_unpacklo_epi16(vec1, vec1)), 5),
				_mm256_srai_epi16(_mm256_add_epi16(vecYhigh, _mm256_unpackhi_epi16(vec1, vec1)), 5)
			);

			/* Compute G = (37Y' - 13U' - 26V') >> 5 = (37Y' - (13U' + 26V')) >> 5 */
			vec0 = _mm256_madd_epi16(vec13_26, _mm256_unpacklo_epi16(vecU, vecV)); // (13U' + 26V').low - I32
			vec1 = _mm256_madd_epi16(vec13_26, _mm256_unpackhi_epi16(vecU, vecV)); // (13U' + 26V').high - I32
			vec0 = _mm256_packs_epi32(vec0, vec1);
			vecG = _mm256_packus_epi16(
				_mm256_srai_epi16(_mm256_sub_epi16(vecYlow, _mm256_unpacklo_epi16(vec0, vec0)), 5),
				_mm256_srai_epi16(_mm256_sub_epi16(vecYhigh, _mm256_unpackhi_epi16(vec0, vec0)), 5)
			);
			
			/* Store result */
			COMPV_VST4_U8_AVX2(&rgbaPtr[k], vecR, vecG, vecB, vecA, vec0, vec1);

		} // End_Of for (i = 0; i < width; i += 32)
		yPtr += stride;
		rgbaPtr += strideRGBA;
		if (j & 1) {
			uPtr += strideUV;
			vPtr += strideUV;
		} // End_Of for (j = 0; j < height; ++j)
	}
	_mm256_zeroupper();
}

void CompVImageConvYuv420p_to_Rgb24_Intrin_AVX2(COMPV_ALIGNED(AVX) const uint8_t* yPtr, COMPV_ALIGNED(AVX) const uint8_t* uPtr, COMPV_ALIGNED(AVX) const uint8_t* vPtr, COMPV_ALIGNED(AVX) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(AVX) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_AVX2();
	_mm256_zeroupper();

	compv_uscalar_t i, j, k, l;
	const compv_uscalar_t strideUV = ((stride + 1) >> 1);
	const compv_uscalar_t strideRGB = stride + (stride << 1);
	__m256i vecYlow, vecYhigh, vecU, vecV, vecR, vecG, vecB;
	__m256i vec0, vec1;
	__m128i vecUn, vecVn, vecRn, vecGn, vecBn, vec0n, vec1n;
	// To avoid AVX transition issues keep the next static variables local
	static const __m256i vecZero = _mm256_setzero_si256();
	static const __m256i vec16 = _mm256_set1_epi16(16);
	static const __m256i vec37 = _mm256_set1_epi16(37);
	static const __m256i vec51 = _mm256_set1_epi16(51);
	static const __m256i vec65 = _mm256_set1_epi16(65);
	static const __m256i vec127 = _mm256_set1_epi16(127);
	static const __m256i vec13_26 = _mm256_set1_epi32(0x001a000d); // 13, 26, 13, 26 ...

	// ASM code do not create variables for k and l: k = [(i<<2)] and l = [i>>1]

	for (j = 0; j < height; ++j) {
		for (i = 0, k = 0, l = 0; i < width; i += 32, k += 96, l += 16) {
			/* Load samples */
			vecYlow = _mm256_load_si256(reinterpret_cast<const __m256i*>(&yPtr[i])); // #16 Y samples
			vecUn = _mm_load_si128(reinterpret_cast<const __m128i*>(&uPtr[l])); // #8 U samples, low mem
			vecVn = _mm_load_si128(reinterpret_cast<const __m128i*>(&vPtr[l])); // #8 V samples, low mem

			/* == Staring this line we're just converting from Y,U,V to R,G,B == */

			/* Convert to I16 */
			vecYhigh = _mm256_unpackhi_epi8(vecYlow, vecZero);
			vecYlow = _mm256_unpacklo_epi8(vecYlow, vecZero);
			vecU = _mm256_cvtepu8_epi16(vecUn);
			vecV = _mm256_cvtepu8_epi16(vecVn);

			/* Compute Y', U', V' */
			vecYlow = _mm256_sub_epi16(vecYlow, vec16);
			vecYhigh = _mm256_sub_epi16(vecYhigh, vec16);
			vecU = _mm256_sub_epi16(vecU, vec127);
			vecV = _mm256_sub_epi16(vecV, vec127);

			/* Compute (37Y'), (51V') and (65U') */
			vecYlow = _mm256_mullo_epi16(vec37, vecYlow);
			vecYhigh = _mm256_mullo_epi16(vec37, vecYhigh);
			vec0 = _mm256_mullo_epi16(vec51, vecV);
			vec1 = _mm256_mullo_epi16(vec65, vecU);

			/* Compute R = (37Y' + 0U' + 51V') >> 5 */
			vecR = _mm256_packus_epi16(
				_mm256_srai_epi16(_mm256_add_epi16(vecYlow, _mm256_unpacklo_epi16(vec0, vec0)), 5),
				_mm256_srai_epi16(_mm256_add_epi16(vecYhigh, _mm256_unpackhi_epi16(vec0, vec0)), 5)
			);

			/* B = (37Y' + 65U' + 0V') >> 5 */
			vecB = _mm256_packus_epi16(
				_mm256_srai_epi16(_mm256_add_epi16(vecYlow, _mm256_unpacklo_epi16(vec1, vec1)), 5),
				_mm256_srai_epi16(_mm256_add_epi16(vecYhigh, _mm256_unpackhi_epi16(vec1, vec1)), 5)
			);

			/* Compute G = (37Y' - 13U' - 26V') >> 5 = (37Y' - (13U' + 26V')) >> 5 */
			vec0 = _mm256_madd_epi16(vec13_26, _mm256_unpacklo_epi16(vecU, vecV)); // (13U' + 26V').low - I32
			vec1 = _mm256_madd_epi16(vec13_26, _mm256_unpackhi_epi16(vecU, vecV)); // (13U' + 26V').high - I32
			vec0 = _mm256_packs_epi32(vec0, vec1);
			vecG = _mm256_packus_epi16(
				_mm256_srai_epi16(_mm256_sub_epi16(vecYlow, _mm256_unpacklo_epi16(vec0, vec0)), 5),
				_mm256_srai_epi16(_mm256_sub_epi16(vecYhigh, _mm256_unpackhi_epi16(vec0, vec0)), 5)
			);

			/* Store result */
			//!\\ AVX<->SSE transition issues if code code compiled with /AVX2 flag. 
			//		Even if this is done, you should not trust the compiler, enable ASM to make sure.
			vecRn = _mm256_castsi256_si128(vecR);
			vecGn = _mm256_castsi256_si128(vecG);
			vecBn = _mm256_castsi256_si128(vecB);
			COMPV_VST3_U8_SSSE3(&rgbPtr[k], vecRn, vecGn, vecBn, vec0n, vec1n);
			vecRn = _mm256_extractf128_si256(vecR, 0x1);
			vecGn = _mm256_extractf128_si256(vecG, 0x1);
			vecBn = _mm256_extractf128_si256(vecB, 0x1);
			COMPV_VST3_U8_SSSE3(&rgbPtr[k + 48], vecRn, vecGn, vecBn, vec0n, vec1n);

		} // End_Of for (i = 0; i < width; i += 32)
		yPtr += stride;
		rgbPtr += strideRGB;
		if (j & 1) {
			uPtr += strideUV;
			vPtr += strideUV;
		} // End_Of for (j = 0; j < height; ++j)
	}
	_mm256_zeroupper();
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
