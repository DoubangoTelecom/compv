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
#include "compv/base/compv_cpu.h"
#include "compv/base/compv_debug.h"

#define _neon_fast_check(a, b) \
	vec0 = vld1q_u8(&circle[a][i]); \
	vec1 = vld1q_u8(&circle[b][i]); \
	vec2 = vorrq_u8( \
		vorrq_u8(vcltq_u8(vec0, vecDarker1), vcltq_u8(vec1, vecDarker1)), \
		vorrq_u8(vcgtq_u8(vec0, vecBrighter1), vcgtq_u8(vec1, vecBrighter1)) \
	); \
	if (COMPV_ARM_NEON_EQ_ZERO(vec2)) goto Next; \
	vecCircle16[a] = vec0, vecCircle16[b] = vec1


#define _neon_fast_compute_Darkers(a, b, c, d) \
	vecDiff16[a] = vqsubq_u8(vecDarker1, vecCircle16[a]); \
	vecDiff16[b] = vqsubq_u8(vecDarker1, vecCircle16[b]); \
	vecDiff16[c] = vqsubq_u8(vecDarker1, vecCircle16[c]); \
	vecDiff16[d] = vqsubq_u8(vecDarker1, vecCircle16[d])
#define _neon_fast_compute_Brighters(a, b, c, d) \
	vecDiff16[a] = vqsubq_u8(vecCircle16[a], vecBrighter1); \
	vecDiff16[b] = vqsubq_u8(vecCircle16[b], vecBrighter1); \
	vecDiff16[c] = vqsubq_u8(vecCircle16[c], vecBrighter1); \
	vecDiff16[d] = vqsubq_u8(vecCircle16[d], vecBrighter1)

#define _neon_fast_load(a, b, c, d, type) \
	_neon_fast_compute_##type##s(a, b, c, d); \
	vecDiffBinary16[a] = vandq_u8(vcgtq_u8(vecDiff16[a], vecZero), vecOne); \
	vecDiffBinary16[b] = vandq_u8(vcgtq_u8(vecDiff16[b], vecZero), vecOne); \
	vecDiffBinary16[c] = vandq_u8(vcgtq_u8(vecDiff16[c], vecZero), vecOne); \
	vecDiffBinary16[d] = vandq_u8(vcgtq_u8(vecDiff16[d], vecZero), vecOne); \
	vecSum1 = vaddq_u8(vecSum1, vaddq_u8(vecDiffBinary16[a], vecDiffBinary16[b])); \
	vecSum1 = vaddq_u8(vecSum1, vaddq_u8(vecDiffBinary16[c], vecDiffBinary16[d]))

// Sum arc #0 without the tail (#NminusOne lines)
#define _neon_fast_init_diffbinarysum(vecSum1, vecBinary16) \
	vecSum1 = vaddq_u8(vecBinary16[0], vecBinary16[1]); \
	vecSum1 = vaddq_u8(vecSum1, vecBinary16[2]); \
	vecSum1 = vaddq_u8(vecSum1, vecBinary16[3]); \
	vecSum1 = vaddq_u8(vecSum1, vecBinary16[4]); \
	vecSum1 = vaddq_u8(vecSum1, vecBinary16[5]); \
	vecSum1 = vaddq_u8(vecSum1, vecBinary16[6]); \
	vecSum1 = vaddq_u8(vecSum1, vecBinary16[7]); \
	if (N == 12) { \
		vecSum1 = vaddq_u8(vecSum1, vecBinary16[8]); \
		vecSum1 = vaddq_u8(vecSum1, vecBinary16[9]); \
		vecSum1 = vaddq_u8(vecSum1, vecBinary16[10]); \
	}
