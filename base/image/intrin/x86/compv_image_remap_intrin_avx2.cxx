/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/intrin/x86/compv_image_remap_intrin_avx2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// TODO(dmi): add ASM code
#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx2
#endif
void CompVImageRemapBilinear_8u32f_Intrin_AVX2(
	COMPV_ALIGNED(AVX) const compv_float32_t* mapXPtr, COMPV_ALIGNED(AVX) const compv_float32_t* mapYPtr,
	const uint8_t* inputPtr, compv_float32_t* outputPtr,
	COMPV_ALIGNED(AVX) const compv_float32_t* roi, COMPV_ALIGNED(AVX) const int32_t* size,
	const compv_float32_t* defaultPixelValue1,
	COMPV_ALIGNED(AVX) const compv_uscalar_t count
)
{
	COMPV_DEBUG_INFO_CHECK_AVX2();

	// TODO(dmi): No ASM code

	_mm256_zeroupper();

	const __m256 roi_left = _mm256_set1_ps(roi[0]); // 32f
	const __m256 roi_right = _mm256_set1_ps(roi[1]); // 32f
	const __m256 roi_top = _mm256_set1_ps(roi[2]); // 32f
	const __m256 roi_bottom = _mm256_set1_ps(roi[3]); // 32f

	const __m256 one = _mm256_set1_ps(1.f);

	const __m256i inWidthMinus1 = _mm256_set1_epi32(size[0]); // 32s
	const __m256i inHeightMinus1 = _mm256_set1_epi32(size[1]); // 32s
	const __m256i stride = _mm256_set1_epi32(size[2]); // 32s
	const __m256i maxIndex = _mm256_set1_epi32((size[2] * (size[1] + 1)) - 1); // (stride * inHeight) - 1

	const __m256i zero = _mm256_setzero_si256();
	const __m256i clearbit = _mm256_set1_epi32(0x000000ff); // 32s

	const __m256 defaultPixelValue = _mm256_set1_ps(*defaultPixelValue1); // 32f

	for (compv_uscalar_t i = 0; i < count; i += 8) {
		const __m256 x = _mm256_load_ps(&mapXPtr[i]);
		const __m256 y = _mm256_load_ps(&mapYPtr[i]);

		const __m256 cmp = _mm256_and_ps(
			_mm256_and_ps(_mm256_cmp_ps(x, roi_left, _CMP_GE_OQ), _mm256_cmp_ps(x, roi_right, _CMP_LE_OQ)),
			_mm256_and_ps(_mm256_cmp_ps(y, roi_top, _CMP_GE_OQ), _mm256_cmp_ps(y, roi_bottom, _CMP_LE_OQ))
		);
		const int mask = _mm256_movemask_epi8(_mm256_castps_si256(cmp));

		if (mask) {
			const __m256i x1 = _mm256_cvttps_epi32(x); // truncation to emulate "static_cast<int32_t>()"
			const __m256i x2 = _mm256_min_epi32(_mm256_cvttps_epi32(_mm256_add_ps(x, one)), inWidthMinus1); // TODO(dmi): use "_mm256_cvtps_epi32" ?
			const __m256 xfractpart = _mm256_sub_ps(x, _mm256_cvtepi32_ps(x1));
			__m256i y1 = _mm256_cvttps_epi32(y); // truncation to emulate "static_cast<int32_t>()"
			__m256i y2 = _mm256_min_epi32(_mm256_cvttps_epi32(_mm256_add_ps(y, one)), inHeightMinus1); // TODO(dmi): use "_mm256_cvtps_epi32" ?
			const __m256 yfractpart = _mm256_sub_ps(y, _mm256_cvtepi32_ps(y1));
			const __m256 xyfractpart = _mm256_mul_ps(xfractpart, yfractpart);
			y1 = _mm256_mullo_epi32(y1, stride);
			y2 = _mm256_mullo_epi32(y2, stride);
			const __m256i y1x1 = _mm256_max_epi32(_mm256_min_epi32(maxIndex, _mm256_add_epi32(y1, x1)), zero); // clip to avoid reading beyong accessible memory address. "mask" is useless here.
			const __m256i y1x2 = _mm256_max_epi32(_mm256_min_epi32(maxIndex, _mm256_add_epi32(y1, x2)), zero);
			const __m256i y2x1 = _mm256_max_epi32(_mm256_min_epi32(maxIndex, _mm256_add_epi32(y2, x1)), zero);
			const __m256i y2x2 = _mm256_max_epi32(_mm256_min_epi32(maxIndex, _mm256_add_epi32(y2, x2)), zero);
			const __m256 y1x1_vec = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_i32gather_epi32(reinterpret_cast<int const*>(inputPtr), y1x1, 1), clearbit)); // 1 = sizeof(uint8_t)
			const __m256 y1x2_vec = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_i32gather_epi32(reinterpret_cast<int const*>(inputPtr), y1x2, 1), clearbit));
			const __m256 y2x1_vec = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_i32gather_epi32(reinterpret_cast<int const*>(inputPtr), y2x1, 1), clearbit));
			const __m256 y2x2_vec = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_i32gather_epi32(reinterpret_cast<int const*>(inputPtr), y2x2, 1), clearbit));

			// "A = (1 - xfractpart - yfractpart + xyfractpart)"
			const __m256 A = _mm256_add_ps(_mm256_sub_ps(_mm256_sub_ps(one, xfractpart), yfractpart), xyfractpart);
			// "B = (xfractpart - xyfractpart)"
			const __m256 B = _mm256_sub_ps(xfractpart, xyfractpart);
			// "C = (yfractpart - xyfractpart)"
			const __m256 C = _mm256_sub_ps(yfractpart, xyfractpart);

			__m256 pixel = _mm256_add_ps(_mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(y1x1_vec, A), _mm256_mul_ps(y1x2_vec, B)), _mm256_mul_ps(y2x1_vec, C)), _mm256_mul_ps(y2x2_vec, xyfractpart));
			pixel = _mm256_or_ps(_mm256_and_ps(pixel, cmp), _mm256_andnot_ps(cmp, defaultPixelValue));

			_mm256_storeu_ps(&outputPtr[i], pixel);
		}
		else {
			_mm256_storeu_ps(&outputPtr[i], defaultPixelValue);
		}
	}

	_mm256_zeroupper();
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
