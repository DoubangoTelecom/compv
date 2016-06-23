/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/intrinsics/x86/math/compv_math_matrix_intrin_avx.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/intrinsics/x86/compv_intrin_avx.h"
#include "compv/compv_simd_globals.h"
#include "compv/math/compv_math.h"
#include "compv/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

#if defined __INTEL_COMPILER
#	pragma intel optimization_parameter target_arch=avx
#endif
// We'll read beyond the end of the data which means ri and rj must be strided and IS_ALIGNED_AVX(strideInBytes)
void MatrixMulGA_float64_Intrin_AVX(COMPV_ALIGNED(AVX2) compv_float64_t* ri, COMPV_ALIGNED(AVX2) compv_float64_t* rj, const compv_float64_t* c1, const compv_float64_t* s1, compv_uscalar_t count)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // Use ASM
	COMPV_DEBUG_INFO_CHECK_AVX();

	_mm256_zeroupper();

	__m256d ymmC, ymmS, ymmRI0, ymmRJ0, ymmRI1, ymmRJ1;
	compv_scalar_t i, countSigned = static_cast<compv_scalar_t>(count);

	ymmC = _mm256_broadcast_sd(c1);
	ymmS = _mm256_broadcast_sd(s1);

	// FMA3 friendly but unfortunately not faster

	for (i = 0; i < countSigned - 7; i += 8) {
		ymmRI0 = _mm256_load_pd(&ri[i]);
		ymmRI1 = _mm256_load_pd(&ri[i + 4]);
		ymmRJ0 = _mm256_load_pd(&rj[i]);
		ymmRJ1 = _mm256_load_pd(&rj[i + 4]);
		_mm256_store_pd(&ri[i], _mm256_add_pd(_mm256_mul_pd(ymmRI0, ymmC), _mm256_mul_pd(ymmRJ0, ymmS)));
		_mm256_store_pd(&ri[i + 4], _mm256_add_pd(_mm256_mul_pd(ymmRI1, ymmC), _mm256_mul_pd(ymmRJ1, ymmS)));
		_mm256_store_pd(&rj[i], _mm256_sub_pd(_mm256_mul_pd(ymmRJ0, ymmC), _mm256_mul_pd(ymmRI0, ymmS)));
		_mm256_store_pd(&rj[i + 4], _mm256_sub_pd(_mm256_mul_pd(ymmRJ1, ymmC), _mm256_mul_pd(ymmRI1, ymmS)));
	}
	for (; i < countSigned; i += 4) { // more than count, upto stride
		ymmRI0 = _mm256_load_pd(&ri[i]);
		ymmRJ0 = _mm256_load_pd(&rj[i]);
		_mm256_store_pd(&ri[i], _mm256_add_pd(_mm256_mul_pd(ymmRI0, ymmC), _mm256_mul_pd(ymmRJ0, ymmS)));
		_mm256_store_pd(&rj[i], _mm256_sub_pd(_mm256_mul_pd(ymmRJ0, ymmC), _mm256_mul_pd(ymmRI0, ymmS)));
	}

	_mm256_zeroupper();
}

