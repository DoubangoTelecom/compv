/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/x86/compv_math_convlt_intrin_avx2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/intrin/x86/compv_intrin_avx.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_simd_globals.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// TODO(dmi): ASM code is faster (Visual Studio 2015)
#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx2
#endif
void CompVMathConvlt1VtHzFixedPoint_8u16u8u_Intrin_AVX2(const uint8_t* inPtr, uint8_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const uint16_t* vthzKernPtr, compv_uscalar_t kernSize)
{
	COMPV_DEBUG_INFO_CHECK_AVX2();
	_mm256_zeroupper();
	compv_uscalar_t i, j, k, row;
	const compv_uscalar_t stride = (width + pad);
	const compv_uscalar_t width32 = width & -32;
	__m256i vecInPtr, vec0, vec1, vecSum0, vecSum1, vecCoeff;
	const __m256i vecZero = _mm256_setzero_si256();
	COMPV_ALIGN_AVX() uint8_t mem[32];

	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 32) {
			vecSum0 = _mm256_setzero_si256();
			vecSum1 = _mm256_setzero_si256();
			for (row = 0, k = 0; row < kernSize; ++row, k += step) {
				vecInPtr = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&inPtr[i + k]));
				vecCoeff = _mm256_set1_epi16(static_cast<short>(vthzKernPtr[row]));
				vec0 = _mm256_unpacklo_epi8(vecInPtr, vecZero); // epi8 -> epi16
				vec1 = _mm256_unpackhi_epi8(vecInPtr, vecZero); // epi8 -> epi16
				vecSum0 = _mm256_adds_epu16(vecSum0, _mm256_mulhi_epu16(vec0, vecCoeff));
				vecSum1 = _mm256_adds_epu16(vecSum1, _mm256_mulhi_epu16(vec1, vecCoeff));
			}
			vec0 = _mm256_packus_epi16(vecSum0, vecSum1);
			if (i < width32) {
				_mm256_storeu_si256(reinterpret_cast<__m256i*>(&outPtr[i]), vec0);
			}
			else {
				_mm256_store_si256(reinterpret_cast<__m256i*>(mem), vec0);
				for (k = 0; i < width; ++i, ++k) {
					outPtr[i] = mem[k];
				}
			}
		}

		inPtr += stride;
		outPtr += stride;
	}
	_mm256_zeroupper();
}

