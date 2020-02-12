/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/features/hog/intrin/arm/compv_core_feature_hog_common_norm_intrin_neon.h"

#if COMPV_ARCH_ARM && COMPV_INTRINSIC
#include "compv/base/intrin/arm/compv_intrin_neon.h"
#include "compv/base/compv_debug.h"

#include <cmath> /* std::sqrt */

COMPV_NAMESPACE_BEGIN()

void CompVHogCommonNormL1_32f_Intrin_NEON(compv_float32_t* inOutPtr, const compv_float32_t* eps1, const compv_uscalar_t count)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No ASM code");
    if (count == 9) {
        COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("There is a faster version");
    }
	const compv_uscalar_t count8 = count & -8; // count equal #9 is very common -> use count8 instead of count16
	const compv_uscalar_t count4 = count & -4;
	compv_uscalar_t i;
	
	float32x4_t vec0 = vdupq_n_f32(0.f);
	float32x4_t vec1 = vdupq_n_f32(0.f);

	for (i = 0; i < count8; i += 8) {
		// no need for abs because hists are always >= 0
		vec0 = vaddq_f32(vec0, vld1q_f32(&inOutPtr[i]));
		vec1 = vaddq_f32(vec1, vld1q_f32(&inOutPtr[i + 4]));
	}
	for (; i < count4; i += 4) {
		vec0 = vaddq_f32(vec0, vld1q_f32(&inOutPtr[i]));
	}
    vec0 = vaddq_f32(vec0, vec1);
    float32x2_t vec0n = vadd_f32(vget_low_f32(vec0), vget_high_f32(vec0));
    vec0n = vpadd_f32(vec0n, vec0n);
    compv_float32_t vv = vget_lane_f32(vec0n, 0);
	for (; i < count; i += 1) {
		vv += inOutPtr[i];
	}
	vv += *eps1;
    
#if 0 // TODO(dmi): use RCP instead of 1/den
	vec0 = _mm_rcp_ss(vec0);
#else
    const compv_float32_t vv0 = 1.f / vv;
    vec0 = vdupq_n_f32(vv0);
#endif

	// Compute norm = v * (1 / den)
	for (i = 0; i < count8; i += 8) {
		vst1q_f32(&inOutPtr[i], vmulq_f32(vec0, vld1q_f32(&inOutPtr[i])));
		vst1q_f32(&inOutPtr[i + 4], vmulq_f32(vec0, vld1q_f32(&inOutPtr[i + 4])));
	}
	for (; i < count4; i += 4) {
		vst1q_f32(&inOutPtr[i], vmulq_f32(vec0, vld1q_f32(&inOutPtr[i])));
	}
	for (; i < count; ++i) {
		inOutPtr[i] *= vv0;
	}
}

void CompVHogCommonNormL1_9_32f_Intrin_NEON(compv_float32_t* inOutPtr, const compv_float32_t* eps1, const compv_uscalar_t count)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("ASM code faster");
    const float32x4_t veca = vld1q_f32(&inOutPtr[0]);
    const float32x4_t vecb = vld1q_f32(&inOutPtr[4]);
    const compv_float32_t& vvb = inOutPtr[8];
	float32x4_t vec0 = vaddq_f32(veca, vecb);
    float32x2_t vec0n = vadd_f32(vget_low_f32(vec0), vget_high_f32(vec0));
    vec0n = vpadd_f32(vec0n, vec0n);
    const compv_float32_t vv = vget_lane_f32(vec0n, 0) + vvb + *eps1;
    
#if 0 // TODO(dmi): use RCP instead of 1/den - http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0491i/CIHBIIBG.html
	vec0 = vrecpsq_f32(vec0);
#else
    const compv_float32_t vv0 = 1.f / vv;
	vec0 = vdupq_n_f32(vv0);
#endif
    
	vst1q_f32(&inOutPtr[0], vmulq_f32(vec0, veca));
	vst1q_f32(&inOutPtr[4], vmulq_f32(vec0, vecb));
	inOutPtr[8] = (vv0 * vvb);
}

void CompVHogCommonNormL1Sqrt_32f_Intrin_NEON(compv_float32_t* inOutPtr, const compv_float32_t* eps1, const compv_uscalar_t count)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No ASM code");
#if COMPV_ARCH_ARM32
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("vsqrtq_f32 not optimised");
#endif
	CompVHogCommonNormL1_32f_Intrin_NEON(inOutPtr, eps1, count);
	const compv_uscalar_t count8 = count & -8; // count equal #9 is very common -> use count8 instead of count16
	const compv_uscalar_t count4 = count & -4;
	compv_uscalar_t i;
	for (i = 0; i < count8; i += 8) {
		vst1q_f32(&inOutPtr[i], vsqrtq_f32(vld1q_f32(&inOutPtr[i])));
		vst1q_f32(&inOutPtr[i + 4], vsqrtq_f32(vld1q_f32(&inOutPtr[i + 4])));
	}
	for (; i < count4; i += 4) {
		vst1q_f32(&inOutPtr[i], vsqrtq_f32(vld1q_f32(&inOutPtr[i])));
	}
	for (; i < count; i += 1) {
        inOutPtr[i] = std::sqrt(inOutPtr[i]);
	}
}

void CompVHogCommonNormL1Sqrt_9_32f_Intrin_NEON(compv_float32_t* inOutPtr, const compv_float32_t* eps1, const compv_uscalar_t count)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("ASM code faster");
	CompVHogCommonNormL1_9_32f_Intrin_NEON(inOutPtr, eps1, count);
	vst1q_f32(&inOutPtr[0], vsqrtq_f32(vld1q_f32(&inOutPtr[0])));
	vst1q_f32(&inOutPtr[4], vsqrtq_f32(vld1q_f32(&inOutPtr[4])));
    inOutPtr[8] = std::sqrt(inOutPtr[8]);
}

void CompVHogCommonNormL2_32f_Intrin_NEON(compv_float32_t* inOutPtr, const compv_float32_t* eps_square1, const compv_uscalar_t count)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No ASM code");
    if (count == 9) {
        COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("There is a faster version");
    }
    
	const compv_uscalar_t count8 = count & -8; // count equal #9 is very common -> use count8 instead of count16
	const compv_uscalar_t count4 = count & -4;
	compv_uscalar_t i;

    float32x4_t vec0 = vdupq_n_f32(0.f);
    float32x4_t vec1 = vdupq_n_f32(0.f);

	for (i = 0; i < count8; i += 8) {
		const float32x4_t vec2 = vld1q_f32(&inOutPtr[i]);
		const float32x4_t vec3 = vld1q_f32(&inOutPtr[i + 4]);
		vec0 = vaddq_f32(vec0, vmulq_f32(vec2, vec2));
		vec1 = vaddq_f32(vec1, vmulq_f32(vec3, vec3));
	}
	for (; i < count4; i += 4) {
		const float32x4_t vec2 = vld1q_f32(&inOutPtr[i]);
		vec0 = vaddq_f32(vec0, vmulq_f32(vec2, vec2));
	}
    vec0 = vaddq_f32(vec0, vec1);
    float32x2_t vec0n = vadd_f32(vget_low_f32(vec0), vget_high_f32(vec0));
    vec0n = vpadd_f32(vec0n, vec0n);
    compv_float32_t vv = vget_lane_f32(vec0n, 0);
	for (; i < count; i += 1) {
		const compv_float32_t& vvb = inOutPtr[i];
		vv += (vvb * vvb);
	}
	vv += *eps_square1;
    
#if 0 // TODO(dmi): use RSQRT instead of SQRT followed by DIV
	vec0 = _mm_rsqrt_ss(vec0);
#else
    const compv_float32_t vv0 = 1.f / std::sqrt(vv);
    vec0 = vdupq_n_f32(vv0);
#endif

	for (i = 0; i < count8; i += 8) {
		vst1q_f32(&inOutPtr[i], vmulq_f32(vec0, vld1q_f32(&inOutPtr[i])));
		vst1q_f32(&inOutPtr[i + 4], vmulq_f32(vec0, vld1q_f32(&inOutPtr[i + 4])));
	}
	for (; i < count4; i += 4) {
		vst1q_f32(&inOutPtr[i], vmulq_f32(vec0, vld1q_f32(&inOutPtr[i])));
	}
	for (; i < count; i += 1) {
		inOutPtr[i] *= vv0;
	}
}

void CompVHogCommonNormL2_9_32f_Intrin_NEON(compv_float32_t* inOutPtr, const compv_float32_t* eps_square1, const compv_uscalar_t count)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("ASM code faster");
    const float32x4_t veca = vld1q_f32(&inOutPtr[0]);
    const float32x4_t vecb = vld1q_f32(&inOutPtr[4]);
    const compv_float32_t& vvb = inOutPtr[8];
    const compv_float32_t vvb2 = vvb * vvb;
	float32x4_t vec0 = vmulq_f32(veca, veca);
	const float32x4_t vec1 = vmulq_f32(vecb, vecb);
    vec0 = vaddq_f32(vec0, vec1);
    float32x2_t vec0n = vadd_f32(vget_low_f32(vec0), vget_high_f32(vec0));
    vec0n = vpadd_f32(vec0n, vec0n);
    const compv_float32_t vv = vget_lane_f32(vec0n, 0) + vvb2 + *eps_square1;
    
    
#if 0 // TODO(dmi): use RSQRT instead of SQRT followed by DIV
	vec0 = _mm_rsqrt_ss(vec0);
#else
    const compv_float32_t vv0 = 1.f / std::sqrt(vv);
    vec0 = vdupq_n_f32(vv0);
#endif
    
    vst1q_f32(&inOutPtr[0], vmulq_f32(vec0, veca));
    vst1q_f32(&inOutPtr[4], vmulq_f32(vec0, vecb));
    inOutPtr[8] = (vv0 * vvb);
}

void CompVHogCommonNormL2Hys_32f_Intrin_NEON(compv_float32_t* inOutPtr, const compv_float32_t* eps_square1, const compv_uscalar_t count)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No ASM code");
    if (count == 9) {
        COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("There is a faster version");
    }
	static const float32x4_t vecMax = vdupq_n_f32(0.2f);
	CompVHogCommonNormL2_32f_Intrin_NEON(inOutPtr, eps_square1, count);
	const compv_uscalar_t count8 = count & -8; // count equal #9 is very common -> use count8 instead of count16
	const compv_uscalar_t count4 = count & -4;
	compv_uscalar_t i;
	for (i = 0; i < count8; i += 8) {
		vst1q_f32(&inOutPtr[i], vminq_f32(vld1q_f32(&inOutPtr[i]), vecMax));
		vst1q_f32(&inOutPtr[i + 4], vminq_f32(vld1q_f32(&inOutPtr[i + 4]), vecMax));
	}
	for (; i < count4; i += 4) {
		vst1q_f32(&inOutPtr[i], vminq_f32(vld1q_f32(&inOutPtr[i]), vecMax));
	}
	for (; i < count; ++i) {
        inOutPtr[i] = std::min(inOutPtr[i], 0.2f);
	}
	CompVHogCommonNormL2_32f_Intrin_NEON(inOutPtr, eps_square1, count);
}

void CompVHogCommonNormL2Hys_9_32f_Intrin_NEON(compv_float32_t* inOutPtr, const compv_float32_t* eps_square1, const compv_uscalar_t count)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("ASM code faster");
	static const float32x4_t vecMax = vdupq_n_f32(0.2f);
	CompVHogCommonNormL2_9_32f_Intrin_NEON(inOutPtr, eps_square1, count);
	vst1q_f32(&inOutPtr[0], vminq_f32(vld1q_f32(&inOutPtr[0]), vecMax));
	vst1q_f32(&inOutPtr[4], vminq_f32(vld1q_f32(&inOutPtr[4]), vecMax));
    inOutPtr[8] = std::min(inOutPtr[8], 0.2f);
	CompVHogCommonNormL2_9_32f_Intrin_NEON(inOutPtr, eps_square1, count);
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_ARM && COMPV_INTRINSIC */
