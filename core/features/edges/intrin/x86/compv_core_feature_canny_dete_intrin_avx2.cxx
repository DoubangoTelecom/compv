/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/features/edges/intrin/x86/compv_core_feature_canny_dete_intrin_avx2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/intrin/x86/compv_intrin_avx.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx2
#endif
void CompVCannyNMSApply_Intrin_AVX2(COMPV_ALIGNED(AVX) uint16_t* grad, COMPV_ALIGNED(AVX) uint8_t* nms, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(AVX) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_AVX2(); // AVX/SSE transition issues
	_mm256_zeroupper();
	__m128i vec0n;
	__m256i vec0;
	compv_uscalar_t col_, row_;
	static const __m256i vecZero = _mm256_setzero_si256();
	for (row_ = 1; row_ < height; ++row_) { // row starts to #1 and ends at (heigth = imageHeigth - 1)
		for (col_ = 0; col_ < width; col_ += 16) { // SIMD, starts at 0 (instead of 1) to have memory aligned, reading beyong width which means data must be strided
			vec0n = _mm_cmpeq_epi8(_mm_load_si128(reinterpret_cast<const __m128i*>(&nms[col_])), _mm256_castsi256_si128(vecZero));
			if (_mm_movemask_epi8(vec0n) ^ 0xffff) {
				// TODO(dmi): assembler -> (with xmm1 = vec0n and ymm1 = vec0)
				// vpunpckhbw xmm2, xmm1, xmm1
				// vpunpcklbw xmm1, xmm1, xmm1
				// vinsertf128 ymm1, ymm1, xmm2, 1
				vec0 = _mm256_broadcastsi128_si256(vec0n);
				vec0 = _mm256_permute4x64_epi64(vec0, COMPV_MM_SHUFFLE(3, 1, 2, 0));
				vec0 = _mm256_and_si256(_mm256_unpacklo_epi8(vec0, vec0), _mm256_load_si256(reinterpret_cast<const __m256i*>(&grad[col_])));
				_mm256_store_si256(reinterpret_cast<__m256i*>(&grad[col_]), vec0);
				_mm_store_si128(reinterpret_cast<__m128i*>(&nms[col_]), _mm256_castsi256_si128(vecZero));
			}
		}
		nms += stride;
		grad += stride;
	}
	_mm256_zeroupper();
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */