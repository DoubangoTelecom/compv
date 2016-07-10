/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/intrinsics/x86/math/compv_math_convlt_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/compv_simd_globals.h"
#include "compv/math/compv_math.h"
#include "compv/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// Also works with "uint16"
// TODO(dmi): add support for ASM
void MathConvlt1VertHz_8u16i16i_Intrin_SSE2(const uint8_t* inPtr, int16_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride, compv_uscalar_t pad, const int16_t* vhkernPtr, compv_uscalar_t kernSize)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // AVX
	COMPV_DEBUG_INFO_CHECK_SSE2();
	compv_scalar_t i, width_ = static_cast<compv_scalar_t>(width);
	compv_uscalar_t j, k, m;
	__m128i xmmI0, xmmS0, xmmS1, xmmCoeff;
	int16_t sum;
	const __m128i xmmZero = _mm_setzero_si128();

	for (j = 0; j < height; ++j) {
		for (i = 0; i < width_ - 15; i += 16) {
			xmmS0 = _mm_setzero_si128();
			xmmS1 = _mm_setzero_si128();
			for (k = 0, m = 0; k < kernSize; ++k, m += stride) {
				xmmI0 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&inPtr[m]));
				xmmCoeff = _mm_set1_epi16(vhkernPtr[k]);
				xmmS0 = _mm_add_epi16(xmmS0, _mm_mullo_epi16(_mm_unpacklo_epi8(xmmI0, xmmZero), xmmCoeff));
				xmmS1 = _mm_add_epi16(xmmS1, _mm_mullo_epi16(_mm_unpackhi_epi8(xmmI0, xmmZero), xmmCoeff));
			}
			_mm_storeu_si128(reinterpret_cast<__m128i*>(outPtr), xmmS0);
			_mm_storeu_si128(reinterpret_cast<__m128i*>(outPtr + 8), xmmS1);
			inPtr += 16;
			outPtr += 16;
		}
		if (i < width_ - 7) {
			xmmS0 = _mm_setzero_si128();
			for (k = 0, m = 0; k < kernSize; ++k, m += stride) {
				xmmI0 = _mm_cvtsi64_si128(*reinterpret_cast<const uint64_t*>(&inPtr[m]));
				xmmCoeff = _mm_set1_epi16(vhkernPtr[k]);
				xmmS0 = _mm_add_epi16(xmmS0, _mm_mullo_epi16(_mm_unpacklo_epi8(xmmI0, xmmZero), xmmCoeff));
			}
			_mm_storeu_si128(reinterpret_cast<__m128i*>(outPtr), xmmS0);
			i += 8;
			inPtr += 8;
			outPtr += 8;
		}
		if (i < width_ - 3) {
			xmmS0 = _mm_setzero_si128();
			for (k = 0, m = 0; k < kernSize; ++k, m += stride) {
				xmmI0 = _mm_cvtsi32_si128(*reinterpret_cast<const uint32_t*>(&inPtr[m]));
				xmmCoeff = _mm_set1_epi16(vhkernPtr[k]);
				xmmS0 = _mm_add_epi16(xmmS0, _mm_mullo_epi16(_mm_unpacklo_epi8(xmmI0, xmmZero), xmmCoeff));
			}
			*reinterpret_cast<uint64_t*>(outPtr) = static_cast<uint64_t>(_mm_cvtsi128_si64(xmmS0));
			i += 4;
			inPtr += 4;
			outPtr += 4;
		}
		for (; i < width_; i += 1) {
			sum = 0;
			for (k = 0; k < kernSize; ++k) {
				sum += inPtr[k * stride] * vhkernPtr[k];
			}
			*reinterpret_cast<int16_t*>(outPtr) = sum;
			inPtr += 1;
			outPtr += 1;
		}

		inPtr += pad;
		outPtr += pad;
	}
}

// TODO(dmi): add support for ASM
void MathConvlt1VertHz_16i16i16i_Intrin_SSE2(const int16_t* inPtr, int16_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride, compv_uscalar_t pad, const int16_t* vhkernPtr, compv_uscalar_t kernSize)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // AVX
	COMPV_DEBUG_INFO_CHECK_SSE2();
	compv_scalar_t i, width_ = static_cast<compv_scalar_t>(width);
	compv_uscalar_t j, k, m;
	__m128i xmmI0, xmmI1, xmmS0, xmmS1, xmmCoeff;
	int16_t sum;

	for (j = 0; j < height; ++j) {
		for (i = 0; i < width_ - 15; i += 16) {
			xmmS0 = _mm_setzero_si128();
			xmmS1 = _mm_setzero_si128();
			for (k = 0, m = 0; k < kernSize; ++k, m += stride) {
				xmmI0 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&inPtr[m]));
				xmmI1 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&inPtr[m + 8]));
				xmmCoeff = _mm_set1_epi16(vhkernPtr[k]);
				xmmS0 = _mm_add_epi16(xmmS0, _mm_mullo_epi16(xmmI0, xmmCoeff));
				xmmS1 = _mm_add_epi16(xmmS1, _mm_mullo_epi16(xmmI1, xmmCoeff));
			}
			_mm_storeu_si128(reinterpret_cast<__m128i*>(outPtr), xmmS0);
			_mm_storeu_si128(reinterpret_cast<__m128i*>(outPtr + 8), xmmS1);
			inPtr += 16;
			outPtr += 16;
		}
		if (i < width_ - 7) {
			xmmS0 = _mm_setzero_si128();
			for (k = 0, m = 0; k < kernSize; ++k, m += stride) {
				xmmI0 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&inPtr[m]));
				xmmCoeff = _mm_set1_epi16(vhkernPtr[k]);
				xmmS0 = _mm_add_epi16(xmmS0, _mm_mullo_epi16(xmmI0, xmmCoeff));
			}
			_mm_storeu_si128(reinterpret_cast<__m128i*>(outPtr), xmmS0);
			i += 8;
			inPtr += 8;
			outPtr += 8;
		}
		if (i < width_ - 3) {
			xmmS0 = _mm_setzero_si128();
			for (k = 0, m = 0; k < kernSize; ++k, m += stride) {
				xmmI0 = _mm_cvtsi64_si128(*reinterpret_cast<const uint64_t*>(&inPtr[m]));
				xmmCoeff = _mm_set1_epi16(vhkernPtr[k]);
				xmmS0 = _mm_add_epi16(xmmS0, _mm_mullo_epi16(xmmI0, xmmCoeff));
			}
			*reinterpret_cast<uint64_t*>(outPtr) = static_cast<uint64_t>(_mm_cvtsi128_si64(xmmS0));
			i += 4;
			inPtr += 4;
			outPtr += 4;
		}
		for (; i < width_; i += 1) {
			sum = 0;
			for (k = 0; k < kernSize; ++k) {
				sum += inPtr[k * stride] * vhkernPtr[k];
			}
			*reinterpret_cast<int16_t*>(outPtr) = sum;
			inPtr += 1;
			outPtr += 1;
		}

		inPtr += pad;
		outPtr += pad;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */