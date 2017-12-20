/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/ccl/intrin/x86/compv_core_ccl_lsl_intrin_ssse3.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/core/compv_core_simd_globals.h"
#include "compv/base/compv_simd_globals.h"
#include "compv/base/intrin/x86/compv_intrin_sse.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// Function requires width > 16 (not ">=" but ">")
void CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Intrin_SSSE3(
	COMPV_ALIGNED(SSE) const uint8_t* Xi, const compv_uscalar_t Xi_stride,
	int16_t* ERi, 
	int16_t* ner, int16_t* ner_max1, int32_t* ner_sum1,
	const compv_uscalar_t width, const compv_uscalar_t height
)
{
	COMPV_DEBUG_INFO_CHECK_SSSE3();
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Should use AVX2 code which is faster (requires enabling asm)");
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Asm code faster");
	int16_t i, er, ner_max = 0;
	const int16_t width1 = static_cast<int16_t>(width);
	const int16_t width16 = (width1 - 1) & -16; // width > 16 (at least 17) which means never equal to zero
	int32_t ner_sum = 0;
	__m128i vec0, vec1, vecER;
	const __m128i vecOne = _mm_set1_epi8(1);
	const __m128i vecZero = _mm_setzero_si128();
	const __m128i vecDup7th16s = _mm_load_si128(reinterpret_cast<const __m128i*>(kShuffleEpi8_DUP_16s7_32s));

	// TODO(dmi): for ARM neon do not use vtbl to replace _mm_shufle_epi8 but
	// vdup.u8 dn, d1[n] then shift right to add zeros
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

	for (compv_uscalar_t j = 0; j < height; ++j) {
		er = (Xi[0] & 1);
		ERi[0] = er;
		vecER = _mm_set1_epi16(er);

		// In asm code, no need to test "width16 != 0" because "width1" > 16 (at least 17)

		for (i = 1; i < width16; i += 16) {
			/* Xi[i - 1] ^ Xi[i] & 1 */
			vec0 = _mm_xor_si128(
				_mm_load_si128(reinterpret_cast<const __m128i*>(&Xi[i - 1])),
				_mm_loadu_si128(reinterpret_cast<const __m128i*>(&Xi[i]))
			);
			vec0 = _mm_and_si128(vec0, vecOne);

			if (_mm_movemask_epi8(_mm_cmpgt_epi8(vec0, vecZero))) { // ARM neon: use COMPV_ARM_NEON_NEQ_ZEROQ(vec0)
				/* erUint8 += ((Xi[i - 1] ^ Xi[i]) & 1) */
				vec1 = _mm_shuffle_epi8(vec0, __vecMask0);
				vec1 = _mm_add_epi8(vec1, _mm_shuffle_epi8(vec0, __vecMask1));
				vec1 = _mm_add_epi8(vec1, _mm_shuffle_epi8(vec0, __vecMask2));
				vec1 = _mm_add_epi8(vec1, _mm_shuffle_epi8(vec0, __vecMask3));
				vec1 = _mm_add_epi8(vec1, _mm_shuffle_epi8(vec0, __vecMask4));
				vec1 = _mm_add_epi8(vec1, _mm_shuffle_epi8(vec0, __vecMask5));
				vec1 = _mm_add_epi8(vec1, _mm_shuffle_epi8(vec0, __vecMask6));
				vec1 = _mm_add_epi8(vec1, _mm_shuffle_epi8(vec0, __vecMask7));
				vec1 = _mm_add_epi8(vec1, _mm_shuffle_epi8(vec0, __vecMask8));
				vec1 = _mm_add_epi8(vec1, _mm_shuffle_epi8(vec0, __vecMask9));
				vec1 = _mm_add_epi8(vec1, _mm_shuffle_epi8(vec0, __vecMask10));
				vec1 = _mm_add_epi8(vec1, _mm_shuffle_epi8(vec0, __vecMask11));
				vec1 = _mm_add_epi8(vec1, _mm_shuffle_epi8(vec0, __vecMask12));
				vec1 = _mm_add_epi8(vec1, _mm_shuffle_epi8(vec0, __vecMask13));
				vec1 = _mm_add_epi8(vec1, _mm_shuffle_epi8(vec0, __vecMask14));
				vec1 = _mm_add_epi8(vec1, _mm_shuffle_epi8(vec0, __vecMask15));

				/* Convert erUint8 to erInt16 and sum then store */
				vec0 = _mm_add_epi16(vecER, _mm_unpacklo_epi8(vec1, vecZero));
				vecER = _mm_add_epi16(vecER, _mm_unpackhi_epi8(vec1, vecZero));
				_mm_storeu_si128(reinterpret_cast<__m128i*>(&ERi[i]), vec0);
				_mm_storeu_si128(reinterpret_cast<__m128i*>(&ERi[i + 8]), vecER);

				/* Duplicate latest element */
				vecER = _mm_shuffle_epi8(vecER, vecDup7th16s);
			}
			else {
				/* Store previous ER */
				_mm_storeu_si128(reinterpret_cast<__m128i*>(&ERi[i]), vecER);
				_mm_storeu_si128(reinterpret_cast<__m128i*>(&ERi[i + 8]), vecER);
			}
		}

		/* Get highest "er" before switching from SIMD to serial code */
		er = _mm_extract_epi16(vecER, 7);

		for (; i < width1; ++i) {
			er += ((Xi[i - 1] ^ Xi[i]) & 1);
			ERi[i] = er;
		}
		er += (Xi[width1 - 1] & 1);
		ner[j] = er;
		ner_sum += er;
		if (ner_max < er) { // TODO(dmi): asm use cmovl
			ner_max = er;
		}
		/* next */
		Xi += Xi_stride;
		ERi += width;
	}

	*ner_max1 = ner_max;
	*ner_sum1 = ner_sum;
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */