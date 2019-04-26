/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/intrin/arm/compv_image_scale_bicubic_intrin_neon.h"

#if COMPV_ARCH_ARM && COMPV_INTRINSIC
#include "compv/base/intrin/arm/compv_intrin_neon.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

#define HERMITE1_32F_INTRIN_NEON(A, B, C, D, ttt, ret) { \
	static const float32x4_t vecCoeff0 = (float32x4_t){ -0.5f, 1.f, -0.5f, 0.0f }; \
	static const float32x4_t vecCoeff1 = (float32x4_t){ 1.5f, -2.5f, 0.f, 1.0f }; \
	static const float32x4_t vecCoeff2 = (float32x4_t){ -1.5f, 2.0f, 0.5f, 0.0f }; \
	static const float32x4_t vecCoeff3 = (float32x4_t){ 0.5f, -0.5f, 0.0f, 0.0f }; \
	const float32x4_t vec1 = vmulq_f32(B, vecCoeff1); \
	const float32x4_t vec3 = vmulq_f32(D, vecCoeff3); \
	float32x4_t vec0 = vmlaq_f32(vec1, A, vecCoeff0); \
	const float32x4_t vec2 = vmlaq_f32(vec3, C, vecCoeff2); \
	vec0 = vaddq_f32(vec0, vec2); \
	vec0 = vmulq_f32(vec0, ttt); \
	float32x2_t vec0n = vadd_f32(vget_low_f32(vec0), vget_high_f32(vec0)); \
	vec0n = vpadd_f32(vec0n, vec0n); \
	ret = vget_lane_f32(vec0n, 0); \
}

#define HERMITE4_32F_INTRIN_NEON(A, B, C, D, t, t2, t3, ret) { \
	static const float32x4_t vec05m = vdupq_n_f32(-0.5f); \
	static const float32x4_t vec15 = vdupq_n_f32(1.5f); \
	static const float32x4_t vec20 = vdupq_n_f32(2.0f); \
	static const float32x4_t vec25m = vdupq_n_f32(-2.5f); \
	ret = vmulq_f32(vsubq_f32(A, D), vec05m); \
	ret = vmlaq_f32(ret, vsubq_f32(B, C), vec15); \
	float32x4_t vec1 = vmlaq_f32(A, B, vec25m); \
	vec1 = vmlaq_f32(vec1, C, vec20); \
	vec1 = vmlaq_f32(vec1, D, vec05m); \
	float32x4_t vec2 = vmulq_f32(vsubq_f32(A, C), vec05m); \
	vec2 = vmulq_f32(vec2, t); \
	ret = vmlaq_f32(vec2, ret, t3); \
	vec1 = vmlaq_f32(B, vec1, t2); \
	ret = vaddq_f32(ret, vec1); \
}

