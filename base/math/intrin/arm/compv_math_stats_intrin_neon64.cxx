/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/arm/compv_math_stats_intrin_neon64.h"

#if COMPV_ARCH_ARM64 && COMPV_INTRINSIC
#include "compv/base/compv_simd_globals.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

void CompVMathStatsNormalize2DHartley_64f_Intrin_NEON64(const COMPV_ALIGNED(NEON) compv_float64_t* x, const COMPV_ALIGNED(NEON) compv_float64_t* y, compv_uscalar_t numPoints, compv_float64_t* tx1, compv_float64_t* ty1, compv_float64_t* s1)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	compv_scalar_t i, numPoints_ = static_cast<compv_scalar_t>(numPoints);
	
	float64x2_t vec0, vec1, vec2, vec3;
	float64x2_t vecTx = vdupq_n_f64(0.);
	float64x2_t vecTy = vdupq_n_f64(0.);
	float64x2_t vecMagnitude = vdupq_n_f64(0.);
	const float64x1_t vecOneOverNumPoints = vdup_n_f64(static_cast<compv_float64_t>(1.) / static_cast<compv_float64_t>(numPoints));
	const float64x1_t vecSqrt2 = vdup_n_f64(COMPV_MATH_SQRT_2);

	/*** TX and TY ***/

	for (i = 0; i < numPoints_ - 7; i += 8) {
		vecTx = vaddq_f64(vaddq_f64(vaddq_f64(vld1q_f64(&x[i]), vld1q_f64(&x[i + 2])),
			vaddq_f64(vld1q_f64(&x[i + 4]), vld1q_f64(&x[i + 6]))), vecTx);
		vecTy = vaddq_f64(vaddq_f64(vaddq_f64(vld1q_f64(&y[i]), vld1q_f64(&y[i + 2])),
			vaddq_f64(vld1q_f64(&y[i + 4]), vld1q_f64(&y[i + 6]))), vecTy);
	}
	if (i < numPoints_ - 3) {
		vecTx = vaddq_f64(vaddq_f64(vld1q_f64(&x[i]), vld1q_f64(&x[i + 2])), vecTx);
		vecTy = vaddq_f64(vaddq_f64(vld1q_f64(&y[i]), vld1q_f64(&y[i + 2])), vecTy);
		i += 4;
	}
	if (i < numPoints_ - 1) {
		vecTx = vaddq_f64(vld1q_f64(&x[i]), vecTx);
		vecTy = vaddq_f64(vld1q_f64(&y[i]), vecTy);
		i += 2;
	}
	if (numPoints_ & 1) {
		vecTx = vcombine_f64(vadd_f64(vget_low_f64(vecTx), vld1_f64(&x[i])), vget_high_f64(vecTx));
		vecTy = vcombine_f64(vadd_f64(vget_low_f64(vecTy), vld1_f64(&y[i])), vget_high_f64(vecTy));
	}
	const float64x1_t vecTxn = vmul_f64(vadd_f64(vget_low_f64(vecTx), vget_high_f64(vecTx)), vecOneOverNumPoints); // horiz add and result in first double
	const float64x1_t vecTyn = vmul_f64(vadd_f64(vget_low_f64(vecTy), vget_high_f64(vecTy)), vecOneOverNumPoints); // horiz add and result in first double
	vecTx = vcombine_f64(vecTxn, vecTxn); // duplicate vecTxn
	vecTy = vcombine_f64(vecTyn, vecTyn); // duplicate vecTyn

	/*** Magnitude ***/

	for (i = 0; i < numPoints_ - 3; i += 4) { 
		vec0 = vsubq_f64(vld1q_f64(&x[i]), vecTx);
		vec1 = vsubq_f64(vld1q_f64(&y[i]), vecTy);
		vec2 = vsqrtq_f64(vaddq_f64(vmulq_f64(vec0, vec0), vmulq_f64(vec1, vec1)));
		vec0 = vsubq_f64(vld1q_f64(&x[i + 2]), vecTx);
		vec1 = vsubq_f64(vld1q_f64(&y[i + 2]), vecTy);
		vec3 = vsqrtq_f64(vaddq_f64(vmulq_f64(vec0, vec0), vmulq_f64(vec1, vec1)));
		vecMagnitude = vaddq_f64(vec2, vecMagnitude);
		vecMagnitude = vaddq_f64(vec3, vecMagnitude);
	}
	if (i < numPoints_ - 1) {
		vec0 = vsubq_f64(vld1q_f64(&x[i]), vecTx);
		vec1 = vsubq_f64(vld1q_f64(&y[i]), vecTy);
		vec2 = vsqrtq_f64(vaddq_f64(vmulq_f64(vec0, vec0), vmulq_f64(vec1, vec1)));
		vecMagnitude = vaddq_f64(vec2, vecMagnitude);
		i += 2;
	}
	if (numPoints_ & 1) {
		const float64x1_t vec0 = vsub_f64(vld1_f64(&x[i]), vecTxn);  // vecTxn = vget_low_f64(vecTx)
		const float64x1_t vec1 = vsub_f64(vld1_f64(&y[i]), vecTyn); // vecTyn = vget_low_f64(vecTy)
		const float64x1_t vec2 = vadd_f64(vmul_f64(vec0, vec0), vmul_f64(vec1, vec1));
		const float64x1_t vec3 = vsqrt_f64(vec2); 
		vecMagnitude = vcombine_f64(vadd_f64(vget_low_f64(vecMagnitude), vec3), vget_high_f64(vecMagnitude));
	}
	float64x1_t vecMagnituden = vmul_f64(vadd_f64(vget_low_f64(vecMagnitude), vget_high_f64(vecMagnitude)), vecOneOverNumPoints); // horiz add and result in first double
	vecMagnituden = vdiv_f64(vecSqrt2, vecMagnituden);

	vst1_f64(tx1, vecTxn); // vecTxn = vget_low_f64(vecTx)
	vst1_f64(ty1, vecTyn); // vecTyn = vget_low_f64(vecTy)
	vst1_f64(s1, vecMagnituden);
}