#define _neon_fast_strength(ii, vecSum1, vecDiff16, vecStrengths, vecTemp) \
	vecSum1 = vaddq_u8(vecSum1, vecDiffBinary16[(NminusOne + ii) & 15]); /* add tail */ \
	vecTemp = vcgeq_u8(vecSum1, vecN); \
	if (COMPV_ARM_NEON_NEQ_ZERO(vecTemp)) { \
		vecTemp = vecDiff16[(ii + 0) & 15]; \
		vecTemp = vminq_u8(vecDiff16[(ii + 1) & 15], vecTemp); \
		vecTemp = vminq_u8(vecDiff16[(ii + 2) & 15], vecTemp); \
		vecTemp = vminq_u8(vecDiff16[(ii + 3) & 15], vecTemp); \
		vecTemp = vminq_u8(vecDiff16[(ii + 4) & 15], vecTemp); \
		vecTemp = vminq_u8(vecDiff16[(ii + 5) & 15], vecTemp); \
		vecTemp = vminq_u8(vecDiff16[(ii + 6) & 15], vecTemp); \
		vecTemp = vminq_u8(vecDiff16[(ii + 7) & 15], vecTemp); \
		vecTemp = vminq_u8(vecDiff16[(ii + 8) & 15], vecTemp); \
		if (N == 12) { \
			vecTemp = vminq_u8(vecDiff16[(ii + 9) & 15], vecTemp); \
			vecTemp = vminq_u8(vecDiff16[(ii + 10) & 15], vecTemp); \
			vecTemp = vminq_u8(vecDiff16[(ii + 11) & 15], vecTemp); \
		} \
		vecStrengths = vmaxq_u8(vecStrengths, vecTemp); \
	} \
	vecSum1 = vsubq_u8(vecSum1, vecDiffBinary16[ii & 15]) /* sub head */

COMPV_NAMESPACE_BEGIN()