void CompVImageScaleBicubicPreprocess_32s32f_Intrin_NEON(
	COMPV_ALIGNED(NEON) int32_t* intergral,
	COMPV_ALIGNED(NEON) compv_float32_t* fraction,
	const compv_float32_t* sv1,
	COMPV_ALIGNED(NEON) const compv_uscalar_t outSize,
	const compv_scalar_t intergralMax,
	const compv_scalar_t intergralStride
)
{
	COMPV_DEBUG_INFO_CHECK_NEON();

	// TODO(dmi): No ASM code

	static const int32x4_t vecIntegralOffset = (int32x4_t) { -1, 0, 1, 2 };
	static const int32x4_t vecZero = vdupq_n_s32(0);
	static const float32x4_t vecHalf = vdupq_n_f32(0.5f);

	const compv_uscalar_t maxI = outSize << 2;
	const compv_float32_t& sv = *sv1;
	const compv_float32_t m = (0.5f * sv);
	const int32x4_t vecIntergralMax = vdupq_n_s32(static_cast<int32_t>(intergralMax));
	const int32x4_t vecIntergralStride = vdupq_n_s32(static_cast<int32_t>(intergralStride));
	const float32x4_t vecSV4 = vdupq_n_f32(sv * 4.f);
	float32x4_t vecM = (float32x4_t) { m, m + sv, m + sv*2.f, m + sv*3.f };

	for (compv_uscalar_t i = 0; i < maxI; i += 16) {
		const float32x4_t vecFract = vsubq_f32(vecM, vecHalf);
		const float32x4_t vecIntegralf = COMPV_ARM_NEON_FLOOR_F32(vecFract); // SSE2: _mm_round_ps(vecFract, _MM_FROUND_FLOOR);
		const int32x4_t vecIntegrali = vcvtq_s32_f32(vecIntegralf);

		int32x4_t vecIntegrali0 = vaddq_s32(vdupq_lane_s32(vget_low_f32(vecIntegrali), 0), vecIntegralOffset);
		int32x4_t vecIntegrali1 = vaddq_s32(vdupq_lane_s32(vget_low_f32(vecIntegrali), 1), vecIntegralOffset);
		int32x4_t vecIntegrali2 = vaddq_s32(vdupq_lane_s32(vget_high_f32(vecIntegrali), 0), vecIntegralOffset);
		int32x4_t vecIntegrali3 = vaddq_s32(vdupq_lane_s32(vget_high_f32(vecIntegrali), 1), vecIntegralOffset);
		vecIntegrali0 = vmaxq_s32(vecZero, vminq_s32(vecIntegrali0, vecIntergralMax));
		vecIntegrali1 = vmaxq_s32(vecZero, vminq_s32(vecIntegrali1, vecIntergralMax));
		vecIntegrali2 = vmaxq_s32(vecZero, vminq_s32(vecIntegrali2, vecIntergralMax));
		vecIntegrali3 = vmaxq_s32(vecZero, vminq_s32(vecIntegrali3, vecIntergralMax));
		vecIntegrali0 = vmulq_s32(vecIntegrali0, vecIntergralStride);
		vecIntegrali1 = vmulq_s32(vecIntegrali1, vecIntergralStride);
		vecIntegrali2 = vmulq_s32(vecIntegrali2, vecIntergralStride);
		vecIntegrali3 = vmulq_s32(vecIntegrali3, vecIntergralStride);

		float32x4_t vecFraction = vsubq_f32(vecFract, vecIntegralf);
		float32x4_t vecFraction2 = vmulq_f32(vecFraction, vecFraction);
		float32x4_t vecFraction3 = vmulq_f32(vecFraction2, vecFraction);
		float32x4_t vecOne = vdupq_n_f32(1.f);
		COMPV_ARM_NEON_TRANSPOSE4x4_32(vecFraction3, vecFraction2, vecFraction, vecOne);

		vst1q_s32(&intergral[i + 0], vecIntegrali0);
		vst1q_s32(&intergral[i + 4], vecIntegrali1);
		vst1q_s32(&intergral[i + 8], vecIntegrali2);
		vst1q_s32(&intergral[i + 12], vecIntegrali3);

		vst1q_f32(&fraction[i + 0], vecFraction3);
		vst1q_f32(&fraction[i + 4], vecFraction2);
		vst1q_f32(&fraction[i + 8], vecFraction);
		vst1q_f32(&fraction[i + 12], vecOne);

		vecM = vaddq_f32(vecM, vecSV4);
	}
}

void CompVImageScaleBicubicPostProcessRow_32f32s_Intrin_NEON(
	compv_float32_t* outPtr,
	const compv_float32_t* inPtr,
	COMPV_ALIGNED(NEON) const int32_t* xint4,
	COMPV_ALIGNED(NEON) const compv_float32_t* xfract4,
	COMPV_ALIGNED(NEON) const int32_t* yint4,
	COMPV_ALIGNED(NEON) const compv_float32_t* yfract4,
	const compv_uscalar_t rowCount
)
{
	COMPV_DEBUG_INFO_CHECK_NEON();

	// TODO(dmi): No ASM code

	const compv_float32_t* p0 = &inPtr[yint4[0]];
	const compv_float32_t* p1 = &inPtr[yint4[1]];
	const compv_float32_t* p2 = &inPtr[yint4[2]];
	const compv_float32_t* p3 = &inPtr[yint4[3]];

	float32x4_t AA, BB, CC, DD, EE;
	const float32x4_t yfract = vld1q_f32(yfract4);

	for (compv_uscalar_t i = 0; i < rowCount; ++i, xint4 += 4, xfract4 += 4) {
		const int32_t& x0 = xint4[0];
		const int32_t& x3 = xint4[3];		
		
		if ((x3 - x0) == 3) {
			AA = vld1q_f32(&p0[x0]);
			BB = vld1q_f32(&p1[x0]);
			CC = vld1q_f32(&p2[x0]);
			DD = vld1q_f32(&p3[x0]);
			COMPV_ARM_NEON_TRANSPOSE4x4_32(AA, BB, CC, DD);
		}
		else {
			// TODO(dmi): use shufle
			const int32_t& x1 = xint4[1];
			const int32_t& x2 = xint4[2];
			AA = (float32x4_t){ p0[x0], p1[x0], p2[x0], p3[x0] };
			BB = (float32x4_t) { p0[x1], p1[x1], p2[x1], p3[x1] };
			CC = (float32x4_t) { p0[x2], p1[x2], p2[x2], p3[x2] };
			DD = (float32x4_t) { p0[x3], p1[x3], p2[x3], p3[x3] };
		}

		const float32x4_t xfract = vld1q_f32(xfract4);
		const float32x4_t xfract3 = vdupq_lane_f32(vget_low_f32(xfract), 0);
		const float32x4_t xfract2 = vdupq_lane_f32(vget_low_f32(xfract), 1);
		const float32x4_t xfract1 = vdupq_lane_f32(vget_high_f32(xfract), 0);

		HERMITE4_32F_INTRIN_NEON(
			AA, BB, CC, DD,
			xfract1, xfract2, xfract3,
			EE
		);
		HERMITE1_32F_INTRIN_NEON(
			vdupq_lane_f32(vget_low_f32(EE), 0),
			vdupq_lane_f32(vget_low_f32(EE), 1),
			vdupq_lane_f32(vget_high_f32(EE), 0),
			vdupq_lane_f32(vget_high_f32(EE), 1),
			yfract,
			outPtr[i]
		);
	}
}