// "numPoints = 4" -> Very common (Homography)
void CompVMathStatsNormalize2DHartley_4_64f_Intrin_NEON64(const COMPV_ALIGNED(NEON) compv_float64_t* x, const COMPV_ALIGNED(NEON) compv_float64_t* y, compv_uscalar_t numPoints, compv_float64_t* tx1, compv_float64_t* ty1, compv_float64_t* s1)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	const float64x1_t vecOneOverNumPoints = vdup_n_f64(static_cast<compv_float64_t>(1.) / static_cast<compv_float64_t>(numPoints));
	const float64x1_t vecSqrt2 = vdup_n_f64(COMPV_MATH_SQRT_2);

	float64x2_t vecTx = vaddq_f64(vld1q_f64(&x[0]), vld1q_f64(&x[2]));
	float64x2_t vecTy = vaddq_f64(vld1q_f64(&y[0]), vld1q_f64(&y[2]));
	const float64x1_t vecTxn = vmul_f64(vadd_f64(vget_low_f64(vecTx), vget_high_f64(vecTx)), vecOneOverNumPoints); // horiz add and result in first double
	const float64x1_t vecTyn = vmul_f64(vadd_f64(vget_low_f64(vecTy), vget_high_f64(vecTy)), vecOneOverNumPoints); // horiz add and result in first double
	vecTx = vcombine_f64(vecTxn, vecTxn); // duplicate vecTxn
	vecTy = vcombine_f64(vecTyn, vecTyn); // duplicate vecTyn

	float64x2_t vec0 = vsubq_f64(vld1q_f64(&x[0]), vecTx);
	float64x2_t vec1 = vsubq_f64(vld1q_f64(&y[0]), vecTy);
	float64x2_t vecMagnitude = vsqrtq_f64(vaddq_f64(vmulq_f64(vec0, vec0), vmulq_f64(vec1, vec1)));
	vec0 = vsubq_f64(vld1q_f64(&x[2]), vecTx);
	vec1 = vsubq_f64(vld1q_f64(&y[2]), vecTy);
	float64x2_t vec2 = vsqrtq_f64(vaddq_f64(vmulq_f64(vec0, vec0), vmulq_f64(vec1, vec1)));
	vecMagnitude = vaddq_f64(vec2, vecMagnitude);
	float64x1_t vecMagnituden = vmul_f64(vadd_f64(vget_low_f64(vecMagnitude), vget_high_f64(vecMagnitude)), vecOneOverNumPoints); // horiz add and result in first double
	vecMagnituden = vdiv_f64(vecSqrt2, vecMagnituden);

	vst1_f64(tx1, vecTxn);
	vst1_f64(ty1, vecTyn);
	vst1_f64(s1, vecMagnituden);
}

