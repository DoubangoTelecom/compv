/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/ccl/intrin/x86/compv_core_ccl_lsl_intrin_avx2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/core/compv_core_simd_globals.h"
#include "compv/base/intrin/x86/compv_intrin_avx.h"
#include "compv/base/compv_simd_globals.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// Function requires width > 32 (not ">=" but ">")
#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx2
#endif
void CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_8u16s_Intrin_AVX2(
	const uint8_t* Xi, const compv_uscalar_t Xi_stride,
	int16_t* ERi, 
	int16_t* RLCi, const compv_uscalar_t RLCi_stride,
	const compv_uscalar_t width, const compv_uscalar_t height
)
{
	COMPV_DEBUG_INFO_CHECK_AVX2();
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("SSE version faster");
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("ASM version faster is #8 times faster (thanks to 'bsf' and 'blsr' instructions)");
	_mm256_zeroupper();
	const int16_t width1 = static_cast<int16_t>(width);
	const int16_t width32 = (width1 - 1) & -32; // width > 32 (at least 33) which means never equal to zero
	int16_t er, i;
	__m256i vec0, vec1;
	unsigned mask, m; // unsigned shift (shr) instead of signed shift (sar)

	for (compv_uscalar_t j = 0; j < height; ++j) {
		er = (Xi[0] & 1);
		RLCi[0] = 0;

		// In asm code, no need to test "width16 != 0" because "width1" > 16 (at least 17)
		for (i = 1; i < width32; i += 32) {
			vec0 = _mm256_cmpeq_epi16(
				_mm256_loadu_si256(reinterpret_cast<const __m256i*>(&ERi[i - 1])),
				_mm256_loadu_si256(reinterpret_cast<const __m256i*>(&ERi[i]))
			);
			vec1 = _mm256_cmpeq_epi16(
				_mm256_loadu_si256(reinterpret_cast<const __m256i*>(&ERi[i + 15])),
				_mm256_loadu_si256(reinterpret_cast<const __m256i*>(&ERi[i + 16]))
			);
			vec0 = _mm256_packs_epi16(vec0, vec1);
			vec0 = _mm256_permute4x64_epi64(vec0, 0xD8);
			mask = _mm256_movemask_epi8(vec0) ^ 0xffffffff;
			if (mask) {
				m = i;
				do {
					if (mask & 1) {
						RLCi[er++] = m;
					}
					++m;
				} while (mask >>= 1);
			}
		}

		for (; i < width1; ++i) {
			if (ERi[i - 1] != ERi[i]) {
				RLCi[er++] = i;
			}
		}

		RLCi[er] = width1 - ((Xi[width1 - 1] & 1) ^ 1);

		/* next */
		Xi += Xi_stride;
		RLCi += RLCi_stride;
		ERi += width;
	}
	_mm256_zeroupper();
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