void CompVFastDataRow_Intrin_NEON(const uint8_t* IP, COMPV_ALIGNED(NEON) compv_uscalar_t width, const compv_scalar_t *pixels16, compv_uscalar_t N, compv_uscalar_t threshold, uint8_t* strengths)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	compv_uscalar_t i;
	const compv_uscalar_t NminusOne = N - 1;
	const uint8x16_t vecThreshold = vdupq_n_u8(static_cast<uint8_t>(threshold));
	const uint8x16_t vecMinSum = vdupq_n_u8(static_cast<uint8_t>((N == 12 ? 3 : 2))); // asm: N >> 2
	const uint8x16_t vecN = vdupq_n_u8(static_cast<uint8_t>(N));
	static const uint8x16_t vecOne = vdupq_n_u8(1);
	static const uint8x16_t vecZero = vdupq_n_u8(0);
	uint8x16_t vec0, vec1, vec2, vecSum1, vecStrengths, vecBrighter1, vecDarker1, vecDiffBinary16[16], vecDiff16[16], vecCircle16[16];
	const uint8_t* circle[16] = {
		&IP[pixels16[0]], &IP[pixels16[1]], &IP[pixels16[2]], &IP[pixels16[3]],
		&IP[pixels16[4]], &IP[pixels16[5]], &IP[pixels16[6]], &IP[pixels16[7]],
		&IP[pixels16[8]], &IP[pixels16[9]], &IP[pixels16[10]], &IP[pixels16[11]],
		&IP[pixels16[12]], &IP[pixels16[13]], &IP[pixels16[14]], &IP[pixels16[15]]
	};

	// TODO(dmi): ASM code faster by far. Why?

	for (i = 0; i < width; i += 16) {
		vec0 = vld1q_u8(&IP[i]);
		vecDarker1 = vqsubq_u8(vec0, vecThreshold); // IP < (Ix - t)
		vecBrighter1 = vqaddq_u8(vec0, vecThreshold); // Ip > (Ix + t)
		vecStrengths = vdupq_n_u8(0);

		/* Check */ {
			// Order is important for performance: When (a, b) is a candidate pair then the points close to 
			// it are also probable candidates. This is what we check farrest neighborhoods. For example,
			// (4, 12) is the farrestneighborhood for (0, 8)
			_neon_fast_check(0, 8); _neon_fast_check(4, 12);
			_neon_fast_check(1, 9); _neon_fast_check(5, 13);
			_neon_fast_check(2, 10); _neon_fast_check(6, 14);
			_neon_fast_check(3, 11); _neon_fast_check(7, 15);
		}
        if (i > 10000)
            printf("FIXME:%u\n", i);
#if 0
		/* Darkers */ {
			vecSum1 = vdupq_n_u8(0);
			_neon_fast_load(0, 8, 4, 12, Darker);
			vec0 = vcgeq_u8(vecSum1, vecMinSum);
			if (COMPV_ARM_NEON_EQ_ZERO(vec0)) goto EndOfDarkers; // at least #3 for FAST12 and #2 for FAST9
			_neon_fast_load(1, 9, 5, 13, Darker);
			_neon_fast_load(2, 10, 6, 14, Darker);
			_neon_fast_load(3, 11, 7, 15, Darker);
			vec0 = vcgeq_u8(vecSum1, vecN);
			if (COMPV_ARM_NEON_EQ_ZERO(vec0)) goto EndOfDarkers; // at least #12 for FAST12 and #9 for FAST9

			_neon_fast_init_diffbinarysum(vecSum1, vecDiffBinary16);
			_neon_fast_strength(0, vecSum1, vecDiff16, vecStrengths, vec0); _neon_fast_strength(1, vecSum1, vecDiff16, vecStrengths, vec0);
			_neon_fast_strength(2, vecSum1, vecDiff16, vecStrengths, vec0); _neon_fast_strength(3, vecSum1, vecDiff16, vecStrengths, vec0);
			_neon_fast_strength(4, vecSum1, vecDiff16, vecStrengths, vec0); _neon_fast_strength(5, vecSum1, vecDiff16, vecStrengths, vec0);
			_neon_fast_strength(6, vecSum1, vecDiff16, vecStrengths, vec0); _neon_fast_strength(7, vecSum1, vecDiff16, vecStrengths, vec0);
			_neon_fast_strength(8, vecSum1, vecDiff16, vecStrengths, vec0); _neon_fast_strength(9, vecSum1, vecDiff16, vecStrengths, vec0);
			_neon_fast_strength(10, vecSum1, vecDiff16, vecStrengths, vec0); _neon_fast_strength(11, vecSum1, vecDiff16, vecStrengths, vec0);
			_neon_fast_strength(12, vecSum1, vecDiff16, vecStrengths, vec0); _neon_fast_strength(13, vecSum1, vecDiff16, vecStrengths, vec0);
			_neon_fast_strength(14, vecSum1, vecDiff16, vecStrengths, vec0); _neon_fast_strength(15, vecSum1, vecDiff16, vecStrengths, vec0);
		} EndOfDarkers:

		/* Brighters */ {
			vecSum1 = vdupq_n_u8(0);
			_neon_fast_load(0, 8, 4, 12, Brighter);
			vec0 = vcgeq_u8(vecSum1, vecMinSum);
			if (COMPV_ARM_NEON_EQ_ZERO(vec0)) goto EndOfBrighters; // at least #3 for FAST12 and #2 for FAST9
			_neon_fast_load(1, 9, 5, 13, Brighter);
			_neon_fast_load(2, 10, 6, 14, Brighter);
			_neon_fast_load(3, 11, 7, 15, Brighter);
			vec0 = vcgeq_u8(vecSum1, vecN);
			if (COMPV_ARM_NEON_EQ_ZERO(vec0)) goto EndOfBrighters; // at least #12 for FAST12 and #9 for FAST9

			_neon_fast_init_diffbinarysum(vecSum1, vecDiffBinary16);
			_neon_fast_strength(0, vecSum1, vecDiff16, vecStrengths, vec0); _neon_fast_strength(1, vecSum1, vecDiff16, vecStrengths, vec0);
			_neon_fast_strength(2, vecSum1, vecDiff16, vecStrengths, vec0); _neon_fast_strength(3, vecSum1, vecDiff16, vecStrengths, vec0);
			_neon_fast_strength(4, vecSum1, vecDiff16, vecStrengths, vec0); _neon_fast_strength(5, vecSum1, vecDiff16, vecStrengths, vec0);
			_neon_fast_strength(6, vecSum1, vecDiff16, vecStrengths, vec0); _neon_fast_strength(7, vecSum1, vecDiff16, vecStrengths, vec0);
			_neon_fast_strength(8, vecSum1, vecDiff16, vecStrengths, vec0); _neon_fast_strength(9, vecSum1, vecDiff16, vecStrengths, vec0);
			_neon_fast_strength(10, vecSum1, vecDiff16, vecStrengths, vec0); _neon_fast_strength(11, vecSum1, vecDiff16, vecStrengths, vec0);
			_neon_fast_strength(12, vecSum1, vecDiff16, vecStrengths, vec0); _neon_fast_strength(13, vecSum1, vecDiff16, vecStrengths, vec0);
			_neon_fast_strength(14, vecSum1, vecDiff16, vecStrengths, vec0); _neon_fast_strength(15, vecSum1, vecDiff16, vecStrengths, vec0);
		} EndOfBrighters:
#endif
	
	Next:
		vst1q_u8(&strengths[i], vecStrengths);
	}
}