void CompVMathStatsMSE2DHomogeneous_64f_Intrin_NEON64(const COMPV_ALIGNED(NEON) compv_float64_t* aX_h, const COMPV_ALIGNED(NEON) compv_float64_t* aY_h, const COMPV_ALIGNED(NEON) compv_float64_t* aZ_h, const COMPV_ALIGNED(NEON) compv_float64_t* bX, const COMPV_ALIGNED(NEON) compv_float64_t* bY, COMPV_ALIGNED(NEON) compv_float64_t* mse, compv_uscalar_t numPoints)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	compv_scalar_t numPointsSigned = static_cast<compv_scalar_t>(numPoints), i;
	float64x2_t vecEX0, vecEY0, vecScale0, vecEX1, vecEY1, vecScale1;
	const float64x2_t vecOne = vdupq_n_f64(1.);

	for (i = 0; i < numPointsSigned - 3; i += 4) {
		vecScale0 = vdivq_f64(vecOne, vld1q_f64(&aZ_h[i]));
		vecScale1 = vdivq_f64(vecOne, vld1q_f64(&aZ_h[i + 2]));
		vecEX0 = vsubq_f64(vmulq_f64(vld1q_f64(&aX_h[i]), vecScale0), vld1q_f64(&bX[i]));
		vecEX1 = vsubq_f64(vmulq_f64(vld1q_f64(&aX_h[i + 2]), vecScale1), vld1q_f64(&bX[i + 2]));
		vecEY0 = vsubq_f64(vmulq_f64(vld1q_f64(&aY_h[i]), vecScale0), vld1q_f64(&bY[i]));
		vecEY1 = vsubq_f64(vmulq_f64(vld1q_f64(&aY_h[i + 2]), vecScale1), vld1q_f64(&bY[i + 2]));
		vst1q_f64(&mse[i], vaddq_f64(vmulq_f64(vecEX0, vecEX0), vmulq_f64(vecEY0, vecEY0)));
		vst1q_f64(&mse[i + 2], vaddq_f64(vmulq_f64(vecEX1, vecEX1), vmulq_f64(vecEY1, vecEY1)));
	}
	if (i < numPointsSigned - 1) {
		vecScale0 = vdivq_f64(vecOne, vld1q_f64(&aZ_h[i]));
		vecEX0 = vsubq_f64(vmulq_f64(vld1q_f64(&aX_h[i]), vecScale0), vld1q_f64(&bX[i]));
		vecEY0 = vsubq_f64(vmulq_f64(vld1q_f64(&aY_h[i]), vecScale0), vld1q_f64(&bY[i]));
		vst1q_f64(&mse[i], vaddq_f64(vmulq_f64(vecEX0, vecEX0), vmulq_f64(vecEY0, vecEY0)));
		i += 2;
	}
	if (numPointsSigned & 1) {
		const float64x1_t vecScale0n = vdiv_f64(vget_low_f64(vecOne), vld1_f64(&aZ_h[i]));
		const float64x1_t vecEX0n = vsub_f64(vmul_f64(vld1_f64(&aX_h[i]), vecScale0n), vld1_f64(&bX[i]));
		const float64x1_t vecEY0n = vsub_f64(vmul_f64(vld1_f64(&aY_h[i]), vecScale0n), vld1_f64(&bY[i]));
		vst1_f64(&mse[i], vadd_f64(vmul_f64(vecEX0n, vecEX0n), vmul_f64(vecEY0n, vecEY0n)));
	}
}

