/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/arm/compv_math_trig_intrin_neon.h"

#if COMPV_ARCH_ARM && COMPV_INTRINSIC
#include "compv/base/compv_simd_globals.h"
#include "compv/base/intrin/arm/compv_intrin_neon.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_mem.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

void CompVMathTrigFastAtan2_32f_Intrin_NEON(COMPV_ALIGNED(NEON) const compv_float32_t* y, COMPV_ALIGNED(NEON) const compv_float32_t* x, COMPV_ALIGNED(NEON) compv_float32_t* r, const compv_float32_t* scale1, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride)
{
    COMPV_DEBUG_INFO_CHECK_NEON();
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("ASM FMA version is faster");
#if COMPV_ARCH_ARM32
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("vdivq_f32 not optimized");
#endif
    static const float32x4_t vecAtan2_eps = vld1q_f32(kAtan2Eps_32f);
    static const float32x4_t vecAtan2_p1 = vld1q_f32(kAtan2P1_32f);
    static const float32x4_t vecAtan2_p3 = vld1q_f32(kAtan2P3_32f);
    static const float32x4_t vecAtan2_p5 = vld1q_f32(kAtan2P5_32f);
    static const float32x4_t vecAtan2_p7 = vld1q_f32(kAtan2P7_32f);
    static const float32x4_t vecAtan2_zero = vdupq_n_f32(0.f);
    static const float32x4_t vecAtan2_plus90 = vld1q_f32(k90_32f);
    static const float32x4_t vecAtan2_plus180 = vld1q_f32(k180_32f);
    static const float32x4_t vecAtan2_plus360 = vld1q_f32(k360_32f);
    const float32x4_t vecAtan2_scale = vdupq_n_f32(*scale1);
    for (compv_uscalar_t j = 0; j < height; ++j) {
        for (compv_uscalar_t i = 0; i < width; i += 4) {
            // ax = std::abs(x[i]), ay = std::abs(y[i]);
            float32x4_t vecAx = vabsq_f32(vld1q_f32(&x[i]));
            float32x4_t vecAy = vabsq_f32(vld1q_f32(&y[i]));
            
            // if (ax >= ay) vec1 = ay, vec2 = ax;
            // else vec1 = ax, vec2 = ay;
            float32x4_t vecMask = vcgeq_f32(vecAx, vecAy);
            float32x4_t vec1 = vandq_s32(vecAy, vecMask);
            float32x4_t vec2 = vandq_s32(vecAx, vecMask);
            vec1 = vorrq_s32(vec1, vbicq_s32(vecAx, vecMask));
            vec2 = vorrq_s32(vec2, vbicq_s32(vecAy, vecMask));
            
            // c = vec1 / (vec2 + atan2_eps)
            // c2 = c*c
            const float32x4_t vecC = vdivq_f32(vec1, vaddq_f32(vec2, vecAtan2_eps));
            const float32x4_t vecC2 = vmulq_f32(vecC, vecC);
            
            // a = (((atan2_p7*c2 + atan2_p5)*c2 + atan2_p3)*c2 + atan2_p1)*c
            float32x4_t vec0 = vmlaq_f32(vecAtan2_p5, vecAtan2_p7, vecC2); // TODO(dmi): AVX/NEON: Use fusedMulAdd
            vec0 = vmlaq_f32(vecAtan2_p3, vec0, vecC2); // TODO(dmi): AVX/NEON: Use fusedMulAdd
            vec0 = vmlaq_f32(vecAtan2_p1, vec0, vecC2); // TODO(dmi): AVX/NEON: Use fusedMulAdd
            vec0 = vmulq_f32(vec0, vecC);
            
            // if (!(ax >= ay)) a = 90 - a
            vec1 = vbicq_s32(vsubq_f32(vecAtan2_plus90, vec0), vecMask);
            vec0 = vorrq_s32(vandq_s32(vec0, vecMask), vec1);
            
            // if (x[i] < 0) a = 180.f - a
            vecMask = vcltq_f32(vld1q_f32(&x[i]), vecAtan2_zero);
            vec1 = vandq_s32(vecMask, vsubq_f32(vecAtan2_plus180, vec0));
            vec0 = vorrq_s32(vbicq_s32(vec0, vecMask), vec1);
            
            // if (y[i + k] < 0) a = 360.f - a
            vecMask = vcltq_f32(vld1q_f32(&y[i]), vecAtan2_zero);
            vec1 = vandq_s32(vecMask, vsubq_f32(vecAtan2_plus360, vec0));
            vec0 = vorrq_s32(vbicq_s32(vec0, vecMask), vec1);
            
            // r[i] = a * scale
            vst1q_f32(&r[i], vmulq_f32(vec0, vecAtan2_scale));
        }
        y += stride;
        x += stride;
        r += stride;
    }
}

void CompVMathTrigHypotNaive_32f_Intrin_NEON(COMPV_ALIGNED(NEON) const compv_float32_t* x, COMPV_ALIGNED(NEON) const compv_float32_t* y, COMPV_ALIGNED(NEON) compv_float32_t* r, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride)
{
    COMPV_DEBUG_INFO_CHECK_NEON();
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("ASM FMA version is faster");
#if COMPV_ARCH_ARM32
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("vsqrtq_f32 not optimized");
#endif
    const compv_uscalar_t width16 = width & -16;
    compv_uscalar_t i;
    float32x4_t vec0, vec1, vec2, vec3, vec4, vec5, vec6, vec7;
    
    for (compv_uscalar_t j = 0; j < height; ++j) {
        for (i = 0; i < width16; i += 16) {
            vec0 = vld1q_f32(&x[i]);
            vec1 = vld1q_f32(&x[i + 4]);
            vec2 = vld1q_f32(&x[i + 8]);
            vec3 = vld1q_f32(&x[i + 12]);
            vec4 = vld1q_f32(&y[i]);
            vec5 = vld1q_f32(&y[i + 4]);
            vec6 = vld1q_f32(&y[i + 8]);
            vec7 = vld1q_f32(&y[i + 12]);
            vst1q_f32(&r[i], vsqrtq_f32(vmlaq_f32(vmulq_f32(vec0, vec0), vec4, vec4))); // TODO(dmi): Add support for FMA3 (see ASM code)
            vst1q_f32(&r[i + 4], vsqrtq_f32(vmlaq_f32(vmulq_f32(vec1, vec1), vec5, vec5))); // TODO(dmi): Add support for FMA3 (see ASM code)
            vst1q_f32(&r[i + 8], vsqrtq_f32(vmlaq_f32(vmulq_f32(vec2, vec2), vec6, vec6))); // TODO(dmi): Add support for FMA3 (see ASM code)
            vst1q_f32(&r[i + 12], vsqrtq_f32(vmlaq_f32(vmulq_f32(vec3, vec3), vec7, vec7))); // TODO(dmi): Add support for FMA3 (see ASM code)
        }
        for (; i < width; i += 4) {
            vec0 = vld1q_f32(&x[i]);
            vec4 = vld1q_f32(&y[i]);
            vst1q_f32(&r[i], vsqrtq_f32(vmlaq_f32(vmulq_f32(vec0, vec0), vec4, vec4))); // TODO(dmi): Add support for FMA3 (see ASM code)
        }
        y += stride;
        x += stride;
        r += stride;
    }
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_ARM && COMPV_INTRINSIC */
