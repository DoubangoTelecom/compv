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

#define rgb24_store(ptr, vecR, vecG, vecB, vecA, vectmp0, vectmp1)  { \
	vecRn = _mm256_castsi256_si128(vecR); \
	vecGn = _mm256_castsi256_si128(vecG); \
	vecBn = _mm256_castsi256_si128(vecB); \
	COMPV_VST3_U8_SSSE3(&ptr[0], vecRn, vecGn, vecBn, vec0n, vec1n); \
	vecRn = _mm256_extractf128_si256(vecR, 0x1); \
	vecGn = _mm256_extractf128_si256(vecG, 0x1); \
	vecBn = _mm256_extractf128_si256(vecB, 0x1); \
	COMPV_VST3_U8_SSSE3(&ptr[48], vecRn, vecGn, vecBn, vec0n, vec1n); \
}
#define rgba32_store(ptr, vecR, vecG, vecB, vecA, vectmp0, vectmp1) { \
	(void)(vecRn); (void)(vecGn); (void)(vecBn); (void)(vec0n); (void)(vec1n); \
	COMPV_VST4_U8_AVX2(ptr, vecR, vecG, vecB, vecA, vectmp0, vectmp1); \
}

#define rgb24_step						96 /* (32 * 3) */
#define rgba32_step						128 /* (32 * 4) */

#define rgb24_bytes_per_sample			3
#define rgba32_bytes_per_sample			4

#define yuv420p_uv_step					16
#define yuv422p_uv_step					16
#define yuv444p_uv_step					32
#define nv12_uv_step					32
#define nv21_uv_step					32

#define yuv420p_uv_stride				(stride >> 1) /* no need for "((stride + 1) >> 1)" because stride is even (aligned on #16 bytes) */
#define yuv422p_uv_stride				(stride >> 1) /* no need for "((stride + 1) >> 1)" because stride is even (aligned on #16 bytes) */
#define yuv444p_uv_stride				(stride)
#define nv12_uv_stride					(stride)
#define nv21_uv_stride					(stride)

#define yuv420p_u_load					vecUn = _mm_load_si128(reinterpret_cast<const __m128i*>(&uPtr[l]))
#define yuv422p_u_load					vecUn = _mm_load_si128(reinterpret_cast<const __m128i*>(&uPtr[l]))
#define yuv444p_u_load					vecUlo = _mm256_load_si256(reinterpret_cast<const __m256i*>(&uPtr[l]))
#define nv12_u_load						vecUlo = _mm256_shuffle_epi8(_mm256_load_si256(reinterpret_cast<const __m256i*>(&uvPtr[l])), vecDeinterleaveUV)
#define nv21_u_load						vecVlo = _mm256_shuffle_epi8(_mm256_load_si256(reinterpret_cast<const __m256i*>(&uvPtr[l])), vecDeinterleaveUV)

#define yuv420p_v_load					vecVn = _mm_load_si128(reinterpret_cast<const __m128i*>(&vPtr[l]))
#define yuv422p_v_load					vecVn = _mm_load_si128(reinterpret_cast<const __m128i*>(&vPtr[l]))
#define yuv444p_v_load					vecVlo = _mm256_load_si256(reinterpret_cast<const __m256i*>(&vPtr[l]))
#define nv12_v_load						vecVlo = _mm256_unpackhi_epi64(vecUlo, vecUlo)
#define nv21_v_load						vecUlo = _mm256_unpackhi_epi64(vecVlo, vecVlo)

#define yuv420p_uv_inc_check			if (j & 1)
#define yuv422p_uv_inc_check 
#define yuv444p_uv_inc_check
#define nv12_uv_inc_check				if (j & 1)
#define nv21_uv_inc_check				if (j & 1)

#define yuv420p_uv_inc					(uPtr) += strideUV; (vPtr) += strideUV
#define yuv422p_uv_inc					(uPtr) += strideUV; (vPtr) += strideUV
#define yuv444p_uv_inc					(uPtr) += strideUV; (vPtr) += strideUV
#define nv12_uv_inc						(uvPtr) += strideUV
#define nv21_uv_inc						(uvPtr) += strideUV

#define yuv420p_u_unpacklo				vecUlo = _mm256_cvtepu8_epi16(vecUn)
#define yuv422p_u_unpacklo				vecUlo = _mm256_cvtepu8_epi16(vecUn)
#define yuv444p_u_unpacklo				vecUlo = _mm256_cvtepu8_epi16(_mm256_castsi256_si128(vecUlo))
#define nv12_u_unpacklo					vecUlo = _mm256_cvtepu8_epi16(_mm256_castsi256_si128(vecUlo))
#define nv21_u_unpacklo					vecUlo = _mm256_cvtepu8_epi16(_mm256_castsi256_si128(vecUlo))

#define yuv420p_v_unpacklo				vecVlo = _mm256_cvtepu8_epi16(vecVn)
#define yuv422p_v_unpacklo				vecVlo = _mm256_cvtepu8_epi16(vecVn)
#define yuv444p_v_unpacklo				vecVlo = _mm256_cvtepu8_epi16(_mm256_castsi256_si128(vecVlo))
#define nv12_v_unpacklo					vecVlo = _mm256_cvtepu8_epi16(_mm256_castsi256_si128(vecVlo))
#define nv21_v_unpacklo					vecVlo = _mm256_cvtepu8_epi16(_mm256_castsi256_si128(vecVlo))

#define yuv420p_u_unpackhi				(void)(vecUhi)
#define yuv422p_u_unpackhi				(void)(vecUhi)
#define yuv444p_u_unpackhi				vecUhi = _mm256_cvtepu8_epi16(_mm256_extractf128_si256(vecUlo, 0x1))
#define nv12_u_unpackhi					(void)(vecUhi)
#define nv21_u_unpackhi					(void)(vecUhi)

#define yuv420p_v_unpackhi				(void)(vecVhi)
#define yuv422p_v_unpackhi				(void)(vecVhi)
#define yuv444p_v_unpackhi				vecVhi = _mm256_cvtepu8_epi16(_mm256_extractf128_si256(vecVlo, 0x1))
#define nv12_v_unpackhi					(void)(vecVhi)
#define nv21_v_unpackhi					(void)(vecVhi)

#define yuv420p_u_primehi				(void)(vecUhi)
#define yuv422p_u_primehi				(void)(vecUhi)
#define yuv444p_u_primehi				vecUhi = _mm256_sub_epi16(vecUhi, vec127)
#define nv12_u_primehi					(void)(vecUhi)
#define nv21_u_primehi					(void)(vecUhi)

#define yuv420p_v_primehi				(void)(vecVhi)
#define yuv422p_v_primehi				(void)(vecVhi)
#define yuv444p_v_primehi				vecVhi = _mm256_sub_epi16(vecVhi, vec127)
#define nv12_v_primehi					(void)(vecVhi)
#define nv21_v_primehi					(void)(vecVhi)

#define yuv420p_u_primehi65				(void)(vec1hi)
#define yuv422p_u_primehi65				(void)(vec1hi)
#define yuv444p_u_primehi65				vec1hi = _mm256_mullo_epi16(vecUhi, vec65)
#define nv12_u_primehi65				(void)(vec1hi)
#define nv21_u_primehi65				(void)(vec1hi)

#define yuv420p_v_primehi51				(void)(vec0hi)
#define yuv422p_v_primehi51				(void)(vec0hi)
#define yuv444p_v_primehi51				vec0hi = _mm_mullo_epi16(vecVhi, vec51)
#define nv12_v_primehi51				(void)(vec0hi)
#define nv21_v_primehi51				(void)(vec0hi)

#define yuv420p_final_vec(vec, p)		vec##p = _mm256_unpack##p##_epi16(vec##lo, vec##lo)
#define yuv422p_final_vec(vec, p)		vec##p = _mm256_unpack##p##_epi16(vec##lo, vec##lo)
#define yuv444p_final_vec(vec, p)		(void)(vec##p)
#define nv12_final_vec(vec, p)			vec##p = _mm256_unpack##p##_epi16(vec##lo, vec##lo)
#define nv21_final_vec(vec, p)			vec##p = _mm256_unpack##p##_epi16(vec##lo, vec##lo)