// TODO(dmi): ASM code is faster (Visual Studio 2015)
#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx2
#endif
void CompVMathConvlt1VtHz_8u32f8u_Intrin_AVX2(const uint8_t* inPtr, uint8_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const compv_float32_t* vthzKernPtr, compv_uscalar_t kernSize)
{
	COMPV_DEBUG_INFO_CHECK_AVX2(); // AVX/SSE transition issues
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("You should use ASM code which has support for FMA");
	_mm256_zeroupper();
	compv_uscalar_t i, j, k, row;
	const compv_uscalar_t stride = (width + pad);
	const compv_uscalar_t width32 = width & -32;
	__m256i vec0i, vec1i, vec2i, vec3i;
	__m256 vecCoeff, vecSum0, vecSum1, vecSum2, vecSum3, vec0f, vec1f, vec2f, vec3f;
	const __m256i vecAEBFCGDH = _mm256_load_si256(reinterpret_cast<const __m256i*>(kAVXPermutevar8x32_AEBFCGDH_i32));
	COMPV_ALIGN_AVX() uint8_t mem[32];

	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 32) {
			vecSum0 = _mm256_setzero_ps();
			vecSum1 = _mm256_setzero_ps();
			vecSum2 = _mm256_setzero_ps();
			vecSum3 = _mm256_setzero_ps();
			for (row = 0, k = 0; row < kernSize; ++row, k += step) {
				vec0i = _mm256_cvtepu8_epi32(_mm_loadl_epi64(reinterpret_cast<const __m128i*>(&inPtr[i + k + 0])));
				vec1i = _mm256_cvtepu8_epi32(_mm_loadl_epi64(reinterpret_cast<const __m128i*>(&inPtr[i + k + 8])));
				vec2i = _mm256_cvtepu8_epi32(_mm_loadl_epi64(reinterpret_cast<const __m128i*>(&inPtr[i + k + 16])));
				vec3i = _mm256_cvtepu8_epi32(_mm_loadl_epi64(reinterpret_cast<const __m128i*>(&inPtr[i + k + 24])));
				vecCoeff = _mm256_set1_ps(vthzKernPtr[row]);
				vec0f = _mm256_cvtepi32_ps(vec0i);
				vec1f = _mm256_cvtepi32_ps(vec1i);
				vec2f = _mm256_cvtepi32_ps(vec2i);
				vec3f = _mm256_cvtepi32_ps(vec3i);
				vec0f = _mm256_mul_ps(vec0f, vecCoeff);
				vec1f = _mm256_mul_ps(vec1f, vecCoeff);
				vec2f = _mm256_mul_ps(vec2f, vecCoeff);
				vec3f = _mm256_mul_ps(vec3f, vecCoeff);
				vecSum0 = _mm256_add_ps(vecSum0, vec0f);
				vecSum1 = _mm256_add_ps(vecSum1, vec1f);
				vecSum2 = _mm256_add_ps(vecSum2, vec2f);
				vecSum3 = _mm256_add_ps(vecSum3, vec3f);
			}
			vec0i = _mm256_cvttps_epi32(vecSum0);
			vec1i = _mm256_cvttps_epi32(vecSum1);
			vec2i = _mm256_cvttps_epi32(vecSum2);
			vec3i = _mm256_cvttps_epi32(vecSum3);
			vec0i = _mm256_packs_epi32(vec0i, vec1i);
			vec2i = _mm256_packs_epi32(vec2i, vec3i);
			vec0i = _mm256_packus_epi16(vec0i, vec2i);
			vec0i = _mm256_permutevar8x32_epi32(vec0i, vecAEBFCGDH);
			if (i < width32) {
				_mm256_storeu_si256(reinterpret_cast<__m256i*>(&outPtr[i]), vec0i);
			}
			else {
				_mm256_store_si256(reinterpret_cast<__m256i*>(mem), vec0i);
				for (k = 0; i < width; ++i, ++k) {
					outPtr[i] = mem[k];
				}
			}
		}

		inPtr += stride;
		outPtr += stride;
	}
	_mm256_zeroupper();
}

// TODO(dmi): ASM code is faster (not only because of FMA) - Visual Studio 2015
#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx2
#endif
void CompVMathConvlt1VtHz_8u32f32f_Intrin_AVX2(const uint8_t* inPtr, compv_float32_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const compv_float32_t* vthzKernPtr, compv_uscalar_t kernSize)
{
	COMPV_DEBUG_INFO_CHECK_AVX2();
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("You should use ASM code which has support for FMA");
	_mm256_zeroupper();
	compv_uscalar_t i, j, k, row;
	const compv_uscalar_t stride = (width + pad);
	const compv_uscalar_t width32 = width & -32;
	__m256i vec0i, vec1i, vec2i, vec3i;
	__m256 vecCoeff, vecSum0, vecSum1, vecSum2, vecSum3, vec0f, vec1f, vec2f, vec3f;
	COMPV_ALIGN_AVX() compv_float32_t mem[8*4];

	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 32) {
			vecSum0 = _mm256_setzero_ps();
			vecSum1 = _mm256_setzero_ps();
			vecSum2 = _mm256_setzero_ps();
			vecSum3 = _mm256_setzero_ps();
			for (row = 0, k = 0; row < kernSize; ++row, k += step) {
				vec0i = _mm256_cvtepu8_epi32(_mm_loadl_epi64(reinterpret_cast<const __m128i*>(&inPtr[i + k + 0])));
				vec1i = _mm256_cvtepu8_epi32(_mm_loadl_epi64(reinterpret_cast<const __m128i*>(&inPtr[i + k + 8])));
				vec2i = _mm256_cvtepu8_epi32(_mm_loadl_epi64(reinterpret_cast<const __m128i*>(&inPtr[i + k + 16])));
				vec3i = _mm256_cvtepu8_epi32(_mm_loadl_epi64(reinterpret_cast<const __m128i*>(&inPtr[i + k + 24])));
				vecCoeff = _mm256_set1_ps(vthzKernPtr[row]);
				vec0f = _mm256_cvtepi32_ps(vec0i);
				vec1f = _mm256_cvtepi32_ps(vec1i);
				vec2f = _mm256_cvtepi32_ps(vec2i);
				vec3f = _mm256_cvtepi32_ps(vec3i);
				vec0f = _mm256_mul_ps(vec0f, vecCoeff);
				vec1f = _mm256_mul_ps(vec1f, vecCoeff);
				vec2f = _mm256_mul_ps(vec2f, vecCoeff);
				vec3f = _mm256_mul_ps(vec3f, vecCoeff);
				vecSum0 = _mm256_add_ps(vecSum0, vec0f);
				vecSum1 = _mm256_add_ps(vecSum1, vec1f);
				vecSum2 = _mm256_add_ps(vecSum2, vec2f);
				vecSum3 = _mm256_add_ps(vecSum3, vec3f);
			}
			if (i < width32) {
				_mm256_storeu_ps(&outPtr[i], vecSum0);
				_mm256_storeu_ps(&outPtr[i + 8], vecSum1);
				_mm256_storeu_ps(&outPtr[i + 16], vecSum2);
				_mm256_storeu_ps(&outPtr[i + 24], vecSum3);
			}
			else {
				_mm256_store_ps(&mem[0], vecSum0);
				_mm256_store_ps(&mem[8], vecSum1);
				_mm256_store_ps(&mem[16], vecSum2);
				_mm256_store_ps(&mem[24], vecSum3);
				for (k = 0; i < width; ++i, ++k) {
					outPtr[i] = mem[k];
				}
			}
		}

		inPtr += stride;
		outPtr += stride;
	}
	_mm256_zeroupper();
}

#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx2
#endif
void CompVMathConvlt1VtHz_32f32f32f_Intrin_AVX2(const compv_float32_t* inPtr, compv_float32_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const compv_float32_t* vthzKernPtr, compv_uscalar_t kernSize)
{
	COMPV_DEBUG_INFO_CHECK_AVX2();
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("You should use ASM code which has support for FMA");
	_mm256_zeroupper();
	compv_uscalar_t i, j, k, row;
	const compv_uscalar_t stride = (width + pad);
	const compv_uscalar_t width32 = width & -32;
	__m256 vecCoeff, vec0f, vec1f, vec2f, vec3f, vecSum0, vecSum1, vecSum2, vecSum3;
	const __m256i vecZero = _mm256_setzero_si256();
	COMPV_ALIGN_AVX() compv_float32_t mem[8 * 4];

	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 32) {
			vecSum0 = _mm256_setzero_ps();
			vecSum1 = _mm256_setzero_ps();
			vecSum2 = _mm256_setzero_ps();
			vecSum3 = _mm256_setzero_ps();
			for (row = 0, k = 0; row < kernSize; ++row, k += step) {
				vec0f = _mm256_loadu_ps(&inPtr[i + k]);
				vec1f = _mm256_loadu_ps(&inPtr[i + k + 8]);
				vec2f = _mm256_loadu_ps(&inPtr[i + k + 16]);
				vec3f = _mm256_loadu_ps(&inPtr[i + k + 24]);
				vecCoeff = _mm256_set1_ps(vthzKernPtr[row]);
				vec0f = _mm256_mul_ps(vec0f, vecCoeff);
				vec1f = _mm256_mul_ps(vec1f, vecCoeff);
				vec2f = _mm256_mul_ps(vec2f, vecCoeff);
				vec3f = _mm256_mul_ps(vec3f, vecCoeff);
				vecSum0 = _mm256_add_ps(vecSum0, vec0f);
				vecSum1 = _mm256_add_ps(vecSum1, vec1f);
				vecSum2 = _mm256_add_ps(vecSum2, vec2f);
				vecSum3 = _mm256_add_ps(vecSum3, vec3f);
			}
			if (i < width32) {
				_mm256_storeu_ps(&outPtr[i], vecSum0);
				_mm256_storeu_ps(&outPtr[i + 8], vecSum1);
				_mm256_storeu_ps(&outPtr[i + 16], vecSum2);
				_mm256_storeu_ps(&outPtr[i + 24], vecSum3);
			}
			else {
				_mm256_store_ps(&mem[0], vecSum0);
				_mm256_store_ps(&mem[8], vecSum1);
				_mm256_store_ps(&mem[16], vecSum2);
				_mm256_store_ps(&mem[24], vecSum3);
				for (k = 0; i < width; ++i, ++k) {
					outPtr[i] = mem[k];
				}
			}
		}

		inPtr += stride;
		outPtr += stride;
	}
	_mm256_zeroupper();
}

