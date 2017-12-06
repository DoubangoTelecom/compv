/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/arm/compv_math_morph_intrin_neon.h"

#if COMPV_ARCH_ARM && COMPV_INTRINSIC
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

#define CompVMathMorphProcessOp_8u_Intrin_NEON(op, strelInputPtrsPtr, strelInputPtrsCount, outPtr, width, height, stride) { \
	COMPV_DEBUG_INFO_CHECK_NEON(); \
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("ASM code is faster"); \
	compv_uscalar_t i, j, k, v; \
	const compv_uscalar_t width64 = width & -64; \
	const compv_uscalar_t width16 = width & -16; \
	const compv_uscalar_t strelInputPtrsPad = (stride - ((width + 15) & -16)); /* Asm code not the same padding: reading orphans one-by-one instead of packed 16B */ \
	uint8x16_t vec0, vec1, vec2, vec3; \
	COMPV_ALIGN_SSE() uint8_t mem[16]; \
	 \
	for (j = 0, k = 0; j < height; ++j) { \
		for (i = 0; i < width64; i += 64, k += 64) { \
			vec0 = vld1q_u8(reinterpret_cast<const uint8_t*>(k + strelInputPtrsPtr[0])); \
			vec1 = vld1q_u8(reinterpret_cast<const uint8_t*>(k + strelInputPtrsPtr[0]) + 16); \
			vec2 = vld1q_u8(reinterpret_cast<const uint8_t*>(k + strelInputPtrsPtr[0]) + 32); \
			vec3 = vld1q_u8(reinterpret_cast<const uint8_t*>(k + strelInputPtrsPtr[0]) + 48); \
			for (v = 1; v < strelInputPtrsCount; ++v) { \
				vec0 = v##op##q_u8(vec0, vld1q_u8(reinterpret_cast<const uint8_t*>(k + strelInputPtrsPtr[v]))); \
				vec1 = v##op##q_u8(vec1, vld1q_u8(reinterpret_cast<const uint8_t*>(k + strelInputPtrsPtr[v]) + 16)); \
				vec2 = v##op##q_u8(vec2, vld1q_u8(reinterpret_cast<const uint8_t*>(k + strelInputPtrsPtr[v]) + 32)); \
				vec3 = v##op##q_u8(vec3, vld1q_u8(reinterpret_cast<const uint8_t*>(k + strelInputPtrsPtr[v]) + 48)); \
			} \
			vst1q_u8(reinterpret_cast<uint8_t*>(&outPtr[i]), vec0); \
			vst1q_u8(reinterpret_cast<uint8_t*>(&outPtr[i]) + 16, vec1); \
			vst1q_u8(reinterpret_cast<uint8_t*>(&outPtr[i]) + 32, vec2); \
			vst1q_u8(reinterpret_cast<uint8_t*>(&outPtr[i]) + 48, vec3); \
		} \
		for (; i < width; i += 16, k += 16) { \
			vec0 = vld1q_u8(reinterpret_cast<const uint8_t*>(k + strelInputPtrsPtr[0])); \
			for (v = 1; v < strelInputPtrsCount; ++v) { \
				vec0 = v##op##q_u8(vec0, vld1q_u8(reinterpret_cast<const uint8_t*>(k + strelInputPtrsPtr[v]))); \
			} \
			if (i < width16) { \
				vst1q_u8(reinterpret_cast<uint8_t*>(&outPtr[i]), vec0); \
			} \
			else { \
				vst1q_u8(reinterpret_cast<uint8_t*>(mem), vec0); \
				for (v = 0; i < width; ++i, ++v) { \
					outPtr[i] = mem[v]; \
				} \
			} \
		} \
		outPtr += stride; \
		k += strelInputPtrsPad; \
	} \
}

/* For Close operation (Dilate+Erode): */
// TODO(dmi): huge gain -> on Android (Huawei MediaPad2, no FMA, ARM32, (1285 x 1285), single threaded, #1000 times), asm code: 6313.ms, intrin code: 7552.ms - faster even without cache preload
// TODO(dmi): huge gain -> on Android (Huawei MediaPad2, no FMA, ARM64, (1285 x 1285), single threaded, #1000 times), asm code: 5418.ms, intrin code: 6022.ms - faster even without cache preload

/* For Erode operation: */
// TODO(dmi): huge gain -> on Android (Huawei MediaPad2, no FMA, ARM32, (1285 x 1285), single threaded, #1000 times), asm code: 2466.ms, intrin code: 3157.ms - faster even without cache preload
// TODO(dmi): small gain -> on Android (Huawei MediaPad2, no FMA, ARM64, (1285 x 1285), single threaded, #1000 times), asm code: 2024.ms, intrin code: 2123.ms - faster even without cache preload

void CompVMathMorphProcessErode_8u_Intrin_NEON(const compv_uscalar_t* strelInputPtrsPtr, const compv_uscalar_t strelInputPtrsCount, uint8_t* outPtr, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride)
{
	CompVMathMorphProcessOp_8u_Intrin_NEON(min, strelInputPtrsPtr, strelInputPtrsCount, outPtr, width, height, stride);
}

void CompVMathMorphProcessDilate_8u_Intrin_NEON(const compv_uscalar_t* strelInputPtrsPtr, const compv_uscalar_t strelInputPtrsCount, uint8_t* outPtr, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride)
{
	CompVMathMorphProcessOp_8u_Intrin_NEON(max, strelInputPtrsPtr, strelInputPtrsCount, outPtr, width, height, stride);
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_ARM && COMPV_INTRINSIC */
