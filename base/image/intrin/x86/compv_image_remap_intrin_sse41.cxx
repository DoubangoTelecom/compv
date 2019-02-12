/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/intrin/x86/compv_image_remap_intrin_sse41.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// TODO(dmi): add ASM code
// "_mm_mullo_epi32" is SSE4.1
void CompVImageRemapBilinear_8u32f_Intrin_SSE41(
	COMPV_ALIGNED(SSE) const compv_float32_t* mapXPtr, COMPV_ALIGNED(SSE) const compv_float32_t* mapYPtr,
	const uint8_t* inputPtr, compv_float32_t* outputPtr,
	COMPV_ALIGNED(SSE) const compv_float32_t* roi, COMPV_ALIGNED(SSE) const int32_t* size,
	const compv_float32_t* defaultPixelValue1,
	COMPV_ALIGNED(SSE) const compv_uscalar_t count
)
{
	COMPV_DEBUG_INFO_CHECK_SSE41();

	const __m128 roi_left = _mm_set1_ps(roi[0]); // 32f
	const __m128 roi_right = _mm_set1_ps(roi[1]); // 32f
	const __m128 roi_top = _mm_set1_ps(roi[2]); // 32f
	const __m128 roi_bottom = _mm_set1_ps(roi[3]); // 32f

	const __m128 one = _mm_set1_ps(1.f);

	const __m128i inWidthMinus1 = _mm_set1_epi32(size[0]); // 32s
	const __m128i inHeightMinus1 = _mm_set1_epi32(size[1]); // 32s
	const __m128i stride = _mm_set1_epi32(size[2]); // 32s
	const __m128i maxIndex = _mm_set1_epi32((size[2] * (size[1] + 1)) - 1); // (stride * inHeight) - 1

	const __m128i zero = _mm_setzero_si128();

	const __m128 defaultPixelValue = _mm_set1_ps(*defaultPixelValue1); // 32f

	// TODO(dmi): AVX, not needed. Use Gather instruction.
	COMPV_ALIGN_SSE() int32_t y1x1_mem[4];
	COMPV_ALIGN_SSE() int32_t y1x2_mem[4];
	COMPV_ALIGN_SSE() int32_t y2x1_mem[4];
	COMPV_ALIGN_SSE() int32_t y2x2_mem[4];

	for (compv_uscalar_t i = 0; i < count; i += 4) {
		const __m128 x = _mm_load_ps(&mapXPtr[i]);
		const __m128 y = _mm_load_ps(&mapYPtr[i]);

		const __m128 cmp = _mm_and_ps(
			_mm_and_ps(_mm_cmpge_ps(x, roi_left), _mm_cmple_ps(x, roi_right)),
			_mm_and_ps(_mm_cmpge_ps(y, roi_top), _mm_cmple_ps(y, roi_bottom))
		);
		const int mask = _mm_movemask_epi8(_mm_castps_si128(cmp));

		if (mask) {
			const __m128i x1 = _mm_cvttps_epi32(x); // truncation to emulate "static_cast<int32_t>()"
			const __m128i x2 = _mm_min_epi32(_mm_cvttps_epi32(_mm_add_ps(x, one)), inWidthMinus1); // TODO(dmi): use "_mm_cvtps_epi32" ?
			const __m128 xfractpart = _mm_sub_ps(x, _mm_cvtepi32_ps(x1));
			__m128i y1 = _mm_cvttps_epi32(y); // truncation to emulate "static_cast<int32_t>()"
			__m128i y2 = _mm_min_epi32(_mm_cvttps_epi32(_mm_add_ps(y, one)), inHeightMinus1); // TODO(dmi): use "_mm_cvtps_epi32" ?
			const __m128 yfractpart = _mm_sub_ps(y, _mm_cvtepi32_ps(y1));
			const __m128 xyfractpart = _mm_mul_ps(xfractpart, yfractpart);
			y1 = _mm_mullo_epi32(y1, stride);
			y2 = _mm_mullo_epi32(y2, stride);
			const __m128i y1x1 = _mm_max_epi32(_mm_min_epi32(maxIndex, _mm_add_epi32(y1, x1)), zero); // clip to avoid reading beyong accessible memory address. "mask" is useless here.
			const __m128i y1x2 = _mm_max_epi32(_mm_min_epi32(maxIndex, _mm_add_epi32(y1, x2)), zero);
			const __m128i y2x1 = _mm_max_epi32(_mm_min_epi32(maxIndex, _mm_add_epi32(y2, x1)), zero);
			const __m128i y2x2 = _mm_max_epi32(_mm_min_epi32(maxIndex, _mm_add_epi32(y2, x2)), zero);
			_mm_store_si128(reinterpret_cast<__m128i*>(y1x1_mem), y1x1);
			_mm_store_si128(reinterpret_cast<__m128i*>(y1x2_mem), y1x2);
			_mm_store_si128(reinterpret_cast<__m128i*>(y2x1_mem), y2x1);
			_mm_store_si128(reinterpret_cast<__m128i*>(y2x2_mem), y2x2);
			// "A = (1 - xfractpart - yfractpart + xyfractpart)"
			const __m128 A = _mm_add_ps(_mm_sub_ps(_mm_sub_ps(one, xfractpart), yfractpart), xyfractpart);
			// "B = (xfractpart - xyfractpart)"
			const __m128 B = _mm_sub_ps(xfractpart, xyfractpart);
			// "C = (yfractpart - xyfractpart)"
			const __m128 C = _mm_sub_ps(yfractpart, xyfractpart);

			// TODO(dmi): AVX, use Gather instruction.
			y1x1_mem[0] = inputPtr[y1x1_mem[0]];
			y1x1_mem[1] = inputPtr[y1x1_mem[1]];
			y1x1_mem[2] = inputPtr[y1x1_mem[2]];
			y1x1_mem[3] = inputPtr[y1x1_mem[3]];
			y1x2_mem[0] = inputPtr[y1x2_mem[0]];
			y1x2_mem[1] = inputPtr[y1x2_mem[1]];
			y1x2_mem[2] = inputPtr[y1x2_mem[2]];
			y1x2_mem[3] = inputPtr[y1x2_mem[3]];
			y2x1_mem[0] = inputPtr[y2x1_mem[0]];
			y2x1_mem[1] = inputPtr[y2x1_mem[1]];
			y2x1_mem[2] = inputPtr[y2x1_mem[2]];
			y2x1_mem[3] = inputPtr[y2x1_mem[3]];
			y2x2_mem[0] = inputPtr[y2x2_mem[0]];
			y2x2_mem[1] = inputPtr[y2x2_mem[1]];
			y2x2_mem[2] = inputPtr[y2x2_mem[2]];
			y2x2_mem[3] = inputPtr[y2x2_mem[3]];
			
			const __m128 y1x1_vec = _mm_cvtepi32_ps(*reinterpret_cast<const __m128i*>(y1x1_mem));
			const __m128 y1x2_vec = _mm_cvtepi32_ps(*reinterpret_cast<const __m128i*>(y1x2_mem));
			const __m128 y2x1_vec = _mm_cvtepi32_ps(*reinterpret_cast<const __m128i*>(y2x1_mem));
			const __m128 y2x2_vec = _mm_cvtepi32_ps(*reinterpret_cast<const __m128i*>(y2x2_mem));
			
			__m128 pixel = _mm_add_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(y1x1_vec, A), _mm_mul_ps(y1x2_vec, B)), _mm_mul_ps(y2x1_vec, C)), _mm_mul_ps(y2x2_vec, xyfractpart));
			pixel = _mm_or_ps(_mm_and_ps(pixel, cmp), _mm_andnot_ps(cmp, defaultPixelValue));
			
			_mm_storeu_ps(&outputPtr[i], pixel);
		}
		else {
			_mm_storeu_ps(&outputPtr[i], defaultPixelValue);
		}
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
