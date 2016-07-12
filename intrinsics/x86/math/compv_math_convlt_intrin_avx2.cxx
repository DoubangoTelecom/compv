/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/intrinsics/x86/math/compv_math_convlt_intrin_avx2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/intrinsics/x86/compv_intrin_avx.h"
#include "compv/compv_simd_globals.h"
#include "compv/math/compv_math.h"
#include "compv/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// TODO(dmi): Add support for ASM
// Also works with "uint16"
#if defined __INTEL_COMPILER
#	pragma intel optimization_parameter target_arch=avx
#endif
void MathConvlt1VertHz_8u16i16i_Intrin_AVX2(const uint8_t* inPtr, int16_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride, compv_uscalar_t pad, const int16_t* vhkernPtr, compv_uscalar_t kernSize)
{
	COMPV_DEBUG_INFO_CHECK_AVX();
	COMPV_DEBUG_INFO_CHECK_AVX2();
	_mm256_zeroupper();
	compv_scalar_t i, width_ = static_cast<compv_scalar_t>(width);
	compv_uscalar_t j, k, m;
	__m256i ymmI0, ymmS0, ymmS1, ymmCoeff;
	int16_t sum;
	const __m256i ymmZero = _mm256_setzero_si256();
	const __m256i ymmCoeff0 = _mm256_set1_epi16(vhkernPtr[0]);
	static const int shuffle = COMPV_MM_SHUFFLE(3, 1, 2, 0);

	for (j = 0; j < height; ++j) {
		for (i = 0; i < width_ - 31; i += 32) {
			ymmI0 = _mm256_permute4x64_epi64(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(&inPtr[0])), shuffle);
			ymmS0 = _mm256_mullo_epi16(_mm256_unpacklo_epi8(ymmI0, ymmZero), ymmCoeff0);
			ymmS1 = _mm256_mullo_epi16(_mm256_unpackhi_epi8(ymmI0, ymmZero), ymmCoeff0);
			for (k = 1, m = stride; k < kernSize; ++k, m += stride) {
				ymmCoeff = _mm256_set1_epi16(vhkernPtr[k]);
				ymmI0 = _mm256_permute4x64_epi64(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(&inPtr[m])), shuffle);
				ymmS0 = _mm256_add_epi16(ymmS0, _mm256_mullo_epi16(_mm256_unpacklo_epi8(ymmI0, ymmZero), ymmCoeff));
				ymmS1 = _mm256_add_epi16(ymmS1, _mm256_mullo_epi16(_mm256_unpackhi_epi8(ymmI0, ymmZero), ymmCoeff));
			}
			_mm256_storeu_si256(reinterpret_cast<__m256i*>(outPtr), ymmS0);
			_mm256_storeu_si256(reinterpret_cast<__m256i*>(outPtr + 16), ymmS1);
			inPtr += 32;
			outPtr += 32;
		}
		if (i < width_ - 15) {
			ymmI0 = _mm256_permute4x64_epi64(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(&inPtr[0])), shuffle);
			ymmS0 = _mm256_mullo_epi16(_mm256_unpacklo_epi8(ymmI0, ymmZero), ymmCoeff0);
			for (k = 1, m = stride; k < kernSize; ++k, m += stride) {
				ymmCoeff = _mm256_set1_epi16(vhkernPtr[k]);
				ymmI0 = _mm256_permute4x64_epi64(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(&inPtr[m])), shuffle);
				ymmS0 = _mm256_add_epi16(ymmS0, _mm256_mullo_epi16(_mm256_unpacklo_epi8(ymmI0, ymmZero), ymmCoeff));
			}
			_mm256_storeu_si256(reinterpret_cast<__m256i*>(outPtr), ymmS0);
			i += 16;
			inPtr += 16;
			outPtr += 16;
		}
		if (i < width_ - 7) {
			const __m256i ymmMaskToExtract128bits = _mm256_load_si256(reinterpret_cast<const __m256i*>(&kAVXMaskstore_0_1_u64));
			ymmI0 = _mm256_permute4x64_epi64(_mm256_maskload_epi64(reinterpret_cast<const int64_t*>(&inPtr[0]), ymmMaskToExtract128bits), shuffle);
			ymmS0 = _mm256_mullo_epi16(_mm256_unpacklo_epi8(ymmI0, ymmZero), ymmCoeff0);
			for (k = 1, m = stride; k < kernSize; ++k, m += stride) {
				ymmCoeff = _mm256_set1_epi16(vhkernPtr[k]);
				ymmI0 = _mm256_permute4x64_epi64(_mm256_maskload_epi64(reinterpret_cast<const int64_t*>(&inPtr[m]), ymmMaskToExtract128bits), shuffle);
				ymmS0 = _mm256_add_epi16(ymmS0, _mm256_mullo_epi16(_mm256_unpacklo_epi8(ymmI0, ymmZero), ymmCoeff));
			}
			_mm256_maskstore_epi64(reinterpret_cast<int64_t*>(outPtr), ymmMaskToExtract128bits, ymmS0);
			i += 8;
			inPtr += 8;
			outPtr += 8;
		}
		if (i < width_ - 3) {
			const __m256i ymmMaskToExtract64bits = _mm256_load_si256(reinterpret_cast<const __m256i*>(&kAVXMaskstore_0_u64));
			ymmI0 = _mm256_permute4x64_epi64(_mm256_maskload_epi64(reinterpret_cast<const int64_t*>(&inPtr[0]), ymmMaskToExtract64bits), shuffle);
			ymmS0 = _mm256_mullo_epi16(_mm256_unpacklo_epi8(ymmI0, ymmZero), ymmCoeff0);
			for (k = 1, m = stride; k < kernSize; ++k, m += stride) {
				ymmCoeff = _mm256_set1_epi16(vhkernPtr[k]);
				ymmI0 = _mm256_permute4x64_epi64(_mm256_maskload_epi64(reinterpret_cast<const int64_t*>(&inPtr[m]), ymmMaskToExtract64bits), shuffle);
				ymmS0 = _mm256_add_epi16(ymmS0, _mm256_mullo_epi16(_mm256_unpacklo_epi8(ymmI0, ymmZero), ymmCoeff));
			}
			_mm256_maskstore_epi64(reinterpret_cast<int64_t*>(outPtr), ymmMaskToExtract64bits, ymmS0);
			i += 4;
			inPtr += 4;
			outPtr += 4;
		}		
		for (; i < width_; i += 1) {
			sum = inPtr[0] * vhkernPtr[0];
			for (k = 1; k < kernSize; ++k) {
				sum += inPtr[k * stride] * vhkernPtr[k];
			}
			*reinterpret_cast<int16_t*>(outPtr) = sum;
			inPtr += 1;
			outPtr += 1;
		}

		inPtr += pad;
		outPtr += pad;
	}
	_mm256_zeroupper();
}