#define yuv420p_g_high					(void)(vec0hi)
#define yuv422p_g_high					(void)(vec0hi)
#define yuv444p_g_high					vec1lo = _mm256_madd_epi16(_mm256_unpacklo_epi16(vecUhi, vecVhi), vec13_26); vec1hi = _mm256_madd_epi16(_mm256_unpackhi_epi16(vecUhi, vecVhi), vec13_26); vec0hi = _mm256_packs_epi32(vec1lo, vec1hi)
#define nv12_g_high						(void)(vec0hi)
#define nv21_g_high						(void)(vec0hi)

#define CompVImageConvYuvPlanar_to_Rgbx_Intrin_AVX2(nameYuv, nameRgbx) { \
	COMPV_DEBUG_INFO_CHECK_AVX2(); \
	_mm256_zeroupper(); \
	 \
	compv_uscalar_t i, j, k, l; \
	const compv_uscalar_t strideUV = nameYuv##_uv_stride;  \
	const compv_uscalar_t strideRGBx = (stride * nameRgbx##_bytes_per_sample); \
	__m256i vecYlo, vecYhi, vecUlo, vecUhi, vecVlo, vecVhi, vecR, vecG, vecB; \
	__m256i vec0lo, vec0hi, vec1lo, vec1hi; \
	__m128i vecUn, vecVn, vecRn, vecGn, vecBn, vec0n, vec1n; \
	/* To avoid AVX transition issues keep the next static variables local */ \
	static const __m256i vecZero = _mm256_setzero_si256(); \
	static const __m256i vec16 = _mm256_set1_epi16(16); \
	static const __m256i vec37 = _mm256_set1_epi16(37); \
	static const __m256i vec51 = _mm256_set1_epi16(51); \
	static const __m256i vec65 = _mm256_set1_epi16(65); \
	static const __m256i vec127 = _mm256_set1_epi16(127); \
	static const __m256i vec13_26 = _mm256_load_si256(reinterpret_cast<const __m256i*>(k13_26_i16)); /* 13, 26, 13, 26 ...*/ \
	static const __m256i vecA = _mm256_cmpeq_epi8(vec127, vec127); /* 255, 255, 255, 255 */ \
	 \
	for (j = 0; j < height; ++j) { \
		for (i = 0, k = 0, l = 0; i < width; i += 32, k += nameRgbx##_step, l += nameYuv##_uv_step) { \
			/* Load samples */ \
			vecYlo = _mm256_load_si256(reinterpret_cast<const __m256i*>(&yPtr[i])); /* #16 Y samples */ \
			nameYuv##_u_load; /* #8 or #16 U samples, low mem */ \
			nameYuv##_v_load; /* #8 or #16 V samples, low mem */ \
			 \
			/* Convert to I16 */ \
			vecYhi = _mm256_unpackhi_epi8(vecYlo, vecZero); \
			vecYlo = _mm256_unpacklo_epi8(vecYlo, vecZero); \
			nameYuv##_u_unpackhi; \
			nameYuv##_u_unpacklo; \
			nameYuv##_v_unpackhi; \
			nameYuv##_v_unpacklo; \
			 \
			/* Compute Y', U', V' */ \
			vecYlo = _mm256_sub_epi16(vecYlo, vec16); \
			vecYhi = _mm256_sub_epi16(vecYhi, vec16); \
			vecUlo = _mm256_sub_epi16(vecUlo, vec127); \
			nameYuv##_u_primehi; \
			vecVlo = _mm256_sub_epi16(vecVlo, vec127); \
			nameYuv##_v_primehi; \
			 \
			/* Compute (37Y'), (51V') and (65U') */ \
			vecYlo = _mm256_mullo_epi16(vecYlo, vec37); \
			vecYhi = _mm256_mullo_epi16(vecYhi, vec37); \
			vec0lo = _mm256_mullo_epi16(vecVlo, vec51); \
			nameYuv##_v_primehi51; \
			vec1lo = _mm256_mullo_epi16(vecUlo, vec65); \
			nameYuv##_u_primehi65; \
			 \
			/* Compute R = (37Y' + 0U' + 51V') >> 5 */ \
			nameYuv##_final_vec(vec0, hi); \
			nameYuv##_final_vec(vec0, lo); \
			vecR = _mm256_packus_epi16(\
				_mm256_srai_epi16(_mm256_add_epi16(vecYlo, vec0lo), 5), \
				_mm256_srai_epi16(_mm256_add_epi16(vecYhi, vec0hi), 5) \
			); \
			 \
			/* B = (37Y' + 65U' + 0V') >> 5 */ \
			nameYuv##_final_vec(vec1, hi); \
			nameYuv##_final_vec(vec1, lo); \
			vecB = _mm256_packus_epi16(\
				_mm256_srai_epi16(_mm256_add_epi16(vecYlo, vec1lo), 5), \
				_mm256_srai_epi16(_mm256_add_epi16(vecYhi, vec1hi), 5) \
			); \
			 \
			/* Compute G = (37Y' - 13U' - 26V') >> 5 = (37Y' - (13U' + 26V')) >> 5 */ \
			vec0lo = _mm256_madd_epi16(vec13_26, _mm256_unpacklo_epi16(vecUlo, vecVlo)); /* (13U' + 26V').lo - I32 */ \
			vec0hi = _mm256_madd_epi16(vec13_26, _mm256_unpackhi_epi16(vecUlo, vecVlo)); /* (13U' + 26V').hi - I32 */ \
			vec0lo = _mm256_packs_epi32(vec0lo, vec0hi); \
			nameYuv##_g_high; \
			nameYuv##_final_vec(vec0, hi); \
			nameYuv##_final_vec(vec0, lo); \
			vecG = _mm256_packus_epi16(\
				_mm256_srai_epi16(_mm256_sub_epi16(vecYlo, vec0lo), 5), \
				_mm256_srai_epi16(_mm256_sub_epi16(vecYhi, vec0hi), 5) \
			); \
			 \
			/* Store result */ \
			nameRgbx##_store(&rgbxPtr[k], vecR, vecG, vecB, vecA, vec0lo, vec1lo); \
			 \
		} /* End_Of for (i = 0; i < width; i += 32) */ \
		yPtr += stride; \
		rgbxPtr += strideRGBx; \
		nameYuv##_uv_inc_check { \
			nameYuv##_uv_inc; \
		} \
	} /* End_Of for (j = 0; j < height; ++j) */ \
	_mm256_zeroupper(); \
}