void CompVFastNmsGather_Intrin_NEON(const uint8_t* pcStrengthsMap, uint8_t* pNMS, const compv_uscalar_t width, compv_uscalar_t heigth, COMPV_ALIGNED(NEON) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	compv_uscalar_t i, j;
	uint8x16_t vecStrength, vec0, vec1;
	
	pcStrengthsMap += (stride * 3);
	pNMS += (stride * 3);
	static const uint8x16_t vecZero = vdupq_n_u8(0);
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
	static const uint8x16_t vecZero = vdupq_n_u8(0);
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

void CompVFastBuildInterestPoints_Intrin_NEON(COMPV_ALIGNED(NEON) uint8_t* pcStrengthsMap, std::vector<CompVInterestPoint>& interestPoints, compv_uscalar_t thresholdMinus1, const compv_uscalar_t jstart, compv_uscalar_t jend, compv_uscalar_t width, COMPV_ALIGNED(NEON) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("C++ code faster");
#define COMPV_PUSH1_NEON(ii) \
	if (vgetq_lane_u8(vec0, ii)) { \
		interestPoints.push_back(CompVInterestPoint( \
			static_cast<compv_float32_t>(i + ii), \
			static_cast<compv_float32_t>(j), \
			static_cast<compv_float32_t>(pcStrengthsMap[i + ii] + thresholdMinus1))); \
	}
	static const uint8x16_t vecZero = vdupq_n_u8(0);
	uint8x16_t vec0;
	for (compv_uscalar_t j = jstart; j < jend; ++j) {
		for (compv_uscalar_t i = 0; i < width; i += 16) {
			vec0 = vcgtq_u8(vld1q_u8(&pcStrengthsMap[i]), vecZero);
			if (COMPV_ARM_NEON_NEQ_ZERO(vec0)) {
				COMPV_PUSH1_NEON(0); COMPV_PUSH1_NEON(1); COMPV_PUSH1_NEON(2); COMPV_PUSH1_NEON(3); COMPV_PUSH1_NEON(4); COMPV_PUSH1_NEON(5); COMPV_PUSH1_NEON(6); COMPV_PUSH1_NEON(7);
				COMPV_PUSH1_NEON(8); COMPV_PUSH1_NEON(9); COMPV_PUSH1_NEON(10); COMPV_PUSH1_NEON(11); COMPV_PUSH1_NEON(12); COMPV_PUSH1_NEON(13); COMPV_PUSH1_NEON(14); COMPV_PUSH1_NEON(15);
			}
		}
		pcStrengthsMap += stride;
	}
#undef COMPV_PUSH1_NEON
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_ARM && COMPV_INTRINSIC */
