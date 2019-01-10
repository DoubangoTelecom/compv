/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/x86/compv_math_distance_intrin_sse42.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_debug.h"
#include "compv/base/compv_bits.h"

COMPV_NAMESPACE_BEGIN()

// popcnt available starting SSE4.2 but up to the caller to check its availability using CPU features
void CompVMathDistanceHamming_Intrin_POPCNT_SSE42(COMPV_ALIGNED(SSE) const uint8_t* dataPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride, COMPV_ALIGNED(SSE) const uint8_t* patch1xnPtr, int32_t* distPtr)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("ASM code almost #2 times faster (VS2013)"); 
	COMPV_DEBUG_INFO_CHECK_SSE42();

	compv_uscalar_t i, j;
#if COMPV_ARCH_X64
	uint64_t cnt;
	uint64_t pop;
#else
	uint32_t cnt;
	uint32_t pop;
#endif
	__m128i vec0, vec1;

	for (j = 0; j < height; ++j) {
		cnt = 0;

		/* Loop16 */
		for (i = 0; i < width - 15; i += 16) {
			vec0 = _mm_load_si128(reinterpret_cast<const __m128i*>(&dataPtr[i]));
			vec1 = _mm_load_si128(reinterpret_cast<const __m128i*>(&patch1xnPtr[i]));
			vec0 = _mm_xor_si128(vec0, vec1);
#if COMPV_ARCH_X64
			cnt += compv_popcnt64(static_cast<uint64_t>(_mm_cvtsi128_si64(vec0)));
			cnt += compv_popcnt64(static_cast<uint64_t>(_mm_extract_epi64(vec0, 1)));
#else
			cnt += compv_popcnt32(static_cast<uint32_t>(_mm_cvtsi128_si32(vec0)));
			cnt += compv_popcnt32(static_cast<uint32_t>(_mm_extract_epi32(vec0, 1)));
			cnt += compv_popcnt32(static_cast<uint32_t>(_mm_extract_epi32(vec0, 2)));
			cnt += compv_popcnt32(static_cast<uint32_t>(_mm_extract_epi32(vec0, 3)));
#endif
		}

		/* Loop8 */
#if COMPV_ARCH_X64
		if (i < width - 7) {
			pop = *reinterpret_cast<const uint64_t*>(&dataPtr[i]) ^ *reinterpret_cast<const uint64_t*>(&patch1xnPtr[i]);
			cnt += compv_popcnt64(pop);
			i += 8;
		}
#else
		if (i < width - 3) { // fist time
			pop = *reinterpret_cast<const uint32_t*>(&dataPtr[i]) ^ *reinterpret_cast<const uint32_t*>(&patch1xnPtr[i]);
			cnt += compv_popcnt32(static_cast<uint32_t>(pop));
			i += 4;
		}
		if (i < width - 3) { // second time
			pop = *reinterpret_cast<const uint32_t*>(&dataPtr[i]) ^ *reinterpret_cast<const uint32_t*>(&patch1xnPtr[i]);
			cnt += compv_popcnt32(static_cast<uint32_t>(pop));
			i += 4;
		}
#endif

		/* Loop4 */
		if (i < width - 3) { // second time
			pop = *reinterpret_cast<const uint32_t*>(&dataPtr[i]) ^ *reinterpret_cast<const uint32_t*>(&patch1xnPtr[i]);
			cnt += compv_popcnt32(static_cast<uint32_t>(pop));
			i += 4;
		}

		/* Loop2 */
		if (i < width - 1) {
			pop = *reinterpret_cast<const uint16_t*>(&dataPtr[i]) ^ *reinterpret_cast<const uint16_t*>(&patch1xnPtr[i]);
			cnt += compv_popcnt16(static_cast<uint16_t>(pop));
			i += 2;
		}

		/* Loop1 */
		if (i < width) {
			pop = *reinterpret_cast<const uint8_t*>(&dataPtr[i]) ^ *reinterpret_cast<const uint8_t*>(&patch1xnPtr[i]);
			cnt += compv_popcnt16(static_cast<uint16_t>(pop));
		}

		dataPtr += stride;
		distPtr[j] = static_cast<int32_t>(cnt);
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