#if defined __INTEL_COMPILER
#	pragma intel optimization_parameter target_arch=avx
#endif
// We'll read beyond the end of the data which means ri and rj must be strided
void MatrixMulGA_float32_Intrin_AVX(COMPV_ALIGNED(AVX) compv_float32_t* ri, COMPV_ALIGNED(AVX) compv_float32_t* rj, const compv_float32_t* c1, const compv_float32_t* s1, compv_uscalar_t count)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // Use ASM which support FMA3
	COMPV_DEBUG_INFO_CHECK_AVX();

	_mm256_zeroupper();

	__m256 ymmC, ymmS, ymmRI, ymmRJ;

	ymmC = _mm256_broadcast_ss(c1);
	ymmS = _mm256_broadcast_ss(s1);

	for (compv_uscalar_t i = 0; i < count; i += 8) { // more than count, upto stride
		ymmRI = _mm256_load_ps(&ri[i]);
		ymmRJ = _mm256_load_ps(&rj[i]);

#if 1
		_mm256_store_ps(&ri[i], _mm256_add_ps(_mm256_mul_ps(ymmRI, ymmC), _mm256_mul_ps(ymmRJ, ymmS)));
		_mm256_store_ps(&rj[i], _mm256_sub_ps(_mm256_mul_ps(ymmRJ, ymmC), _mm256_mul_ps(ymmRI, ymmS)));
#else // FMA3 disable (different MD5 result and not faster)
		COMPV_DEBUG_INFO_CODE_FOR_TESTING(); // FMA3
		_mm256_store_ps(&ri[i], _mm256_fmadd_ps(ymmC, ymmRI, _mm256_mul_ps(ymmS, ymmRJ)));
		_mm256_store_ps(&rj[i], _mm256_fmsub_ps(ymmC, ymmRJ, _mm256_mul_ps(ymmS, ymmRI)));
#endif
	}
	_mm256_zeroupper();
}

#if defined __INTEL_COMPILER
#	pragma intel optimization_parameter target_arch=avx
#endif
void MatrixMulABt_float64_minpack1_Intrin_AVX(const COMPV_ALIGNED(AVX) compv_float64_t* A, const COMPV_ALIGNED(AVX) compv_float64_t* B, compv_uscalar_t aRows, compv_uscalar_t bRows, compv_uscalar_t bCols, compv_uscalar_t aStrideInBytes, compv_uscalar_t bStrideInBytes, COMPV_ALIGNED(AVX) compv_float64_t* R, compv_uscalar_t rStrideInBytes)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // Use ASM
	COMPV_DEBUG_INFO_CHECK_AVX();
	
	_mm256_zeroupper();

	compv_uscalar_t i, j;

	const compv_float64_t* a = A;
	const compv_float64_t* b;
	compv_float64_t* r = R;

	compv_scalar_t k, bColsSigned = static_cast<compv_scalar_t>(bCols);

	__m256d ymmSum;
	__m256i ymmMaskToExtractFirst64Bits = _mm256_load_si256((__m256i*)kAVXMaskstore_0_u64);
	__m256i ymmMaskToExtractFirst128Bits = _mm256_load_si256((__m256i*)kAVXMaskstore_0_1_u64);

	for (i = 0; i < aRows; ++i) {
		b = B;
		for (j = 0; j < bRows; ++j) {
			ymmSum = _mm256_setzero_pd();
			for (k = 0; k < bColsSigned - 7; k += 8) {
				ymmSum = _mm256_add_pd(_mm256_mul_pd(_mm256_load_pd(&a[k]), _mm256_load_pd(&b[k])), ymmSum);
				ymmSum = _mm256_add_pd(_mm256_mul_pd(_mm256_load_pd(&a[k + 4]), _mm256_load_pd(&b[k + 4])), ymmSum);
			}

			if (k < bColsSigned - 3) {
				ymmSum = _mm256_add_pd(_mm256_mul_pd(_mm256_load_pd(&a[k]), _mm256_load_pd(&b[k])), ymmSum);
				k += 4;
			}
			if (k < bColsSigned - 1) {
				// asm "vmovapd xmm, [m256]"
				ymmSum = _mm256_add_pd(_mm256_mul_pd(_mm256_maskload_pd(&a[k], ymmMaskToExtractFirst128Bits), _mm256_maskload_pd(&b[k], ymmMaskToExtractFirst128Bits)), ymmSum);
				k += 2;
			}
			if (k < bColsSigned) {
				ymmSum = _mm256_add_pd(_mm256_mul_pd(_mm256_maskload_pd(&a[k], ymmMaskToExtractFirst64Bits), _mm256_maskload_pd(&b[k], ymmMaskToExtractFirst64Bits)), ymmSum);
			}
			ymmSum = _mm256_hadd_pd(ymmSum, ymmSum);
			ymmSum = _mm256_add_pd(ymmSum, _mm256_permute2f128_pd(ymmSum, ymmSum, 0x11));
			_mm256_maskstore_pd(&r[j], ymmMaskToExtractFirst64Bits, ymmSum);
			b = reinterpret_cast<const compv_float64_t*>(reinterpret_cast<const uint8_t*>(b)+bStrideInBytes);
		}
		a = reinterpret_cast<const compv_float64_t*>(reinterpret_cast<const uint8_t*>(a)+aStrideInBytes);
		r = reinterpret_cast<compv_float64_t*>(reinterpret_cast<uint8_t*>(r)+rStrideInBytes);
	}

	_mm256_zeroupper();
}