void CompVImageScaleBicubicHermite_32f32s_Intrin_NEON(
	compv_float32_t* outPtr,
	const compv_float32_t* inPtr,
	const int32_t* xint1,
	const compv_float32_t* xfract1,
	const int32_t* yint1,
	const compv_float32_t* yfract1,
	const compv_uscalar_t inWidthMinus1,
	const compv_uscalar_t inHeightMinus1,
	const compv_uscalar_t inStride
)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
#if 0
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No ASM implementation");
#endif
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("For ultimate projects, do not use this function -> re-design remap()");

	static const int32x4_t vecZero = vdupq_n_s32(0);
	static const int32x4_t vecOffset = (int32x4_t) { -1, 0, 1, 2 };

	// Add offsets (-1, 0, 1, 2)
	int32x4_t vecX = vaddq_s32(vdupq_n_s32(*xint1), vecOffset);
	int32x4_t vecY = vaddq_s32(vdupq_n_s32(*yint1), vecOffset);

	// a = COMPV_MATH_CLIP3(0, size-1, a)
	vecX = vmaxq_s32(vecZero, vminq_s32(vecX, vdupq_n_s32(static_cast<int32_t>(inWidthMinus1))));
	vecY = vmaxq_s32(vecZero, vminq_s32(vecY, vdupq_n_s32(static_cast<int32_t>(inHeightMinus1))));

	// Y = Y * stride
	vecY = vmulq_s32(vecY, vdupq_n_s32(static_cast<int32_t>(inStride)));

	// Index[i] = Y[i] + X
	const int32x4_t vecIdx0 = vaddq_s32(vecY, vdupq_lane_s32(vget_low_s32(vecX), 0));
	const int32x4_t vecIdx1 = vaddq_s32(vecY, vdupq_lane_s32(vget_low_s32(vecX), 1));
	const int32x4_t vecIdx2 = vaddq_s32(vecY, vdupq_lane_s32(vget_high_s32(vecX), 0));
	const int32x4_t vecIdx3 = vaddq_s32(vecY, vdupq_lane_s32(vget_high_s32(vecX), 1));

	const float32x4_t xfract = vdupq_n_f32(*xfract1);
	const float32x4_t xfract2 = vmulq_f32(xfract, xfract);
	const float32x4_t xfract3 = vmulq_f32(xfract2, xfract);

	const compv_float32_t& yfract1_ = *yfract1;
	const compv_float32_t yfract2_ = yfract1_ * yfract1_;
	const float32x4_t yfract = (float32x4_t) { (yfract2_ * yfract1_), yfract2_, yfract1_, 1.f };

	// TODO(dmi): use SVE extension (gather) -> https://community.arm.com/processors/b/blog/posts/technology-update-the-scalable-vector-extension-sve-for-the-armv8-a-architecture
	const float32x4_t AA = (float32x4_t) { inPtr[vgetq_lane_s32(vecIdx0, 0)], inPtr[vgetq_lane_s32(vecIdx0, 1)], inPtr[vgetq_lane_s32(vecIdx0, 2)], inPtr[vgetq_lane_s32(vecIdx0, 3)] };
	const float32x4_t BB = (float32x4_t) { inPtr[vgetq_lane_s32(vecIdx1, 0)], inPtr[vgetq_lane_s32(vecIdx1, 1)], inPtr[vgetq_lane_s32(vecIdx1, 2)], inPtr[vgetq_lane_s32(vecIdx1, 3)] };
	const float32x4_t CC = (float32x4_t) { inPtr[vgetq_lane_s32(vecIdx2, 0)], inPtr[vgetq_lane_s32(vecIdx2, 1)], inPtr[vgetq_lane_s32(vecIdx2, 2)], inPtr[vgetq_lane_s32(vecIdx2, 3)] };
	const float32x4_t DD = (float32x4_t) { inPtr[vgetq_lane_s32(vecIdx3, 0)], inPtr[vgetq_lane_s32(vecIdx3, 1)], inPtr[vgetq_lane_s32(vecIdx3, 2)], inPtr[vgetq_lane_s32(vecIdx3, 3)] };
	float32x4_t EE;
	HERMITE4_32F_INTRIN_NEON(
		AA, BB, CC, DD,
		xfract, xfract2, xfract3,
		EE
	);
	HERMITE1_32F_INTRIN_NEON(
		vdupq_lane_f32(vget_low_f32(EE), 0),
		vdupq_lane_f32(vget_low_f32(EE), 1),
		vdupq_lane_f32(vget_high_f32(EE), 0),
		vdupq_lane_f32(vget_high_f32(EE), 1),
		yfract,
		*outPtr
	);
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
