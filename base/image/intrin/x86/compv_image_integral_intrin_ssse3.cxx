/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/intrin/x86/compv_image_integral_intrin_ssse3.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

#if 0
static const COMPV_ALIGN_SSE() int32_t kShuffleEpi8_DUP_SHL0_8u_32s[]{ // To be used with _mm_shuffle_epi8, use vdup.u8[0]/vshl.u8#0 for ARM NEON
	COMPV_MM_SHUFFLE_EPI8(0, 0, 0, 0), COMPV_MM_SHUFFLE_EPI8(0, 0, 0, 0), COMPV_MM_SHUFFLE_EPI8(0, 0, 0, 0), COMPV_MM_SHUFFLE_EPI8(0, 0, 0, 0), // 128bits SSE register
};
static const COMPV_ALIGN_SSE() int32_t kShuffleEpi8_DUP_SHL1_8u_32s[] = { // To be used with _mm_shuffle_epi8, use vdup.u8[1]/vshl.u8#1 for ARM NEON
	COMPV_MM_SHUFFLE_EPI8(1, 1, 1, 0x80), COMPV_MM_SHUFFLE_EPI8(1, 1, 1, 1), COMPV_MM_SHUFFLE_EPI8(1, 1, 1, 1), COMPV_MM_SHUFFLE_EPI8(1, 1, 1, 1), // 128bits SSE register
};
static const COMPV_ALIGN_SSE() int32_t kShuffleEpi8_DUP_SHL2_8u_32s[] = { // To be used with _mm_shuffle_epi8, use vdup.u8[2]/vshl.u8#2 for ARM NEON
	COMPV_MM_SHUFFLE_EPI8(2, 2, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(2, 2, 2, 2), COMPV_MM_SHUFFLE_EPI8(2, 2, 2, 2), COMPV_MM_SHUFFLE_EPI8(2, 2, 2, 2), // 128bits SSE register
};
static const COMPV_ALIGN_SSE() int32_t kShuffleEpi8_DUP_SHL3_8u_32s[] = { // To be used with _mm_shuffle_epi8, use vdup.u8[3]/vshl.u8#3 for ARM NEON
	COMPV_MM_SHUFFLE_EPI8(3, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(3, 3, 3, 3), COMPV_MM_SHUFFLE_EPI8(3, 3, 3, 3), COMPV_MM_SHUFFLE_EPI8(3, 3, 3, 3), // 128bits SSE register
};
static const COMPV_ALIGN_SSE() int32_t kShuffleEpi8_DUP_SHL4_8u_32s[] = { // To be used with _mm_shuffle_epi8, use vdup.u8[4]/vshl.u8#4 for ARM NEON
	COMPV_MM_SHUFFLE_EPI8(0x80, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(4, 4, 4, 4), COMPV_MM_SHUFFLE_EPI8(4, 4, 4, 4), COMPV_MM_SHUFFLE_EPI8(4, 4, 4, 4), // 128bits SSE register
};
static const COMPV_ALIGN_SSE() int32_t kShuffleEpi8_DUP_SHL5_8u_32s[] = { // To be used with _mm_shuffle_epi8, use vdup.u8[5]/vshl.u8#5 for ARM NEON
	COMPV_MM_SHUFFLE_EPI8(0x80, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(5, 5, 5, 0x80), COMPV_MM_SHUFFLE_EPI8(5, 5, 5, 5), COMPV_MM_SHUFFLE_EPI8(5, 5, 5, 5), // 128bits SSE register
};
static const COMPV_ALIGN_SSE() int32_t kShuffleEpi8_DUP_SHL6_8u_32s[] = { // To be used with _mm_shuffle_epi8, use vdup.u8[6]/vshl.u8#6 for ARM NEON
	COMPV_MM_SHUFFLE_EPI8(0x80, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(6, 6, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(6, 6, 6, 6), COMPV_MM_SHUFFLE_EPI8(6, 6, 6, 6), // 128bits SSE register
};
static const COMPV_ALIGN_SSE() int32_t kShuffleEpi8_DUP_SHL7_8u_32s[] = { // To be used with _mm_shuffle_epi8, use vdup.u8[7]/vshl.u8#7 for ARM NEON
	COMPV_MM_SHUFFLE_EPI8(0x80, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(7, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(7, 7, 7, 7), COMPV_MM_SHUFFLE_EPI8(7, 7, 7, 7), // 128bits SSE register
};
static const COMPV_ALIGN_SSE() int32_t kShuffleEpi8_DUP_SHL8_8u_32s[] = { // To be used with _mm_shuffle_epi8, use vdup.u8[8]/vshl.u8#8 for ARM NEON
	COMPV_MM_SHUFFLE_EPI8(0x80, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(0x80, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(8, 8, 8, 8), COMPV_MM_SHUFFLE_EPI8(8, 8, 8, 8), // 128bits SSE register
};
static const COMPV_ALIGN_SSE() int32_t kShuffleEpi8_DUP_SHL9_8u_32s[] = { // To be used with _mm_shuffle_epi8, use vdup.u8[9]/vshl.u8#9 for ARM NEON
	COMPV_MM_SHUFFLE_EPI8(0x80, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(0x80, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(9, 9, 9, 0x80), COMPV_MM_SHUFFLE_EPI8(9, 9, 9, 9), // 128bits SSE register
};
static const COMPV_ALIGN_SSE() int32_t kShuffleEpi8_DUP_SHL10_8u_32s[] = { // To be used with _mm_shuffle_epi8, use vdup.u8[10]/vshl.u8#10 for ARM NEON
	COMPV_MM_SHUFFLE_EPI8(0x80, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(0x80, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(10, 10, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(10, 10, 10, 10), // 128bits SSE register
};
static const COMPV_ALIGN_SSE() int32_t kShuffleEpi8_DUP_SHL11_8u_32s[] = { // To be used with _mm_shuffle_epi8, use vdup.u8[11]/vshl.u8#11 for ARM NEON
	COMPV_MM_SHUFFLE_EPI8(0x80, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(0x80, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(11, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(11, 11, 11, 11), // 128bits SSE register
};
static const COMPV_ALIGN_SSE() int32_t kShuffleEpi8_DUP_SHL12_8u_32s[] = { // To be used with _mm_shuffle_epi8, use vdup.u8[12]/vshl.u8#12 for ARM NEON
	COMPV_MM_SHUFFLE_EPI8(0x80, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(0x80, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(0x80, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(12, 12, 12, 12), // 128bits SSE register
};
static const COMPV_ALIGN_SSE() int32_t kShuffleEpi8_DUP_SHL13_8u_32s[] = { // To be used with _mm_shuffle_epi8, use vdup.u8[13]/vshl.u8#13 for ARM NEON
	COMPV_MM_SHUFFLE_EPI8(0x80, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(0x80, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(0x80, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(13, 13, 13, 0x80), // 128bits SSE register
};
static const COMPV_ALIGN_SSE() int32_t kShuffleEpi8_DUP_SHL14_8u_32s[] = { // To be used with _mm_shuffle_epi8, use vdup.u8[14]/vshl.u8#14 for ARM NEON
	COMPV_MM_SHUFFLE_EPI8(0x80, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(0x80, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(0x80, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(14, 14, 0x80, 0x80), // 128bits SSE register
};
static const COMPV_ALIGN_SSE() int32_t kShuffleEpi8_DUP_SHL15_8u_32s[] = { // To be used with _mm_shuffle_epi8, use vdup.u8[15]/vshl.u8#15 for ARM NEON
	COMPV_MM_SHUFFLE_EPI8(0x80, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(0x80, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(0x80, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(15, 0x80, 0x80, 0x80), // 128bits SSE register
};
#endif

void CompVImageIntegralProcess_8u64f_Intrin_SSSE3(const uint8_t* in, compv_float64_t* sum, compv_float64_t* sumsq, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t in_stride, const compv_uscalar_t sum_stride)
{
	COMPV_DEBUG_INFO_CHECK_SSSE3();
	COMPV_ASSERT(false); // Not implemented yet
#if 0
	static const __m128i __vecMask0 = _mm_load_si128(reinterpret_cast<const __m128i*>(kShuffleEpi8_DUP_SHL0_8u_32s));
	static const __m128i __vecMask1 = _mm_load_si128(reinterpret_cast<const __m128i*>(kShuffleEpi8_DUP_SHL1_8u_32s));
	static const __m128i __vecMask2 = _mm_load_si128(reinterpret_cast<const __m128i*>(kShuffleEpi8_DUP_SHL2_8u_32s));
	static const __m128i __vecMask3 = _mm_load_si128(reinterpret_cast<const __m128i*>(kShuffleEpi8_DUP_SHL3_8u_32s));
	static const __m128i __vecMask4 = _mm_load_si128(reinterpret_cast<const __m128i*>(kShuffleEpi8_DUP_SHL4_8u_32s));
	static const __m128i __vecMask5 = _mm_load_si128(reinterpret_cast<const __m128i*>(kShuffleEpi8_DUP_SHL5_8u_32s));
	static const __m128i __vecMask6 = _mm_load_si128(reinterpret_cast<const __m128i*>(kShuffleEpi8_DUP_SHL6_8u_32s));
	static const __m128i __vecMask7 = _mm_load_si128(reinterpret_cast<const __m128i*>(kShuffleEpi8_DUP_SHL7_8u_32s));
	static const __m128i __vecMask8 = _mm_load_si128(reinterpret_cast<const __m128i*>(kShuffleEpi8_DUP_SHL8_8u_32s));
	static const __m128i __vecMask9 = _mm_load_si128(reinterpret_cast<const __m128i*>(kShuffleEpi8_DUP_SHL9_8u_32s));
	static const __m128i __vecMask10 = _mm_load_si128(reinterpret_cast<const __m128i*>(kShuffleEpi8_DUP_SHL10_8u_32s));
	static const __m128i __vecMask11 = _mm_load_si128(reinterpret_cast<const __m128i*>(kShuffleEpi8_DUP_SHL11_8u_32s));
	static const __m128i __vecMask12 = _mm_load_si128(reinterpret_cast<const __m128i*>(kShuffleEpi8_DUP_SHL12_8u_32s));
	static const __m128i __vecMask13 = _mm_load_si128(reinterpret_cast<const __m128i*>(kShuffleEpi8_DUP_SHL13_8u_32s));
	static const __m128i __vecMask14 = _mm_load_si128(reinterpret_cast<const __m128i*>(kShuffleEpi8_DUP_SHL14_8u_32s));
	static const __m128i __vecMask15 = _mm_load_si128(reinterpret_cast<const __m128i*>(kShuffleEpi8_DUP_SHL15_8u_32s));

	const __m128i vecZero = _mm_setzero_si128();
	const compv_uscalar_t width16 = width & -16;
	for (compv_uscalar_t j = 0; j < height; j++) {

		sum[-1] = 0;
		const compv_float64_t* sump = sum - sum_stride;
		compv_uscalar_t i = 0;
		for (; i < width16; i += 16) {
			const __m128i vecIn = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&in[i]));
			__m128i vec0 = _mm_shuffle_epi8(vecIn, __vecMask0);
			__m128i vec1 = _mm_shuffle_epi8(vecIn, __vecMask1);
			__m128i vec2 = _mm_shuffle_epi8(vecIn, __vecMask2);
			__m128i vec3 = _mm_shuffle_epi8(vecIn, __vecMask3);
			__m128i vec4 = _mm_shuffle_epi8(vecIn, __vecMask4);
			__m128i vec5 = _mm_shuffle_epi8(vecIn, __vecMask5);
			__m128i vec6 = _mm_shuffle_epi8(vecIn, __vecMask6);
			__m128i vec7 = _mm_shuffle_epi8(vecIn, __vecMask7);
			__m128i vec8 = _mm_shuffle_epi8(vecIn, __vecMask8);
			__m128i vec9 = _mm_shuffle_epi8(vecIn, __vecMask9);
			__m128i vec10 = _mm_shuffle_epi8(vecIn, __vecMask10);
			__m128i vec11 = _mm_shuffle_epi8(vecIn, __vecMask11);
			__m128i vec12 = _mm_shuffle_epi8(vecIn, __vecMask12);
			__m128i vec13 = _mm_shuffle_epi8(vecIn, __vecMask13);
			__m128i vec14 = _mm_shuffle_epi8(vecIn, __vecMask14);
			__m128i vec15 = _mm_shuffle_epi8(vecIn, __vecMask15);

			// 8u -> 16s
			vec0 = _mm_add_epi16(_mm_unpacklo_epi8(vec0, vecZero), _mm_unpackhi_epi8(vec0, vecZero));
			vec1 = _mm_add_epi16(_mm_unpacklo_epi8(vec1, vecZero), _mm_unpackhi_epi8(vec1, vecZero));
			vec2 = _mm_add_epi16(_mm_unpacklo_epi8(vec2, vecZero), _mm_unpackhi_epi8(vec2, vecZero));
			vec3 = _mm_add_epi16(_mm_unpacklo_epi8(vec3, vecZero), _mm_unpackhi_epi8(vec3, vecZero));
			vec4 = _mm_add_epi16(_mm_unpacklo_epi8(vec4, vecZero), _mm_unpackhi_epi8(vec4, vecZero));
			vec5 = _mm_add_epi16(_mm_unpacklo_epi8(vec5, vecZero), _mm_unpackhi_epi8(vec5, vecZero));
			vec6 = _mm_add_epi16(_mm_unpacklo_epi8(vec6, vecZero), _mm_unpackhi_epi8(vec6, vecZero));
			vec7 = _mm_add_epi16(_mm_unpacklo_epi8(vec7, vecZero), _mm_unpackhi_epi8(vec7, vecZero));
			vec8 = _mm_add_epi16(_mm_unpacklo_epi8(vec8, vecZero), _mm_unpackhi_epi8(vec8, vecZero));
			vec9 = _mm_add_epi16(_mm_unpacklo_epi8(vec9, vecZero), _mm_unpackhi_epi8(vec9, vecZero));
			vec10 = _mm_add_epi16(_mm_unpacklo_epi8(vec10, vecZero), _mm_unpackhi_epi8(vec10, vecZero));
			vec11 = _mm_add_epi16(_mm_unpacklo_epi8(vec11, vecZero), _mm_unpackhi_epi8(vec11, vecZero));
			vec12 = _mm_add_epi16(_mm_unpacklo_epi8(vec12, vecZero), _mm_unpackhi_epi8(vec12, vecZero));
			vec13 = _mm_add_epi16(_mm_unpacklo_epi8(vec13, vecZero), _mm_unpackhi_epi8(vec13, vecZero));
			vec14 = _mm_add_epi16(_mm_unpacklo_epi8(vec14, vecZero), _mm_unpackhi_epi8(vec14, vecZero));
			vec15 = _mm_add_epi16(_mm_unpacklo_epi8(vec15, vecZero), _mm_unpackhi_epi8(vec15, vecZero));

			// Sum #16 (not overflow #16 8u)
			vec0 = _mm_add_epi16(vec0, vec1);
			vec0 = _mm_add_epi16(vec0, vec1);
			vec0 = _mm_add_epi16(vec0, vec1);
			vec0 = _mm_add_epi16(vec0, vec1);

			row_sum += in[i];

			sum[i] = row_sum + sump[i];
		}
		in += in_stride;
		sum += sum_stride;
	}
#endif
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