void MatrixMulABt_float64_minpack1_Intrin_FMA3_AVX(const COMPV_ALIGNED(AVX) compv_float64_t* A, const COMPV_ALIGNED(AVX) compv_float64_t* B, compv_uscalar_t aRows, compv_uscalar_t bRows, compv_uscalar_t bCols, compv_uscalar_t aStrideInBytes, compv_uscalar_t bStrideInBytes, COMPV_ALIGNED(AVX) compv_float64_t* R, compv_uscalar_t rStrideInBytes)
{
#if defined __INTEL_COMPILER
#	pragma intel optimization_parameter target_arch=avx
#endif
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // Use ASM
	COMPV_DEBUG_INFO_CHECK_AVX();
	
	_mm256_zeroupper();

	compv_uscalar_t i, j;

	const compv_float64_t* a = A;
	const compv_float64_t* b;
	compv_float64_t* r = R;

	compv_scalar_t k, bColsSigned = static_cast<compv_scalar_t>(bCols);

	__m256d ymmSum;
	__m256i ymmMaskToExtractFirst64Bits = _mm256_load_si256((__m256i*)kAVXMaskstore_0_u64);
	__m256i ymmMaskToExtractFirst128Bits = _mm256_load_si256((__m256i*)kAVXMaskstore_0_1_u64);

	for (i = 0; i < aRows; ++i) {
		b = B;
		for (j = 0; j < bRows; ++j) {
			ymmSum = _mm256_setzero_pd();
			for (k = 0; k < bColsSigned - 7; k += 8) {
				ymmSum = _mm256_fmadd_pd(_mm256_load_pd(&a[k]), _mm256_load_pd(&b[k]), ymmSum);
				ymmSum = _mm256_fmadd_pd(_mm256_load_pd(&a[k + 4]), _mm256_load_pd(&b[k + 4]), ymmSum);
			}

			if (k < bColsSigned - 3) {
				ymmSum = _mm256_fmadd_pd(_mm256_load_pd(&a[k]), _mm256_load_pd(&b[k]), ymmSum);
				k += 4;
			}
			if (k < bColsSigned - 1) {
				// asm "vmovapd xmm, [m256]"
				ymmSum = _mm256_fmadd_pd(_mm256_maskload_pd(&a[k], ymmMaskToExtractFirst128Bits), _mm256_maskload_pd(&b[k], ymmMaskToExtractFirst128Bits), ymmSum);
				k += 2;
			}
			if (k < bColsSigned) {
				ymmSum = _mm256_fmadd_pd(_mm256_maskload_pd(&a[k], ymmMaskToExtractFirst64Bits), _mm256_maskload_pd(&b[k], ymmMaskToExtractFirst64Bits), ymmSum);
			}
			ymmSum = _mm256_hadd_pd(ymmSum, ymmSum);
			ymmSum = _mm256_add_pd(ymmSum, _mm256_permute2f128_pd(ymmSum, ymmSum, 0x11));
			_mm256_maskstore_pd(&r[j], ymmMaskToExtractFirst64Bits, ymmSum);
			b = reinterpret_cast<const compv_float64_t*>(reinterpret_cast<const uint8_t*>(b)+bStrideInBytes);
		}
		a = reinterpret_cast<const compv_float64_t*>(reinterpret_cast<const uint8_t*>(a)+aStrideInBytes);
		r = reinterpret_cast<compv_float64_t*>(reinterpret_cast<uint8_t*>(r)+rStrideInBytes);
	}

	_mm256_zeroupper();
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
