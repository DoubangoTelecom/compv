/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/x86/compv_math_distance_intrin_avx2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_simd_globals.h"
#include "compv/base/compv_debug.h"
#include "compv/base/compv_bits.h"

COMPV_NAMESPACE_BEGIN()

#define __pop_count_mula_avx2(v) \
	popcnt1 = _mm256_shuffle_epi8(vecLookup, _mm256_and_si256(v, vecMaskLow)); \
	popcnt2 = _mm256_shuffle_epi8(vecLookup, _mm256_and_si256(_mm256_srli_epi32(v, 4), vecMaskLow)); \
	v = _mm256_sad_epu8(_mm256_add_epi8(popcnt1, popcnt2), _mm256_setzero_si256())


// popcnt available starting SSE4.2 but up to the caller to check its availability using CPU features
// Mula's algorithm
void CompVMathDistanceHamming32_Intrin_POPCNT_AVX2(COMPV_ALIGNED(AVX) const uint8_t* dataPtr, compv_uscalar_t height, COMPV_ALIGNED(AVX) compv_uscalar_t stride, COMPV_ALIGNED(AVX) const uint8_t* patch1xnPtr, int32_t* distPtr)
{
	COMPV_DEBUG_INFO_CHECK_AVX2();
	_mm256_zeroupper();

#if COMPV_ARCH_X64
	uint64_t cnt;
#else
	uint32_t cnt;
#endif
	compv_uscalar_t j, strideTimes4 = (stride << 2);
	__m256i vec0, vec1, vec2, vec3;
	__m128i vec0n, vec1n;
	__m256i popcnt1, popcnt2;
	const __m256i vecPatch = _mm256_load_si256(reinterpret_cast<const __m256i*>(patch1xnPtr));
	const __m256i vecLookup = _mm256_load_si256(reinterpret_cast<const __m256i*>(kShuffleEpi8_Popcnt_i32));
	const __m256i vecMaskLow = _mm256_load_si256(reinterpret_cast<const __m256i*>(k15_i8));

	j = 0;

	if (height > 3) {
		for (; j < height - 3; j += 4) {
			vec0 = _mm256_load_si256(reinterpret_cast<const __m256i*>(&dataPtr[0]));
			vec1 = _mm256_load_si256(reinterpret_cast<const __m256i*>(&dataPtr[stride]));
			vec2 = _mm256_load_si256(reinterpret_cast<const __m256i*>(&dataPtr[stride << 1]));
			vec3 = _mm256_load_si256(reinterpret_cast<const __m256i*>(&dataPtr[(stride << 1) + stride]));
			vec0 = _mm256_xor_si256(vec0, vecPatch);
			vec1 = _mm256_xor_si256(vec1, vecPatch);
			vec2 = _mm256_xor_si256(vec2, vecPatch);
			vec3 = _mm256_xor_si256(vec3, vecPatch);
			__pop_count_mula_avx2(vec0);
			__pop_count_mula_avx2(vec1);
			__pop_count_mula_avx2(vec2);
			__pop_count_mula_avx2(vec3);
			vec1 = _mm256_add_epi64(_mm256_unpacklo_epi64(vec0, vec1), _mm256_unpackhi_epi64(vec0, vec1));
			vec3 = _mm256_add_epi64(_mm256_unpacklo_epi64(vec2, vec3), _mm256_unpackhi_epi64(vec2, vec3));
			vec0 = _mm256_permute2x128_si256(vec1, vec3, 0x20);
			vec2 = _mm256_permute2x128_si256(vec1, vec3, 0x31);
			vec0 = _mm256_shuffle_epi32(_mm256_add_epi64(vec0, vec2), 0x88);
			vec0 = _mm256_permute4x64_epi64(vec0, 0x08);

			// "distPtr" cannot be aligned when multithreading is enabled ("&distPtr[counts * i]")
			_mm_storeu_si128(reinterpret_cast<__m128i*>(&distPtr[j]), _mm256_castsi256_si128(vec0)); // SSE/AVX transition issue if code not built with AVX enabled

			dataPtr += strideTimes4;
		}
	}

	for (; j < height; j += 1) {
		vec0 = _mm256_xor_si256(_mm256_load_si256(reinterpret_cast<const __m256i*>(&dataPtr[0])), vecPatch);
		vec0n = _mm256_castsi256_si128(vec0);
		vec1n = _mm256_extracti128_si256(vec0, 0x1);

		// SSE/AVX transition issue if code not built with AVX enabled
#if COMPV_ARCH_X64
		cnt = compv_popcnt64(static_cast<uint64_t>(_mm_cvtsi128_si64(vec0n)));
		cnt += compv_popcnt64(static_cast<uint64_t>(_mm_extract_epi64(vec0n, 0x1)));
		cnt += compv_popcnt64(static_cast<uint64_t>(_mm_cvtsi128_si64(vec1n)));
		cnt += compv_popcnt64(static_cast<uint64_t>(_mm_extract_epi64(vec1n, 0x1)));
#else
		cnt = compv_popcnt32(static_cast<uint32_t>(_mm_cvtsi128_si32(vec0n)));
		cnt += compv_popcnt32(static_cast<uint32_t>(_mm_extract_epi32(vec0n, 0x1)));
		cnt += compv_popcnt32(static_cast<uint32_t>(_mm_extract_epi32(vec0n, 0x2)));
		cnt += compv_popcnt32(static_cast<uint32_t>(_mm_extract_epi32(vec0n, 0x3)));
		cnt += compv_popcnt32(static_cast<uint32_t>(_mm_cvtsi128_si32(vec1n)));
		cnt += compv_popcnt32(static_cast<uint32_t>(_mm_extract_epi32(vec1n, 0x1)));
		cnt += compv_popcnt32(static_cast<uint32_t>(_mm_extract_epi32(vec1n, 0x2)));
		cnt += compv_popcnt32(static_cast<uint32_t>(_mm_extract_epi32(vec1n, 0x3)));
#endif

		distPtr[j] = static_cast<int32_t>(cnt);

		dataPtr += stride;
	}

	_mm256_zeroupper();
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
