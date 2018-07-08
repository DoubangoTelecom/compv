/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/arm/compv_math_exp_intrin_neon.h"

#if COMPV_ARCH_ARM && COMPV_INTRINSIC
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

#if COMPV_ARCH_ARM64
// Must not require memory alignment (random access from SVM)
void CompVMathExpExp_minpack2_64f64f_Intrin_NEON64(const compv_float64_t* ptrIn, compv_float64_t* ptrOut, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride, const uint64_t* lut64u, const uint64_t* var64u, const compv_float64_t* var64f)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	COMPV_DEBUG_INFO_CODE_TODO("ASM code faster");

	const uint64x2_t vecMask = vdupq_n_u64(var64u[0]);
	const uint64x2_t vecCADJ = vdupq_n_u64(var64u[1]);

	const float64x2_t vecB = vdupq_n_f64(var64f[0]);
	const float64x2_t vecCA = vdupq_n_f64(var64f[1]);
	const float64x2_t vecCRA = vdupq_n_f64(var64f[2]);
	const float64x2_t vecC10 = vdupq_n_f64(var64f[3]);
	const float64x2_t vecC20 = vdupq_n_f64(var64f[4]);
	const float64x2_t vecC30 = vdupq_n_f64(var64f[5]);
	const float64x2_t vecMin = vdupq_n_f64(var64f[6]);
	const float64x2_t vecMax = vdupq_n_f64(var64f[7]);

	const compv_uscalar_t width2 = width & -2;

	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width2; i += 2) {
			float64x2_t vecX = vminq_f64(vld1q_f64(&ptrIn[i]), vecMax);
			vecX = vmaxq_f64(vecX, vecMin);
			float64x2_t vecDI = vmlaq_f64(vecB, vecX, vecCA); // VFMA [vecB + (vecX * vecCA)]
			const float64x2_t vecT = vmlsq_f64(vecX, vsubq_f64(vecDI, vecB), vecCRA); // VFMA [vecX - (sub * vecCRA)]
			uint64x2_t vecU = vshlq_n_u64(vshrq_n_u64(vaddq_u64(vecDI, vecCADJ), 11), 52);
			float64x2_t vecY = vmulq_f64(vecT, vecT);
			vecDI = vandq_u64(vecDI, vecMask);
			vecY = vmulq_f64(vecY,  vaddq_f64(vecC30, vecT));
			// TODO(dmi): use gather to compute "vecLUT"
			const uint64_t i0 = vgetq_lane_u64(vecDI, 0);
			const uint64_t i1 = vgetq_lane_u64(vecDI, 1);
#if ((defined(_DEBUG) && _DEBUG != 0) || (defined(DEBUG) && DEBUG != 0))
			COMPV_ASSERT(i0 >= 0 && i0 < 2048 && i1 >= 0 && i1 < 2048);
#endif
			const uint64x2_t vecLUT = vcombine_u64((uint64x1_t)lut64u[i0], (uint64x1_t)lut64u[i1]);
			vecU = vorrq_u64(vecU, vecLUT);

			vecY = vmlaq_f64(vecT, vecY, vecC20); // VFMA [vecT + (vecY * vecC20)]
			vecY = vaddq_f64(vecC10, vecY);
			vst1q_f64(&ptrOut[i], vmulq_f64(vecY, vecU));
		}
		ptrIn += stride;
		ptrOut += stride;
	}
}
#endif /* COMPV_ARCH_ARM64 */

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