#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx2
#endif
void CompVImageConvYuv420p_to_Rgba32_Intrin_AVX2(COMPV_ALIGNED(AVX) const uint8_t* yPtr, COMPV_ALIGNED(AVX) const uint8_t* uPtr, COMPV_ALIGNED(AVX) const uint8_t* vPtr, COMPV_ALIGNED(AVX) uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(AVX) compv_uscalar_t stride)
{
	CompVImageConvYuvPlanar_to_Rgbx_Intrin_AVX2(yuv420p, rgba32);
}

#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx2
#endif
void CompVImageConvYuv420p_to_Rgb24_Intrin_AVX2(COMPV_ALIGNED(AVX) const uint8_t* yPtr, COMPV_ALIGNED(AVX) const uint8_t* uPtr, COMPV_ALIGNED(AVX) const uint8_t* vPtr, COMPV_ALIGNED(AVX) uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(AVX) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_AVX2();
	_mm256_zeroupper();

	compv_uscalar_t i, j, k, l;
	const compv_uscalar_t strideUV = ((stride + 1) >> 1);
	const compv_uscalar_t strideRGB = stride + (stride << 1);
	__m256i vecYlo, vecYhi, vecU, vecV, vecR, vecG, vecB;
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
			vecYlo = _mm256_load_si256(reinterpret_cast<const __m256i*>(&yPtr[i])); // #16 Y samples
			vecUn = _mm_load_si128(reinterpret_cast<const __m128i*>(&uPtr[l])); // #8 U samples, low mem
			vecVn = _mm_load_si128(reinterpret_cast<const __m128i*>(&vPtr[l])); // #8 V samples, low mem

			/* == Staring this line we're just converting from Y,U,V to R,G,B == */

			/* Convert to I16 */
			vecYhi = _mm256_unpackhi_epi8(vecYlo, vecZero);
			vecYlo = _mm256_unpacklo_epi8(vecYlo, vecZero);
			vecU = _mm256_cvtepu8_epi16(vecUn);
			vecV = _mm256_cvtepu8_epi16(vecVn);

			/* Compute Y', U', V' */
			vecYlo = _mm256_sub_epi16(vecYlo, vec16);
			vecYhi = _mm256_sub_epi16(vecYhi, vec16);
			vecU = _mm256_sub_epi16(vecU, vec127);
			vecV = _mm256_sub_epi16(vecV, vec127);

			/* Compute (37Y'), (51V') and (65U') */
			vecYlo = _mm256_mullo_epi16(vec37, vecYlo);
			vecYhi = _mm256_mullo_epi16(vec37, vecYhi);
			vec0 = _mm256_mullo_epi16(vec51, vecV);
			vec1 = _mm256_mullo_epi16(vec65, vecU);

			/* Compute R = (37Y' + 0U' + 51V') >> 5 */
			vecR = _mm256_packus_epi16(
				_mm256_srai_epi16(_mm256_add_epi16(vecYlo, _mm256_unpacklo_epi16(vec0, vec0)), 5),
				_mm256_srai_epi16(_mm256_add_epi16(vecYhi, _mm256_unpackhi_epi16(vec0, vec0)), 5)
			);

			/* B = (37Y' + 65U' + 0V') >> 5 */
			vecB = _mm256_packus_epi16(
				_mm256_srai_epi16(_mm256_add_epi16(vecYlo, _mm256_unpacklo_epi16(vec1, vec1)), 5),
				_mm256_srai_epi16(_mm256_add_epi16(vecYhi, _mm256_unpackhi_epi16(vec1, vec1)), 5)
			);

			/* Compute G = (37Y' - 13U' - 26V') >> 5 = (37Y' - (13U' + 26V')) >> 5 */
			vec0 = _mm256_madd_epi16(vec13_26, _mm256_unpacklo_epi16(vecU, vecV)); // (13U' + 26V').low - I32
			vec1 = _mm256_madd_epi16(vec13_26, _mm256_unpackhi_epi16(vecU, vecV)); // (13U' + 26V').high - I32
			vec0 = _mm256_packs_epi32(vec0, vec1);
			vecG = _mm256_packus_epi16(
				_mm256_srai_epi16(_mm256_sub_epi16(vecYlo, _mm256_unpacklo_epi16(vec0, vec0)), 5),
				_mm256_srai_epi16(_mm256_sub_epi16(vecYhi, _mm256_unpackhi_epi16(vec0, vec0)), 5)
			);

			/* Store result */
			//!\\ AVX<->SSE transition issues if code code compiled with /AVX2 flag. 
			//		Even if this is done, you should not trust the compiler, enable ASM to make sure.
			vecRn = _mm256_castsi256_si128(vecR);
			vecGn = _mm256_castsi256_si128(vecG);
			vecBn = _mm256_castsi256_si128(vecB);
			COMPV_VST3_U8_SSSE3(&rgbxPtr[k], vecRn, vecGn, vecBn, vec0n, vec1n);
			vecRn = _mm256_extractf128_si256(vecR, 0x1);
			vecGn = _mm256_extractf128_si256(vecG, 0x1);
			vecBn = _mm256_extractf128_si256(vecB, 0x1);
			COMPV_VST3_U8_SSSE3(&rgbxPtr[k + 48], vecRn, vecGn, vecBn, vec0n, vec1n);

		} // End_Of for (i = 0; i < width; i += 32)
		yPtr += stride;
		rgbxPtr += strideRGB;
		if (j & 1) {
			uPtr += strideUV;
			vPtr += strideUV;
		} // End_Of for (j = 0; j < height; ++j)
	}
	_mm256_zeroupper();
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
