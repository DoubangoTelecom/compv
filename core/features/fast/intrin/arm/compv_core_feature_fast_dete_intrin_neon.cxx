/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/features/fast/intrin/arm/compv_core_feature_fast_dete_intrin_neon.h"

#if COMPV_ARCH_ARM && COMPV_INTRINSIC
#include "compv/base/intrin/arm/compv_intrin_neon.h"
#include "compv/base/compv_simd_globals.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

void CompVFastDataRow_Intrin_NEON(const uint8_t* IP, COMPV_ALIGNED(NEON) compv_uscalar_t width, COMPV_ALIGNED(NEON) const compv_scalar_t *pixels16, compv_uscalar_t N, compv_uscalar_t threshold, uint8_t* strengths)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
}

void CompVFastNmsGather_Intrin_NEON(const uint8_t* pcStrengthsMap, uint8_t* pNMS, const compv_uscalar_t width, compv_uscalar_t heigth, COMPV_ALIGNED(NEON) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	compv_uscalar_t i, j;
	uint8x16_t vecStrength, vec0, vec1;
	pcStrengthsMap += (stride * 3);
	pNMS += (stride * 3);
	const uint8x16_t vecZero = vdupq_n_u8(0);
	// TODO(dmi): asm code -> comparison with zero is implicit?
	for (j = 3; j < heigth - 3; ++j) {
		for (i = 3; i < width - 3; i += 16) {
			vecStrength = vld1q_u8(&pcStrengthsMap[i]);
			vec1 = vcgtq_u8(vecStrength, vecZero); // vecStrength is unsigned which means checking it's not eq to #0 is same as checking it's > #0			
			if (COMPV_ARM_NEON_NEQ_ZERO(vec1)) {
				vec0 = vcgeq_u8(vld1q_u8(&pcStrengthsMap[i - 1]), vecStrength);  // left
				vec0 = vorrq_u8(vec0,
					vcgeq_u8(vld1q_u8(&pcStrengthsMap[i + 1]), vecStrength)); // right
				vec0 = vorrq_u8(vec0,
					vcgeq_u8(vld1q_u8(&pcStrengthsMap[i - stride - 1]), vecStrength)); // left-top
				vec0 = vorrq_u8(vec0,
					vcgeq_u8(vld1q_u8(&pcStrengthsMap[i - stride]), vecStrength)); // top
				vec0 = vorrq_u8(vec0,
					vcgeq_u8(vld1q_u8(&pcStrengthsMap[i - stride + 1]), vecStrength)); // right-top
				vec0 = vorrq_u8(vec0,
					vcgeq_u8(vld1q_u8(&pcStrengthsMap[i + stride - 1]), vecStrength)); // left-bottom
				vec0 = vorrq_u8(vec0,
					vcgeq_u8(vld1q_u8(&pcStrengthsMap[i + stride]), vecStrength)); // bottom
				vec0 = vorrq_u8(vec0,
					vcgeq_u8(vld1q_u8(&pcStrengthsMap[i + stride + 1]), vecStrength)); // right-bottom

				// 'and' used to ignore nonzero coeffs. Zero-coeffs are always suppressed (0# >= strength) which means too much work for the nmsApply() function
				vst1q_u8(&pNMS[i], vandq_u8(vec0, vec1));
			}
		}
		pcStrengthsMap += stride;
		pNMS += stride;
	}
}

void CompVFastNmsApply_Intrin_NEON(COMPV_ALIGNED(NEON) uint8_t* pcStrengthsMap, COMPV_ALIGNED(NEON) uint8_t* pNMS, compv_uscalar_t width, compv_uscalar_t heigth, COMPV_ALIGNED(NEON) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	// TODO(dmi): asm code -> comparison with zero is implicit?
	compv_uscalar_t i, j;
	uint8x16_t vec0;
	pcStrengthsMap += (stride * 3);
	pNMS += (stride * 3);
	const uint8x16_t vecZero = vdupq_n_u8(0);
	for (j = 3; j < heigth - 3; ++j) {
		for (i = 0; i < width; i += 16) { // SIMD: start at #zero index to have aligned memory
			vec0 = vcgtq_u8(vld1q_u8(&pNMS[i]), vecZero); // pNMS is unsigned which means checking it's not eq to #0 is same as checking it's > #0	
			if (COMPV_ARM_NEON_NEQ_ZERO(vec0)) {
				vst1q_u8(&pNMS[i], vecZero); // must, for next frame
				vst1q_u8(&pcStrengthsMap[i], vbicq_u8(vld1q_u8(&pcStrengthsMap[i]), vec0)); // suppress
			}
		}
		pcStrengthsMap += stride;
		pNMS += stride;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_ARM && COMPV_INTRINSIC */
