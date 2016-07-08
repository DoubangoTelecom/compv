/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/intrinsics/x86/math/compv_math_utils_intrin_ssse3.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/compv_simd_globals.h"
#include "compv/math/compv_math.h"
#include "compv/compv_mem.h"
#include "compv/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// "strideInBytes" must be SSE-aligned
void MathUtilsAddAbs_16i16u_Intrin_SSSE3(const COMPV_ALIGNED(SSE) int16_t* a, const COMPV_ALIGNED(SSE) int16_t* b, COMPV_ALIGNED(SSE) uint16_t* r, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // TODO(dmi): add ASM SSSE3 version (use "pabsw")
	COMPV_DEBUG_INFO_CHECK_SSSE3();
	compv_uscalar_t i, j;

	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 8) {
			_mm_store_si128(reinterpret_cast<__m128i*>(r + i), _mm_adds_epu16(_mm_abs_epi16(_mm_load_si128(reinterpret_cast<const __m128i*>(a + i))), _mm_abs_epi16(_mm_load_si128(reinterpret_cast<const __m128i*>(b + i)))));
		}
		r += stride;
		a += stride;
		b += stride;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
