/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/arm/compv_math_distance_intrin_neon.h"

#if COMPV_ARCH_ARM && COMPV_INTRINSIC
#include "compv/base/intrin/arm/compv_intrin_neon.h"
#include "compv/base/compv_simd_globals.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_bits.h"
#include "compv/base/compv_debug.h"

// 4 rows, 16 bytes each, result in vec0
#define __hamming4x16(ii, vecPatch) \
	vec0 = vcntq_u8(veorq_u8(vld1q_u8(&dataPtr[ii + 0]), vecPatch)); \
	vec1 = vcntq_u8(veorq_u8(vld1q_u8(&dataPtr[ii + stride]), vecPatch)); \
	vec2 = vcntq_u8(veorq_u8(vld1q_u8(&dataPtr[ii + strideTimes2]), vecPatch)); \
	vec3 = vcntq_u8(veorq_u8(vld1q_u8(&dataPtr[ii + strideTimes3]), vecPatch)); \
	vec0 = vcombine_u8( \
		vpadd_u8(vget_low_u8(vec0), vget_high_u8(vec0)), \
		vpadd_u8(vget_low_u8(vec1), vget_high_u8(vec1)) \
	); \
	vec1 = vcombine_u8( \
		vpadd_u8(vget_low_u8(vec2), vget_high_u8(vec2)), \
		vpadd_u8(vget_low_u8(vec3), vget_high_u8(vec3)) \
	); \
	vec0 = vcombine_u8( \
		vpadd_u8(vget_low_u8(vec0), vget_high_u8(vec0)), \
		vpadd_u8(vget_low_u8(vec1), vget_high_u8(vec1)) \
	)

// 1 row, 64 bytes each, result in vec0
#define __hamming1x64(ii) \
	vec0 = vcntq_u8(veorq_u8(vld1q_u8(&dataPtr[ii + 0]), vld1q_u8(&patch1xnPtr[ii + 0]))); \
	vec1 = vcntq_u8(veorq_u8(vld1q_u8(&dataPtr[ii + 16]), vld1q_u8(&patch1xnPtr[ii + 16]))); \
	vec2 = vcntq_u8(veorq_u8(vld1q_u8(&dataPtr[ii + 32]), vld1q_u8(&patch1xnPtr[ii + 32]))); \
	vec3 = vcntq_u8(veorq_u8(vld1q_u8(&dataPtr[ii + 48]), vld1q_u8(&patch1xnPtr[ii + 48]))); \
	vec0 = vcombine_u8( \
		vpadd_u8(vget_low_u8(vec0), vget_high_u8(vec0)), \
		vpadd_u8(vget_low_u8(vec1), vget_high_u8(vec1)) \
	); \
	vec1 = vcombine_u8( \
		vpadd_u8(vget_low_u8(vec2), vget_high_u8(vec2)), \
		vpadd_u8(vget_low_u8(vec3), vget_high_u8(vec3)) \
	); \
	vec0 = vcombine_u8( \
		vpadd_u8(vget_low_u8(vec0), vget_high_u8(vec0)), \
		vpadd_u8(vget_low_u8(vec1), vget_high_u8(vec1)) \
	)

// 1 row, 16 bytes each, result in vec0
#define __hamming1x16(ii) \
	vec0 = vcntq_u8(veorq_u8(vld1q_u8(&dataPtr[ii]), vld1q_u8(&patch1xnPtr[ii])))

// 4 rows, 8 bytes each (FIXME(dmi): use SIMD)
#if COMPV_ARCH_ARM64
#	define __hamming4x8() \
	if (i < width - 7) { \
		patch64 = *reinterpret_cast<const uint64_t*>(&patch1xnPtr[i]); \
		cntx4[0] += compv_popcnt64(*reinterpret_cast<const uint64_t*>(&dataPtr[i]) ^ patch64); \
		cntx4[1] += compv_popcnt64(*reinterpret_cast<const uint64_t*>(&dataPtr[i + stride]) ^ patch64); \
		cntx4[2] += compv_popcnt64(*reinterpret_cast<const uint64_t*>(&dataPtr[i + strideTimes2]) ^ patch64); \
		cntx4[3] += compv_popcnt64(*reinterpret_cast<const uint64_t*>(&dataPtr[i + strideTimes3]) ^ patch64); \
		i += 8; \
	}
#	define __hamming1x8() \
	if (i < width - 7) { \
		cntx1 += compv_popcnt64(*reinterpret_cast<const uint64_t*>(&dataPtr[i]) ^ *reinterpret_cast<const uint64_t*>(&patch1xnPtr[i])); \
		i += 8; \
	}
#else
#	define __hamming4x8() \
	__hamming4x4(); /* First #4 bytes */ \
	__hamming4x4() /* Second #4 bytes */
#	define __hamming1x8() \
	__hamming1x4(); /* First #4 bytes */ \
	__hamming1x4() /* Second #4 bytes */
#endif