#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx2
#endif
void CompVMathConvlt1VtHz_32f32f8u_Intrin_AVX2(const compv_float32_t* inPtr, uint8_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const compv_float32_t* vthzKernPtr, compv_uscalar_t kernSize)
{
	COMPV_DEBUG_INFO_CHECK_AVX2();
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("You should use ASM code which has support for FMA");
	_mm256_zeroupper();
	compv_uscalar_t i, j, k, row;
	const compv_uscalar_t stride = (width + pad);
	const compv_uscalar_t width32 = width & -32;
	__m256 vecCoeff, vec0f, vec1f, vec2f, vec3f, vecSum0, vecSum1, vecSum2, vecSum3;
	const __m256i vecZero = _mm256_setzero_si256();
	const __m256i vecAEBFCGDH = _mm256_load_si256(reinterpret_cast<const __m256i*>(kAVXPermutevar8x32_AEBFCGDH_i32));
	COMPV_ALIGN_AVX() uint8_t mem[32];

	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 32) {
			vecSum0 = _mm256_setzero_ps();
			vecSum1 = _mm256_setzero_ps();
			vecSum2 = _mm256_setzero_ps();
			vecSum3 = _mm256_setzero_ps();
			for (row = 0, k = 0; row < kernSize; ++row, k += step) {
				vec0f = _mm256_loadu_ps(&inPtr[i + k]);
				vec1f = _mm256_loadu_ps(&inPtr[i + k + 8]);
				vec2f = _mm256_loadu_ps(&inPtr[i + k + 16]);
				vec3f = _mm256_loadu_ps(&inPtr[i + k + 24]);
				vecCoeff = _mm256_set1_ps(vthzKernPtr[row]);
				vec0f = _mm256_mul_ps(vec0f, vecCoeff);
				vec1f = _mm256_mul_ps(vec1f, vecCoeff);
				vec2f = _mm256_mul_ps(vec2f, vecCoeff);
				vec3f = _mm256_mul_ps(vec3f, vecCoeff);
				vecSum0 = _mm256_add_ps(vecSum0, vec0f);
				vecSum1 = _mm256_add_ps(vecSum1, vec1f);
				vecSum2 = _mm256_add_ps(vecSum2, vec2f);
				vecSum3 = _mm256_add_ps(vecSum3, vec3f);
			}
			vecSum0 = _mm256_castsi256_ps(_mm256_cvttps_epi32(vecSum0));
			vecSum1 = _mm256_castsi256_ps(_mm256_cvttps_epi32(vecSum1));
			vecSum2 = _mm256_castsi256_ps(_mm256_cvttps_epi32(vecSum2));
			vecSum3 = _mm256_castsi256_ps(_mm256_cvttps_epi32(vecSum3));
			vecSum0 = _mm256_castsi256_ps(_mm256_packs_epi32(_mm256_castps_si256(vecSum0), _mm256_castps_si256(vecSum1)));
			vecSum2 = _mm256_castsi256_ps(_mm256_packs_epi32(_mm256_castps_si256(vecSum2), _mm256_castps_si256(vecSum3)));
			vecSum0 = _mm256_castsi256_ps(_mm256_packus_epi16(_mm256_castps_si256(vecSum0), _mm256_castps_si256(vecSum2)));
			vecSum0 = _mm256_castsi256_ps(_mm256_permutevar8x32_epi32(_mm256_castps_si256(vecSum0), vecAEBFCGDH));
			if (i < width32) {
				_mm256_storeu_si256(reinterpret_cast<__m256i*>(&outPtr[i]), _mm256_castps_si256(vecSum0));
			}
			else {
				_mm256_store_si256(reinterpret_cast<__m256i*>(mem), _mm256_castps_si256(vecSum0));
				for (k = 0; i < width; ++i, ++k) {
					outPtr[i] = mem[k];
				}
			}
		}

		inPtr += stride;
		outPtr += stride;
	}
	_mm256_zeroupper();
}