// "numPoints = 4" -> Very common (Homography)
void CompVMathStatsMSE2DHomogeneous_4_64f_Intrin_NEON64(const COMPV_ALIGNED(NEON) compv_float64_t* aX_h, const COMPV_ALIGNED(NEON) compv_float64_t* aY_h, const COMPV_ALIGNED(NEON) compv_float64_t* aZ_h, const COMPV_ALIGNED(NEON) compv_float64_t* bX, const COMPV_ALIGNED(NEON) compv_float64_t* bY, COMPV_ALIGNED(NEON) compv_float64_t* mse, compv_uscalar_t numPoints)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	float64x2_t vecEX0, vecEY0, vecScale0, vecEX1, vecEY1, vecScale1;
	const float64x2_t vecOne = vdupq_n_f64(1.);

	vecScale0 = vdivq_f64(vecOne, vld1q_f64(&aZ_h[0]));
	vecScale1 = vdivq_f64(vecOne, vld1q_f64(&aZ_h[2]));
	vecEX0 = vsubq_f64(vmulq_f64(vld1q_f64(&aX_h[0]), vecScale0), vld1q_f64(&bX[0]));
	vecEX1 = vsubq_f64(vmulq_f64(vld1q_f64(&aX_h[2]), vecScale1), vld1q_f64(&bX[2]));
	vecEY0 = vsubq_f64(vmulq_f64(vld1q_f64(&aY_h[0]), vecScale0), vld1q_f64(&bY[0]));
	vecEY1 = vsubq_f64(vmulq_f64(vld1q_f64(&aY_h[2]), vecScale1), vld1q_f64(&bY[2]));
	vst1q_f64(&mse[0], vaddq_f64(vmulq_f64(vecEX0, vecEX0), vmulq_f64(vecEY0, vecEY0)));
	vst1q_f64(&mse[2], vaddq_f64(vmulq_f64(vecEX1, vecEX1), vmulq_f64(vecEY1, vecEY1)));
}

void CompVMathStatsVariance_64f_Intrin_NEON64(const COMPV_ALIGNED(NEON) compv_float64_t* data, compv_uscalar_t count, const compv_float64_t* mean1, compv_float64_t* var1)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	compv_scalar_t countSigned = static_cast<compv_scalar_t>(count), i;
	float64x2_t vecDev0, vecDev1;
	float64x2_t vecVar = vdupq_n_f64(0.);
	const float64x1_t vecCountMinus1 = vdup_n_f64(static_cast<compv_float64_t>(count - 1)); // asm: ctv(epi32, double) and no need to duplicate
	const float64x2_t vecMean = vdupq_n_f64(*mean1); // asm: load_sd() then shufle
	for (i = 0; i < countSigned - 3; i += 4) {
		vecDev0 = vsubq_f64(vld1q_f64(&data[i]), vecMean);
		vecDev1 = vsubq_f64(vld1q_f64(&data[i + 2]), vecMean);
		vecVar = vaddq_f64(vecVar, vmulq_f64(vecDev0, vecDev0));
		vecVar = vaddq_f64(vecVar, vmulq_f64(vecDev1, vecDev1));
	}
	if (i < countSigned - 1) {
		vecDev0 = vsubq_f64(vld1q_f64(&data[i]), vecMean);
		vecVar = vaddq_f64(vecVar, vmulq_f64(vecDev0, vecDev0));
		i += 2;
	}
	if (countSigned & 1) {
		const float64x1_t vecDev0n = vsub_f64(vld1_f64(&data[i]), vget_low_f64(vecMean));
		vecVar = vcombine_f64(vadd_f64(vget_low_f64(vecVar), vmul_f64(vecDev0n, vecDev0n)), vget_high_f64(vecVar));
	}
	float64x1_t vecVarn = vadd_f64(vget_low_f64(vecVar), vget_high_f64(vecVar));
	vecVarn = vdiv_f64(vecVarn, vecCountMinus1);
	vst1_f64(var1, vecVarn);
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
