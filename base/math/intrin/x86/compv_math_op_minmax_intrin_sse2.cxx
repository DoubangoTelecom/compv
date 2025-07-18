/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/x86/compv_math_op_minmax_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

void CompVMathOpMinMax_32f_Intrin_SSE2(COMPV_ALIGNED(SSE) const compv_float32_t* APtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride, compv_float32_t* min1, compv_float32_t* max1)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();

	const compv_uscalar_t width16 = width & -16;
	const compv_uscalar_t width4 = width & -4;
	__m128 vv0_minn = _mm_load1_ps(min1), vv1_minn = vv0_minn, vv2_minn = vv0_minn, vv3_minn = vv0_minn;
	__m128 vv0_maxx = _mm_load1_ps(max1), vv1_maxx = vv0_maxx, vv2_maxx = vv0_maxx, vv3_maxx = vv0_maxx;
	for (compv_uscalar_t j = 0; j < height; ++j) {
		compv_uscalar_t i = 0;
		for (; i < width16; i += 16) {
			const __m128 vv0 = _mm_load_ps(&APtr[i]);
			const __m128 vv1 = _mm_load_ps(&APtr[i + 4]);
			const __m128 vv2 = _mm_load_ps(&APtr[i + 8]);
			const __m128 vv3 = _mm_load_ps(&APtr[i + 12]);
			vv0_minn = _mm_min_ps(vv0_minn, vv0);
			vv1_minn = _mm_min_ps(vv1_minn, vv1);
			vv2_minn = _mm_min_ps(vv2_minn, vv2);
			vv3_minn = _mm_min_ps(vv3_minn, vv3);
			vv0_maxx = _mm_max_ps(vv0_maxx, vv0);
			vv1_maxx = _mm_max_ps(vv1_maxx, vv1);
			vv2_maxx = _mm_max_ps(vv2_maxx, vv2);
			vv3_maxx = _mm_max_ps(vv3_maxx, vv3);
		}
		for (; i < width4; i += 4) {
			const __m128 vv0 = _mm_load_ps(&APtr[i]);
			vv0_minn = _mm_min_ps(vv0_minn, vv0);
			vv0_maxx = _mm_max_ps(vv0_maxx, vv0);
		}

		for (; i < width; ++i) {
			const __m128 vv0 = _mm_load_ss(&APtr[i]);
			vv0_minn = _mm_min_ss(vv0_minn, vv0);
			vv0_maxx = _mm_max_ss(vv0_maxx, vv0);
		}

		APtr += stride;
	}

	vv0_minn = _mm_min_ps(vv0_minn, vv1_minn);
	vv2_minn = _mm_min_ps(vv2_minn, vv3_minn);
	vv0_minn = _mm_min_ps(vv0_minn, vv2_minn);
	vv0_minn = _mm_min_ps(vv0_minn, _mm_shuffle_ps(vv0_minn, vv0_minn, 0xE));
	vv0_minn = _mm_min_ps(vv0_minn, _mm_shuffle_ps(vv0_minn, vv0_minn, 0x1));
	*min1 = _mm_cvtss_f32(vv0_minn);

	vv0_maxx = _mm_max_ps(vv0_maxx, vv1_maxx);
	vv2_maxx = _mm_max_ps(vv2_maxx, vv3_maxx);
	vv0_maxx = _mm_max_ps(vv0_maxx, vv2_maxx);
	vv0_maxx = _mm_max_ps(vv0_maxx, _mm_shuffle_ps(vv0_maxx, vv0_maxx, 0xE));
	vv0_maxx = _mm_max_ps(vv0_maxx, _mm_shuffle_ps(vv0_maxx, vv0_maxx, 0x1));
	*max1 = _mm_cvtss_f32(vv0_maxx);
}

void CompVMathOpMin_8u_Intrin_SSE2(COMPV_ALIGNED(SSE) const uint8_t* APtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride, uint8_t* min1)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();

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
	vec0 = _mm_min_epu8(vec0, _mm_shuffle_epi32(vec0, 0x1));

	const uint32_t min32 = (uint32_t)_mm_cvtsi128_si32(vec0);
	const uint8_t a = (uint8_t)(min32 & 0xff);
	const uint8_t b = (uint8_t)((min32 >> 8) & 0xff);
	const uint8_t c = (uint8_t)((min32 >> 16) & 0xff);
	const uint8_t d = (uint8_t)((min32 >> 24) & 0xff);
	minn = COMPV_MATH_MIN_3(minn, a, b);
	minn = COMPV_MATH_MIN_3(minn, c, d);
}

void CompVMathOpMax_32f_Intrin_SSE2(COMPV_ALIGNED(SSE) const compv_float32_t* APtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride, const compv_float32_t* b1, COMPV_ALIGNED(SSE) compv_float32_t* RPtr)
{
	const __m128 v_b1 = _mm_load1_ps(b1);
	const compv_uscalar_t width16 = width & -16;
	const compv_uscalar_t width4 = width & -4;
	for (compv_uscalar_t j = 0; j < height; ++j) {
		compv_uscalar_t i = 0;
		for (; i < width16; i += 16) {
			_mm_store_ps(&RPtr[i], _mm_max_ps(v_b1, _mm_load_ps(&APtr[i])));
			_mm_store_ps(&RPtr[i + 4], _mm_max_ps(v_b1, _mm_load_ps(&APtr[i + 4])));
			_mm_store_ps(&RPtr[i + 8], _mm_max_ps(v_b1, _mm_load_ps(&APtr[i + 8])));
			_mm_store_ps(&RPtr[i + 12], _mm_max_ps(v_b1, _mm_load_ps(&APtr[i + 12])));
		}
		for (; i < width4; i += 4) {
			_mm_store_ps(&RPtr[i], _mm_max_ps(v_b1, _mm_load_ps(&APtr[i])));
		}
		APtr += stride;
		RPtr += stride;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
