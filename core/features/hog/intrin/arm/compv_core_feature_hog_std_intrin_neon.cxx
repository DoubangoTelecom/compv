/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/features/hog/intrin/arm/compv_core_feature_hog_std_intrin_neon.h"

#if COMPV_ARCH_ARM && COMPV_INTRINSIC
#include "compv/base/intrin/x86/compv_intrin_sse.h"
#include "compv/base/compv_simd_globals.h"
#include "compv/base/compv_debug.h"

#define USE_EXT 0

COMPV_NAMESPACE_BEGIN()

void CompVHogStdBuildMapHistForSingleCellBilinear_32f32s_Intrin_NEON(
	COMPV_ALIGNED(NEON) const compv_float32_t* magPtr,
	COMPV_ALIGNED(NEON) const compv_float32_t* dirPtr,
	COMPV_ALIGNED(NEON) compv_float32_t* mapHistPtr,
	const compv_float32_t* thetaMax1,
	const compv_float32_t* scaleBinWidth1,
	const int32_t* binWidth1,
	const int32_t* binIdxMax1,
	COMPV_ALIGNED(NEON) const compv_uscalar_t cellWidth, // should be small (<= 16)
	const compv_uscalar_t cellHeight,
	COMPV_ALIGNED(NEON) const compv_uscalar_t magStride,
	COMPV_ALIGNED(NEON) const compv_uscalar_t dirStride
)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
#if 0 // TODO(dmi): Add ASM code
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No ASM code");
#endif
	const float32x4_t vecthetaMax = vdupq_n_f32(*thetaMax1);
	const float32x4_t vecScaleBinWidth = vdupq_n_f32(*scaleBinWidth1);
	const float32x4_t vecZero = vdupq_n_f32(0.f);
	const float32x4_t vecHalf = vdupq_n_f32(0.5f);
	const int32x4_t vecOne_plus = vdupq_n_s32(1);
	const int32x4_t vecOne_minus = vdupq_n_s32(-1);
	const int32x4_t vecBinIdxMax = vdupq_n_s32(*binIdxMax1);
	const float32x4_t vecBinWidth = vdupq_n_f32(static_cast<compv_float32_t>(*binWidth1));
#if !USE_EXT
	COMPV_ALIGN_NEON() int32_t indices[8];
	COMPV_ALIGN_NEON() compv_float32_t values[8];
#endif
	for (compv_uscalar_t j = 0; j < cellHeight; ++j) {
		for (compv_uscalar_t i = 0; i < cellWidth; i += 4) {
			float32x4_t vecTheta = vld1q_f32(&dirPtr[i]);
			const float32x4_t vecMask = vcgtq_f32(vecTheta, vecthetaMax);
			vecTheta = vsubq_f32(vecTheta, vandq_s32(vecthetaMax, vecMask));
			const int32x4_t vecBinIdx = vcvtq_s32_f32(vsubq_f32(vmulq_f32(vecScaleBinWidth, vecTheta), vecHalf));
			const float32x4_t vecDiff = vsubq_f32(vmulq_f32(vsubq_f32(vecTheta, vmulq_f32(vcvtq_f32_s32(vecBinIdx), vecBinWidth)), vecScaleBinWidth), vecHalf);
			float32x4_t vecMagPtr = vld1q_f32(&magPtr[i]);
			const float32x4_t vecAVV = vabsq_f32(vmulq_f32(vecMagPtr, vecDiff));
			int32x4_t vecBinIdxNext = vcgeq_f32(vecDiff, vecZero);
			vecBinIdxNext = vorrq_s32(vandq_s32(vecBinIdxNext, vecOne_plus), vbicq_s32(vecOne_minus, vecBinIdxNext));
			vecBinIdxNext = vaddq_s32(vecBinIdxNext, vecBinIdx);
			const int32x4_t vecMaski0 = vcltq_s32(vecBinIdxNext, vecZero);
			const int32x4_t vecMaski1 = vcgtq_s32(vecBinIdxNext, vecBinIdxMax);
			vecBinIdxNext = vorrq_s32(
				vandq_s32(vecMaski0, vecBinIdxMax),
				vbicq_s32(vbicq_s32(vecBinIdxNext, vecMaski1), vecMaski0)
			);
			vecMagPtr = vsubq_f32(vecMagPtr, vecAVV);
#if !USE_EXT
			vst1q_s32(&indices[0], vecBinIdxNext);
			vst1q_s32(&indices[4], vecBinIdx);
			vst1q_f32(&values[0], vecAVV);
			vst1q_f32(&values[4], vecMagPtr);
			// Addition order is important if we want to have the same MD5 as serial code
			mapHistPtr[indices[0]] += values[0];
			mapHistPtr[indices[4]] += values[4];
			mapHistPtr[indices[1]] += values[1];
			mapHistPtr[indices[5]] += values[5];
			mapHistPtr[indices[2]] += values[2];
			mapHistPtr[indices[6]] += values[6];
			mapHistPtr[indices[3]] += values[3];
			mapHistPtr[indices[7]] += values[7];
#else
            mapHistPtr[vgetq_lane_s32(vecBinIdxNext, 0)] += vgetq_lane_f32(vecAVV, 0);
            mapHistPtr[vgetq_lane_s32(vecBinIdx, 0)] += vgetq_lane_f32(vecMagPtr, 0);
            mapHistPtr[vgetq_lane_s32(vecBinIdxNext, 1)] += vgetq_lane_f32(vecAVV, 1);
            mapHistPtr[vgetq_lane_s32(vecBinIdx, 1)] += vgetq_lane_f32(vecMagPtr, 1);
            mapHistPtr[vgetq_lane_s32(vecBinIdxNext, 2)] += vgetq_lane_f32(vecAVV, 2);
            mapHistPtr[vgetq_lane_s32(vecBinIdx, 2)] += vgetq_lane_f32(vecMagPtr, 2);
            mapHistPtr[vgetq_lane_s32(vecBinIdxNext, 3)] += vgetq_lane_f32(vecAVV, 3);
            mapHistPtr[vgetq_lane_s32(vecBinIdx, 3)] += vgetq_lane_f32(vecMagPtr, 3);
#endif
		}
		magPtr += magStride;
		dirPtr += dirStride;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_ARM && COMPV_INTRINSIC */
