/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/intrin/x86/compv_image_threshold_intrin_avx2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/intrin/x86/compv_intrin_avx.h"
#include "compv/base/compv_simd_globals.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx2
#endif
void CompVImageThresholdGlobal_8u8u_Intrin_AVX2(
	COMPV_ALIGNED(AVX) const uint8_t* inPtr,
	COMPV_ALIGNED(AVX) uint8_t* outPtr,
	compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(AVX) compv_uscalar_t stride,
	compv_uscalar_t threshold
)
{
	COMPV_DEBUG_INFO_CHECK_AVX2();

	_mm256_zeroupper();

	__m256i vec0, vec1, vec2, vec3;
	const __m256i vecThreshold = _mm256_set1_epi8((int8_t)(threshold ^ 0x80));
	const __m256i vecMask = _mm256_set1_epi8((int8_t)(0x80));
	const compv_uscalar_t width1 = width & -128;

	compv_uscalar_t i, j;
	for (j = 0; j < height; ++j) {
		for (i = 0; i < width1; i += 128) {
			vec0 = _mm256_load_si256(reinterpret_cast<const __m256i*>(&inPtr[i]));
			vec1 = _mm256_load_si256(reinterpret_cast<const __m256i*>(&inPtr[i + 32]));
			vec2 = _mm256_load_si256(reinterpret_cast<const __m256i*>(&inPtr[i + 64]));
			vec3 = _mm256_load_si256(reinterpret_cast<const __m256i*>(&inPtr[i + 96]));

			vec0 = _mm256_xor_si256(vec0, vecMask);
			vec1 = _mm256_xor_si256(vec1, vecMask);
			vec2 = _mm256_xor_si256(vec2, vecMask);
			vec3 = _mm256_xor_si256(vec3, vecMask);

			vec0 = _mm256_cmpgt_epi8(vec0, vecThreshold);
			vec1 = _mm256_cmpgt_epi8(vec1, vecThreshold);
			vec2 = _mm256_cmpgt_epi8(vec2, vecThreshold);
			vec3 = _mm256_cmpgt_epi8(vec3, vecThreshold);

			_mm256_store_si256(reinterpret_cast<__m256i*>(&outPtr[i]), vec0);
			_mm256_store_si256(reinterpret_cast<__m256i*>(&outPtr[i + 32]), vec1);
			_mm256_store_si256(reinterpret_cast<__m256i*>(&outPtr[i + 64]), vec2);
			_mm256_store_si256(reinterpret_cast<__m256i*>(&outPtr[i + 96]), vec3);
		}
		for (; i < width; i += 32) {
			vec0 = _mm256_load_si256(reinterpret_cast<const __m256i*>(&inPtr[i]));
			vec0 = _mm256_xor_si256(vec0, vecMask);
			vec0 = _mm256_cmpgt_epi8(vec0, vecThreshold);
			_mm256_store_si256(reinterpret_cast<__m256i*>(&outPtr[i]), vec0);
		}
		inPtr += stride;
		outPtr += stride;
	}

	_mm256_zeroupper();
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