// 4 rows, 4 bytes each
#define __hamming4x4() \
	if (i < width - 3) { \
		patch32 = *reinterpret_cast<const uint32_t*>(&patch1xnPtr[i]); \
		cntx4[0] += compv_popcnt32(static_cast<uint32_t>(*reinterpret_cast<const uint32_t*>(&dataPtr[i]) ^ patch32)); \
		cntx4[1] += compv_popcnt32(static_cast<uint32_t>(*reinterpret_cast<const uint32_t*>(&dataPtr[i + stride]) ^ patch32)); \
		cntx4[2] += compv_popcnt32(static_cast<uint32_t>(*reinterpret_cast<const uint32_t*>(&dataPtr[i + strideTimes2]) ^ patch32)); \
		cntx4[3] += compv_popcnt32(static_cast<uint32_t>(*reinterpret_cast<const uint32_t*>(&dataPtr[i + strideTimes3]) ^ patch32)); \
		i += 4; \
	}
#define __hamming1x4() \
	if (i < width - 3) { \
		cntx1 += compv_popcnt32(static_cast<uint32_t>(*reinterpret_cast<const uint32_t*>(&dataPtr[i]) ^ *reinterpret_cast<const uint32_t*>(&patch1xnPtr[i]))); \
		i += 4; \
	}

// 4 rows, 2 bytes each
#define __hamming4x2() \
	if (i < width - 1) { \
		patch16 = *reinterpret_cast<const uint16_t*>(&patch1xnPtr[i]); \
		cntx4[0] += compv_popcnt16(static_cast<uint16_t>(*reinterpret_cast<const uint16_t*>(&dataPtr[i]) ^ patch16)); \
		cntx4[1] += compv_popcnt16(static_cast<uint16_t>(*reinterpret_cast<const uint16_t*>(&dataPtr[i + stride]) ^ patch16)); \
		cntx4[2] += compv_popcnt16(static_cast<uint16_t>(*reinterpret_cast<const uint16_t*>(&dataPtr[i + strideTimes2]) ^ patch16)); \
		cntx4[3] += compv_popcnt16(static_cast<uint16_t>(*reinterpret_cast<const uint16_t*>(&dataPtr[i + strideTimes3]) ^ patch16)); \
		i += 2; \
	}
#define __hamming1x2() \
	if (i < width - 1) { \
		cntx1 += compv_popcnt16(static_cast<uint16_t>(*reinterpret_cast<const uint16_t*>(&dataPtr[i]) ^ *reinterpret_cast<const uint16_t*>(&patch1xnPtr[i]))); \
		i += 2; \
	}

// 4 rows, 1 byte each
#define __hamming4x1() \
	if (i < width) { \
		patch16 = static_cast<uint16_t>(*reinterpret_cast<const uint8_t*>(&patch1xnPtr[i])); \
		cntx4[0] += compv_popcnt16(static_cast<uint16_t>(*reinterpret_cast<const uint8_t*>(&dataPtr[i]) ^ patch16)); \
		cntx4[1] += compv_popcnt16(static_cast<uint16_t>(*reinterpret_cast<const uint8_t*>(&dataPtr[i + stride]) ^ patch16)); \
		cntx4[2] += compv_popcnt16(static_cast<uint16_t>(*reinterpret_cast<const uint8_t*>(&dataPtr[i + strideTimes2]) ^ patch16)); \
		cntx4[3] += compv_popcnt16(static_cast<uint16_t>(*reinterpret_cast<const uint8_t*>(&dataPtr[i + strideTimes3]) ^ patch16)); \
		i += 1; \
	}
#define __hamming1x1() \
	if (i < width) { \
		cntx1 += compv_popcnt16(static_cast<uint16_t>(*reinterpret_cast<const uint8_t*>(&dataPtr[i]) ^ static_cast<uint16_t>(*reinterpret_cast<const uint8_t*>(&patch1xnPtr[i])))); \
		i += 1; \
	}

COMPV_NAMESPACE_BEGIN()