// TODO(dmi): add support for ASM
#if defined __INTEL_COMPILER
#	pragma intel optimization_parameter target_arch=avx
#endif
void MathConvlt1VertHz_16i16i16i_Intrin_AVX2(COMPV_ALIGNED(AVX) const int16_t* inPtr, COMPV_ALIGNED(AVX) int16_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(AVX) compv_uscalar_t stride, compv_uscalar_t pad, const int16_t* vhkernPtr, compv_uscalar_t kernSize)
{
	COMPV_DEBUG_INFO_CHECK_AVX();
	COMPV_DEBUG_INFO_CHECK_AVX2();
	_mm256_zeroupper();

	compv_scalar_t i, width_ = static_cast<compv_scalar_t>(width);
	compv_uscalar_t j, k, m;
	__m256i ymmI0, ymmI1, ymmS0, ymmS1, ymmCoeff;
	int16_t sum;
	const __m256i ymmCoeff0 = _mm256_set1_epi16(vhkernPtr[0]);
	
	for (j = 0; j < height; ++j) {
		for (i = 0; i < width_ - 31; i += 32) {
			ymmS0 = _mm256_mullo_epi16(_mm256_load_si256(reinterpret_cast<const __m256i*>(&inPtr[0])), ymmCoeff0);
			ymmS1 = _mm256_mullo_epi16(_mm256_load_si256(reinterpret_cast<const __m256i*>(&inPtr[16])), ymmCoeff0);
			for (k = 1, m = stride; k < kernSize; ++k, m += stride) {
				ymmCoeff = _mm256_set1_epi16(vhkernPtr[k]);
				ymmI0 = _mm256_load_si256(reinterpret_cast<const __m256i*>(&inPtr[m]));
				ymmI1 = _mm256_load_si256(reinterpret_cast<const __m256i*>(&inPtr[m + 16]));
				ymmS0 = _mm256_add_epi16(ymmS0, _mm256_mullo_epi16(ymmI0, ymmCoeff));
				ymmS1 = _mm256_add_epi16(ymmS1, _mm256_mullo_epi16(ymmI1, ymmCoeff));
			}
			_mm256_store_si256(reinterpret_cast<__m256i*>(outPtr), ymmS0);
			_mm256_store_si256(reinterpret_cast<__m256i*>(outPtr + 16), ymmS1);
			inPtr += 32;
			outPtr += 32;
		}
		if (i < width_ - 15) {
			ymmS0 = _mm256_mullo_epi16(_mm256_load_si256(reinterpret_cast<const __m256i*>(&inPtr[0])), ymmCoeff0);
			for (k = 1, m = stride; k < kernSize; ++k, m += stride) {
				ymmI0 = _mm256_load_si256(reinterpret_cast<const __m256i*>(&inPtr[m]));
				ymmCoeff = _mm256_set1_epi16(vhkernPtr[k]);
				ymmS0 = _mm256_add_epi16(ymmS0, _mm256_mullo_epi16(ymmI0, ymmCoeff));
			}
			_mm256_store_si256(reinterpret_cast<__m256i*>(outPtr), ymmS0);
			i += 16;
			inPtr += 16;
			outPtr += 16;
		}
		if (i < width_ - 7) {
			const __m256i ymmMaskToExtract128bits = _mm256_load_si256(reinterpret_cast<const __m256i*>(&kAVXMaskstore_0_1_u64));
			ymmS0 = _mm256_mullo_epi16(_mm256_maskload_epi64(reinterpret_cast<const int64_t*>(&inPtr[0]), ymmMaskToExtract128bits), ymmCoeff0);
			for (k = 1, m = stride; k < kernSize; ++k, m += stride) {
				ymmI0 = _mm256_maskload_epi64(reinterpret_cast<const int64_t*>(&inPtr[m]), ymmMaskToExtract128bits);
				ymmCoeff = _mm256_set1_epi16(vhkernPtr[k]);
				ymmS0 = _mm256_add_epi16(ymmS0, _mm256_mullo_epi16(ymmI0, ymmCoeff));
			}
			_mm256_maskstore_epi64(reinterpret_cast<int64_t*>(outPtr), ymmMaskToExtract128bits, ymmS0);
			i += 8;
			inPtr += 8;
			outPtr += 8;
		}
		if (i < width_ - 3) {
			const __m256i ymmMaskToExtract64bits = _mm256_load_si256(reinterpret_cast<const __m256i*>(&kAVXMaskstore_0_u64));
			ymmS0 = _mm256_mullo_epi16(_mm256_maskload_epi64(reinterpret_cast<const int64_t*>(&inPtr[0]), ymmMaskToExtract64bits), ymmCoeff0);
			for (k = 1, m = stride; k < kernSize; ++k, m += stride) {
				ymmI0 = _mm256_maskload_epi64(reinterpret_cast<const int64_t*>(&inPtr[m]), ymmMaskToExtract64bits);
				ymmCoeff = _mm256_set1_epi16(vhkernPtr[k]);
				ymmS0 = _mm256_add_epi16(ymmS0, _mm256_mullo_epi16(ymmI0, ymmCoeff));
			}
			_mm256_maskstore_epi64(reinterpret_cast<int64_t*>(outPtr), ymmMaskToExtract64bits, ymmS0);
			i += 4;
			inPtr += 4;
			outPtr += 4;
		}
		for (; i < width_; ++i) {
			sum = inPtr[0] * vhkernPtr[0];
			for (k = 1; k < kernSize; ++k) {
				sum += inPtr[k * stride] * vhkernPtr[k];
			}
			*reinterpret_cast<int16_t*>(outPtr) = sum;
			inPtr++;
			outPtr++;
		}

		inPtr += pad;
		outPtr += pad;
	}
	_mm256_zeroupper();
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */