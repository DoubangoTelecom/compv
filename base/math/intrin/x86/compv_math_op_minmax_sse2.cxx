/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/x86/compv_math_op_minmax_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

void CompVMathOpMin_8u_Intrin_SSE2(COMPV_ALIGNED(SSE) const uint8_t* APtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride, uint8_t* min1)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Please add ASM implementation");

	uint8_t& minn = *min1;
	__m128i vec0 = _mm_set1_epi8((char)minn);
	__m128i vec1 = vec0;
	__m128i vec2 = vec0;
	__m128i vec3 = vec0;
	const compv_uscalar_t width64 = width & -64;
	const compv_uscalar_t width16 = width & -16;
	for (compv_uscalar_t j = 0; j < height; ++j) {
		compv_uscalar_t i = 0;
		for (; i < width64; i += 64) {
			vec0 = _mm_min_epu8(vec0, _mm_load_si128(reinterpret_cast<const __m128i*>(&APtr[i])));
			vec1 = _mm_min_epu8(vec1, _mm_load_si128(reinterpret_cast<const __m128i*>(&APtr[i + 16])));
			vec2 = _mm_min_epu8(vec2, _mm_load_si128(reinterpret_cast<const __m128i*>(&APtr[i + 32])));
			vec3 = _mm_min_epu8(vec3, _mm_load_si128(reinterpret_cast<const __m128i*>(&APtr[i + 48])));
		}
		for (; i < width16; i += 16) {
			vec0 = _mm_min_epu8(vec0, _mm_load_si128(reinterpret_cast<const __m128i*>(&APtr[i])));
		}
		for (; i < width; i += 1) {
			const uint8_t& vv = APtr[i];
			minn = COMPV_MATH_MIN(minn, vv);
		}
		APtr += stride;
	}
	vec0 = _mm_min_epu8(vec0, vec1);
	vec2 = _mm_min_epu8(vec2, vec3);
	vec0 = _mm_min_epu8(vec0, vec2);
	vec0 = _mm_min_epu8(vec0, _mm_shuffle_epi32(vec0, 0xE));
	const uint32_t min32 = (uint32_t)_mm_cvtsi128_si32(vec0);
	const uint8_t a = (uint8_t)(min32 & 0xff);
	const uint8_t b = (uint8_t)((min32 >> 8) & 0xff);
	const uint8_t c = (uint8_t)((min32 >> 16) & 0xff);
	const uint8_t d = (uint8_t)((min32 >> 24) & 0xff);
	minn = COMPV_MATH_MIN_3(minn, a, b);
	minn = COMPV_MATH_MIN_3(minn, c, d);
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