// requires width > 15
void CompVMathDistanceHamming_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* dataPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride, COMPV_ALIGNED(NEON) const uint8_t* patch1xnPtr, int32_t* distPtr)
{
	compv_uscalar_t i, j;
#if COMPV_ARCH_ARM64
	uint64_t patch64;
#else
	int32x2_t veccntn;
#endif
	COMPV_ALIGN_NEON() int32_t cntx4[4];
	int32_t cntx1;
	uint32_t patch32;
	uint16_t patch16;
	uint8x16_t vec0, vec1, vec2, vec3, vecPatch;
	int32x4_t veccnt;
	compv_uscalar_t strideTimes2 = (stride << 1);
	compv_uscalar_t strideTimes3 = strideTimes2 + stride;
	compv_uscalar_t strideTimes4 = strideTimes2 << 1;
	
	j = 0;

	/*** Loop_H4 ***/
	if (height > 3) {
		for (; j < height - 3; j += 4) {
			veccnt = vdupq_n_s32(0);
			
			/* Loop_H4_W16 */
			for (i = 0; i < width - 15; i += 16) { // we already check that width is > 16 before calling this function
				vecPatch = vld1q_u8(&patch1xnPtr[i]);
				__hamming4x16(i, vecPatch);
				veccnt = vaddw_s16(veccnt, vpaddl_u8(vpadd_u8(vget_low_u8(vec0), vget_high_u8(vec0))));
			}
			
			/* Loop_H4_WX */
			if (width & 15) { // width multiple of 16 ?
				vst1q_s32(cntx4, vdupq_n_s32(0)); // cntx4[0] = cntx4[1] = cntx4[2] = cntx4[3] = 0;
				__hamming4x8();
				__hamming4x4();
				__hamming4x2();
				__hamming4x1();
				veccnt = vaddq_s32(veccnt, vld1q_s32(cntx4));
			}

			vst1q_s32(&distPtr[j], veccnt); // Remarque: asm, distPtr not aligned when multithreaded
			dataPtr += strideTimes4;
		}
	}

	/*** Loop_H1 ***/
	for (; j < height; j += 1) {
		veccnt = vdupq_n_s32(0);

		i = 0;

		/* Loop_H1_W64 */
		if (width > 63) {
			for (; i < width - 63; i += 64) {
				__hamming1x64(i);
				veccnt = vaddw_s16(veccnt, vpaddl_u8(vpadd_u8(vget_low_u8(vec0), vget_high_u8(vec0))));
			}
		}

		/* Loop_H1_W16 */
		for (; i < width - 15; i += 16) { // we already check that width is > 16 before calling this function
			__hamming1x16(i);
			veccnt = vaddw_s16(veccnt, vpaddl_u8(vpadd_u8(vget_low_u8(vec0), vget_high_u8(vec0))));
		}

#if COMPV_ARCH_ARM64
		cntx1 = vget_lane_s32(vpaddl_s32(vpadd_s32(vget_low_s32(veccnt), vget_high_s32(veccnt))), 0);
#else
		veccntn = vpadd_s32(vget_low_s32(veccnt), vget_high_s32(veccnt));
		cntx1 = vget_lane_s32(veccntn, 0) + vget_lane_s32(veccntn, 1);
#endif

		/* Loop_H1_WX */
		if (width & 15) { // width multiple of 16 ?
			__hamming1x8();
			__hamming1x4();
			__hamming1x2();
			__hamming1x1();
		}

		distPtr[j] = cntx1;
		dataPtr += stride;
	}
}

// width = 32 (common, e.g. very common (Brief256_31))
void CompVMathDistanceHamming32_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* dataPtr, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride, COMPV_ALIGNED(NEON) const uint8_t* patch1xnPtr, int32_t* distPtr)
{
	compv_uscalar_t j;
	uint8x16_t vec0, vec1, vec2, vec3;
	int16x4_t veccnt; // width = 32 which means popcnt is <= 256 (32 * 8) and can be stored in int16
#if COMPV_ARCH_ARM32
	int32x2_t veccntn;
#endif
	compv_uscalar_t strideTimes2 = (stride << 1);
	compv_uscalar_t strideTimes3 = strideTimes2 + stride;
	compv_uscalar_t strideTimes4 = strideTimes2 << 1;
	const uint8x16_t vecPatch0 = vld1q_u8(&patch1xnPtr[0]);
	const uint8x16_t vecPatch1 = vld1q_u8(&patch1xnPtr[16]);

	j = 0;

	/*** Loop_H4 ***/
	if (height > 3) {
		for (; j < height - 3; j += 4) {
			__hamming4x16(0, vecPatch0);
			veccnt = vpaddl_u8(vpadd_u8(vget_low_u8(vec0), vget_high_u8(vec0))); // paiwise add long
			__hamming4x16(16, vecPatch1);
			veccnt = vpadal_u8(veccnt, vpadd_u8(vget_low_u8(vec0), vget_high_u8(vec0))); // paiwise add and accumulate long
			vst1q_s32(&distPtr[j], vmovl_s16(veccnt)); // Remarque: asm, distPtr not aligned when multithreaded
			dataPtr += strideTimes4;
		}
	}

	/*** Loop_H1 ***/
	for (; j < height; j += 1) {
		vec0 = vcntq_u8(veorq_u8(vld1q_u8(&dataPtr[0]), vecPatch0));
		vec1 = vcntq_u8(veorq_u8(vld1q_u8(&dataPtr[16]), vecPatch1));
		vec0 = vcombine_u8(
			vpadd_u8(vget_low_u8(vec0), vget_high_u8(vec0)),
			vpadd_u8(vget_low_u8(vec1), vget_high_u8(vec1))
		);
		veccnt = vpaddl_u8(vpadd_u8(vget_low_u8(vec0), vget_high_u8(vec0)));
#if COMPV_ARCH_ARM64
		distPtr[j] = vget_lane_s32(vpaddl_s32(vpaddl_u16(veccnt)), 0);
#else
		veccntn = vpaddl_u16(veccnt);
		distPtr[j] = vget_lane_s32(veccntn, 0) + vget_lane_s32(veccntn, 1);
#endif
		dataPtr += stride;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_ARM && COMPV_INTRINSIC */