#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx2
#endif
void CompVMathConvlt1VtHz_8u16s16s_Intrin_AVX2(const uint8_t* inPtr, int16_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const int16_t* vthzKernPtr, compv_uscalar_t kernSize)
{
	COMPV_DEBUG_INFO_CHECK_AVX2();
	_mm256_zeroupper();
	compv_uscalar_t i, j, k, row;
	const compv_uscalar_t stride = (width + pad);
	const compv_uscalar_t width32 = width & -32;
	__m256i vec0, vec1, vec2, vec3, vecSum0, vecSum1, vecSum2, vecSum3, vecCoeff;
	COMPV_ALIGN_AVX() int16_t mem[16*2];

	// Using int32_t as accumulator to avoid overflow

	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 32) {
			vecSum0 = _mm256_setzero_si256();
			vecSum1 = _mm256_setzero_si256();
			vecSum2 = _mm256_setzero_si256();
			vecSum3 = _mm256_setzero_si256();
			for (row = 0, k = 0; row < kernSize; ++row, k += step) {
				vec0 = _mm256_cvtepu8_epi32(_mm_loadl_epi64(reinterpret_cast<const __m128i*>(&inPtr[i + k + 0])));
				vec1 = _mm256_cvtepu8_epi32(_mm_loadl_epi64(reinterpret_cast<const __m128i*>(&inPtr[i + k + 8])));
				vec2 = _mm256_cvtepu8_epi32(_mm_loadl_epi64(reinterpret_cast<const __m128i*>(&inPtr[i + k + 16])));
				vec3 = _mm256_cvtepu8_epi32(_mm_loadl_epi64(reinterpret_cast<const __m128i*>(&inPtr[i + k + 24])));
				vecCoeff = _mm256_set1_epi32(static_cast<int>(vthzKernPtr[row]));
				vec0 = _mm256_mullo_epi32(vec0, vecCoeff);
				vec1 = _mm256_mullo_epi32(vec1, vecCoeff);
				vec2 = _mm256_mullo_epi32(vec2, vecCoeff);
				vec3 = _mm256_mullo_epi32(vec3, vecCoeff);
				vecSum0 = _mm256_add_epi32(vecSum0, vec0);
				vecSum1 = _mm256_add_epi32(vecSum1, vec1);
				vecSum2 = _mm256_add_epi32(vecSum2, vec2);
				vecSum3 = _mm256_add_epi32(vecSum3, vec3);
			}
			vecSum0 = _mm256_packs_epi32(vecSum0, vecSum1);
			vecSum2 = _mm256_packs_epi32(vecSum2, vecSum3);
			vecSum0 = _mm256_permute4x64_epi64(vecSum0, 0xD8);
			vecSum2 = _mm256_permute4x64_epi64(vecSum2, 0xD8);
			if (i < width32) {
				_mm256_storeu_si256(reinterpret_cast<__m256i*>(&outPtr[i]), vecSum0);
				_mm256_storeu_si256(reinterpret_cast<__m256i*>(&outPtr[i + 16]), vecSum2);
			}
			else {
				_mm256_storeu_si256(reinterpret_cast<__m256i*>(&mem[0]), vecSum0);
				_mm256_storeu_si256(reinterpret_cast<__m256i*>(&mem[16]), vecSum2);
				for (k = 0; i < width; ++i, ++k) {
					outPtr[i] = mem[k];
				}
			}
		}

		inPtr += stride;
		outPtr += stride;
	}
	_mm256_zeroupper();
}

#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx2
#endif
void CompVMathConvlt1VtHz_16s16s16s_Intrin_AVX2(const int16_t* inPtr, int16_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const int16_t* vthzKernPtr, compv_uscalar_t kernSize)
{
	COMPV_DEBUG_INFO_CHECK_AVX2();
	_mm256_zeroupper();
	compv_uscalar_t i, j, k, row;
	const compv_uscalar_t stride = (width + pad);
	const compv_uscalar_t width32 = width & -32;
	__m256i vec0, vec1, vec2, vec3, vecSum0, vecSum1, vecSum2, vecSum3, vecCoeff;
	COMPV_ALIGN_AVX() int16_t mem[16 * 2];
	
	// Using int32_t as accumulator to avoid overflow

	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 32) {
			vecSum0 = _mm256_setzero_si256();
			vecSum1 = _mm256_setzero_si256();
			vecSum2 = _mm256_setzero_si256();
			vecSum3 = _mm256_setzero_si256();
			for (row = 0, k = 0; row < kernSize; ++row, k += step) {
				vecCoeff = _mm256_set1_epi32(static_cast<int>(vthzKernPtr[row]));
				vec0 = _mm256_cvtepi16_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&inPtr[i + k + 0]))); // epi16 -> epi32
				vec1 = _mm256_cvtepi16_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&inPtr[i + k + 8]))); // epi16 -> epi32
				vec2 = _mm256_cvtepi16_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&inPtr[i + k + 16]))); // epi16 -> epi32
				vec3 = _mm256_cvtepi16_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&inPtr[i + k + 24]))); // epi16 -> epi32
				vec0 = _mm256_mullo_epi32(vec0, vecCoeff);
				vec1 = _mm256_mullo_epi32(vec1, vecCoeff);
				vec2 = _mm256_mullo_epi32(vec2, vecCoeff);
				vec3 = _mm256_mullo_epi32(vec3, vecCoeff);
				vecSum0 = _mm256_add_epi32(vecSum0, vec0);
				vecSum1 = _mm256_add_epi32(vecSum1, vec1);
				vecSum2 = _mm256_add_epi32(vecSum2, vec2);
				vecSum3 = _mm256_add_epi32(vecSum3, vec3);
			}
			vecSum0 = _mm256_packs_epi32(vecSum0, vecSum1);
			vecSum2 = _mm256_packs_epi32(vecSum2, vecSum3);
			vecSum0 = _mm256_permute4x64_epi64(vecSum0, 0xD8);
			vecSum2 = _mm256_permute4x64_epi64(vecSum2, 0xD8);
			if (i < width32) {
				_mm256_storeu_si256(reinterpret_cast<__m256i*>(&outPtr[i]), vecSum0);
				_mm256_storeu_si256(reinterpret_cast<__m256i*>(&outPtr[i + 16]), vecSum2);
			}
			else {
				_mm256_store_si256(reinterpret_cast<__m256i*>(&mem[0]), vecSum0);
				_mm256_store_si256(reinterpret_cast<__m256i*>(&mem[16]), vecSum2);
				for (k = 0; i < width; ++i, ++k) {
					outPtr[i] = mem[k];
				}
			}
		}

		inPtr += stride;
		outPtr += stride;
	}
	_mm256_zeroupper();
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
