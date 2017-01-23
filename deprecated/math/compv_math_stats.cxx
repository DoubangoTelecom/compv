/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/math/compv_math_stats.h"
#include "compv/math/compv_math_utils.h"
#include "compv/math/compv_math_matrix.h"
#include "compv/compv_cpu.h"

#include "compv/intrinsics/x86/math/compv_math_stats_intrin_sse2.h"
#include "compv/intrinsics/x86/math/compv_math_stats_intrin_avx.h"

#if COMPV_ARCH_X86 && COMPV_ASM
COMPV_EXTERNC void MathStatsNormalize2DHartley_64f_Asm_X86_SSE2(const COMPV_ALIGNED(SSE) compv::compv_float64_t* x, const COMPV_ALIGNED(SSE) compv::compv_float64_t* y, compv::compv_uscalar_t numPoints, compv::compv_float64_t* tx1, compv::compv_float64_t* ty1, compv::compv_float64_t* s1);
COMPV_EXTERNC void MathStatsNormalize2DHartley_64f_Asm_X86_AVX(const COMPV_ALIGNED(AVX) compv::compv_float64_t* x, const COMPV_ALIGNED(AVX) compv::compv_float64_t* y, compv::compv_uscalar_t numPoints, compv::compv_float64_t* tx1, compv::compv_float64_t* ty1, compv::compv_float64_t* s1);
COMPV_EXTERNC void MathStatsNormalize2DHartley_4_64f_Asm_X86_SSE2(const COMPV_ALIGNED(SSE) compv::compv_float64_t* x, const COMPV_ALIGNED(SSE) compv::compv_float64_t* y, compv::compv_uscalar_t numPoints, compv::compv_float64_t* tx1, compv::compv_float64_t* ty1, compv::compv_float64_t* s1);
COMPV_EXTERNC void MathStatsNormalize2DHartley_4_64f_Asm_X86_AVX(const COMPV_ALIGNED(AVX) compv::compv_float64_t* x, const COMPV_ALIGNED(AVX) compv::compv_float64_t* y, compv::compv_uscalar_t numPoints, compv::compv_float64_t* tx1, compv::compv_float64_t* ty1, compv::compv_float64_t* s1);
COMPV_EXTERNC void MathStatsMSE2DHomogeneous_64f_Asm_X86_SSE2(const COMPV_ALIGNED(SSE) compv::compv_float64_t* aX_h, const COMPV_ALIGNED(SSE) compv::compv_float64_t* aY_h, const COMPV_ALIGNED(SSE) compv::compv_float64_t* aZ_h, const COMPV_ALIGNED(SSE) compv::compv_float64_t* bX, const COMPV_ALIGNED(SSE) compv::compv_float64_t* bY, COMPV_ALIGNED(SSE) compv::compv_float64_t* mse, compv::compv_uscalar_t numPoints);
COMPV_EXTERNC void MathStatsMSE2DHomogeneous_4_64f_Asm_X86_SSE2(const COMPV_ALIGNED(SSE) compv::compv_float64_t* aX_h, const COMPV_ALIGNED(SSE) compv::compv_float64_t* aY_h, const COMPV_ALIGNED(SSE) compv::compv_float64_t* aZ_h, const COMPV_ALIGNED(SSE) compv::compv_float64_t* bX, const COMPV_ALIGNED(SSE) compv::compv_float64_t* bY, COMPV_ALIGNED(SSE) compv::compv_float64_t* mse, compv::compv_uscalar_t numPoints);
COMPV_EXTERNC void MathStatsVariance_64f_Asm_X86_SSE2(const COMPV_ALIGNED(SSE) compv::compv_float64_t* data, compv::compv_uscalar_t count, const compv::compv_float64_t* mean1, compv::compv_float64_t* var1);
COMPV_EXTERNC void MathStatsVariance_64f_Asm_X86_AVX(const COMPV_ALIGNED(AVX) compv::compv_float64_t* data, compv::compv_uscalar_t count, const compv::compv_float64_t* mean1, compv::compv_float64_t* var1);
#endif /* COMPV_ARCH_X86 && COMPV_ASM */

#if COMPV_ARCH_X64 && COMPV_ASM
COMPV_EXTERNC void MathStatsMSE2DHomogeneous_64f_Asm_X64_SSE2(const COMPV_ALIGNED(SSE) compv::compv_float64_t* aX_h, const COMPV_ALIGNED(SSE) compv::compv_float64_t* aY_h, const COMPV_ALIGNED(SSE) compv::compv_float64_t* aZ_h, const COMPV_ALIGNED(SSE) compv::compv_float64_t* bX, const COMPV_ALIGNED(SSE) compv::compv_float64_t* bY, COMPV_ALIGNED(SSE) compv::compv_float64_t* mse, compv::compv_uscalar_t numPoints);
#endif /* COMPV_ARCH_X64 && COMPV_ASM */

COMPV_NAMESPACE_BEGIN()

template class CompVMathStats<int32_t >;
template class CompVMathStats<compv_float64_t >;
template class CompVMathStats<compv_float32_t >;

/*
2D Points normalization as described by Hartley. Used before computing Homography or Fundamental matrix.
More info: https://en.wikipedia.org/wiki/Eight-point_algorithm#How_it_can_be_solved
* tx1 / ty1: The X and Y translation values to be used to build the transformation matrix.
* s1: The X and Y scaling factor to be used to build the transformation matrix.
*/
template <class T>
COMPV_ERROR_CODE CompVMathStats<T>::normalize2D_hartley(const T* x, const T* y, size_t numPoints, T* tx1, T* ty1, T* s1)
{
    COMPV_CHECK_EXP_RETURN(!x || !y || !numPoints || !tx1 || !ty1 || !s1, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

    if (std::is_same<T, compv_float64_t>::value) {
        void(*MathStatsNormalize2DHartley_64f)(const COMPV_ALIGNED(V) compv_float64_t* x, const COMPV_ALIGNED(V) compv_float64_t* y, compv_uscalar_t numPoints, compv_float64_t* tx1, compv_float64_t* ty1, compv_float64_t* s1) = NULL;
        if (CompVCpu::isEnabled(compv::kCpuFlagSSE2) && numPoints > 1 && COMPV_IS_ALIGNED_SSE(x) && COMPV_IS_ALIGNED_SSE(y)) {
            COMPV_EXEC_IFDEF_INTRIN_X86(MathStatsNormalize2DHartley_64f = MathStatsNormalize2DHartley_64f_Intrin_SSE2);
            COMPV_EXEC_IFDEF_ASM_X86(MathStatsNormalize2DHartley_64f = MathStatsNormalize2DHartley_64f_Asm_X86_SSE2);
            if (numPoints == 4) { // Homography -> very common
                COMPV_EXEC_IFDEF_INTRIN_X86(MathStatsNormalize2DHartley_64f = MathStatsNormalize2DHartley_4_64f_Intrin_SSE2);
                COMPV_EXEC_IFDEF_ASM_X86(MathStatsNormalize2DHartley_64f = MathStatsNormalize2DHartley_4_64f_Asm_X86_SSE2);
            }
        }
#if 0 // TODO(dmi): AVX code not faster than SSE
        if (CompVCpu::isEnabled(compv::kCpuFlagAVX) && numPoints > 3 && COMPV_IS_ALIGNED_AVX(x) && COMPV_IS_ALIGNED_AVX(y)) {
            COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // AVX code not faster than SSE
            COMPV_EXEC_IFDEF_INTRIN_X86(MathStatsNormalize2DHartley_64f = MathStatsNormalize2DHartley_64f_Intrin_AVX);
            COMPV_EXEC_IFDEF_ASM_X86(MathStatsNormalize2DHartley_64f = MathStatsNormalize2DHartley_64f_Asm_X86_AVX);
            if (numPoints == 4) { // Homography -> very common
                COMPV_EXEC_IFDEF_INTRIN_X86(MathStatsNormalize2DHartley_64f = MathStatsNormalize2DHartley_4_64f_Intrin_AVX);
                COMPV_EXEC_IFDEF_ASM_X86(MathStatsNormalize2DHartley_64f = MathStatsNormalize2DHartley_4_64f_Asm_X86_AVX);
            }
        }
#endif
        if (MathStatsNormalize2DHartley_64f) {
            MathStatsNormalize2DHartley_64f((const compv_float64_t*)x, (const compv_float64_t*)y, (compv_uscalar_t)numPoints, (compv_float64_t*)tx1, (compv_float64_t*)ty1, (compv_float64_t*)s1);
            return COMPV_ERROR_CODE_S_OK;
        }
    }

    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
    size_t i;
    T tx = 0, ty = 0, magnitude = 0, OneOverNumPoints = T(T(1) / numPoints), a, b;

    // Compute the centroid (https://en.wikipedia.org/wiki/Centroid#Of_a_finite_set_of_points)
    for (i = 0; i < numPoints; ++i) { // asm: unroll loop
        tx += x[i];
        ty += y[i];
    }
    tx *= OneOverNumPoints;
    ty *= OneOverNumPoints;

    // AFTER the translation the coordinates are uniformly scaled (Isotropic scaling) so that the mean distance from the origin to a point equals sqrt(2).
    // TODO(dmi): use classic normalization ((x,y)/(max_norm) € [0, 1])
    // TODO(dmi): norm(a) = sqrt(x^2 + y^2) = sqrt(dp(a, a))
    // Isotropic scaling -> scaling is invariant with respect to direction
    for (i = 0; i < numPoints; ++i) { // asm: unroll loop
        // Using naive hypot because X and Y contains point coordinates (no risk for overflow / underflow)
        a = (x[i] - tx);
        b = (y[i] - ty);
        magnitude += T(COMPV_MATH_HYPOT_NAIVE(a, b));
    }
    magnitude *= OneOverNumPoints;

    *s1 = magnitude ? T(COMPV_MATH_SQRT_2 / magnitude) : T(COMPV_MATH_SQRT_2); // (sqrt(2) / magnitude)
    *tx1 = tx;
    *ty1 = ty;

    return COMPV_ERROR_CODE_S_OK;
}

/*
Computes Mean Squared Error (MSE) / Mean Squared Deviation (MSD) between A(x,y,z) in homogeneous coordsys and B(x,y) in cartesian coordsys.
*/
template <class T>
COMPV_ERROR_CODE CompVMathStats<T>::mse2D_homogeneous(const T* aX_h, const T* aY_h, const T* aZ_h, const T* bX, const T* bY, CompVPtrArray(T)& mse, size_t numPoints)
{
    COMPV_CHECK_EXP_RETURN(!aX_h || !aY_h || !aZ_h || !bX || !bY || !numPoints, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

    if (!mse || mse->rows() != 1 || mse->cols() != numPoints) {
        COMPV_CHECK_CODE_RETURN(CompVArray<T>::newObjAligned(&mse, 1, numPoints));
    }
    T* msePtr = const_cast<T*>(mse->ptr());

    if (std::is_same<T, compv_float64_t>::value) {
        void (*MathStatsMSE2DHomogeneous_64f)(const COMPV_ALIGNED(X) compv_float64_t* aX_h, const COMPV_ALIGNED(X) compv_float64_t* aY_h, const COMPV_ALIGNED(X) compv_float64_t* aZ_h, const COMPV_ALIGNED(X) compv_float64_t* bX, const COMPV_ALIGNED(X) compv_float64_t* bY, COMPV_ALIGNED(X) compv_float64_t* mse, compv_uscalar_t numPoints) = NULL;
        if (CompVCpu::isEnabled(compv::kCpuFlagSSE2) && numPoints > 1 && COMPV_IS_ALIGNED_SSE(aX_h) && COMPV_IS_ALIGNED_SSE(aY_h) && COMPV_IS_ALIGNED_SSE(aZ_h) && COMPV_IS_ALIGNED_SSE(bX) && COMPV_IS_ALIGNED_SSE(bY) && COMPV_IS_ALIGNED_SSE(msePtr)) {
            COMPV_EXEC_IFDEF_INTRIN_X86(MathStatsMSE2DHomogeneous_64f = MathStatsMSE2DHomogeneous_64f_Intrin_SSE2);
            COMPV_EXEC_IFDEF_ASM_X86(MathStatsMSE2DHomogeneous_64f = MathStatsMSE2DHomogeneous_64f_Asm_X86_SSE2);
            COMPV_EXEC_IFDEF_ASM_X64(MathStatsMSE2DHomogeneous_64f = MathStatsMSE2DHomogeneous_64f_Asm_X64_SSE2);
            if (numPoints == 4) { // Homography -> very common (not true!!)
                COMPV_EXEC_IFDEF_INTRIN_X86(MathStatsMSE2DHomogeneous_64f = MathStatsMSE2DHomogeneous_4_64f_Intrin_SSE2);
                COMPV_EXEC_IFDEF_ASM_X86(MathStatsMSE2DHomogeneous_64f = MathStatsMSE2DHomogeneous_4_64f_Asm_X86_SSE2);
            }
        }
#if 0
        COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // AVX code not faster than SSE
        if (CompVCpu::isEnabled(compv::kCpuFlagAVX) && numPoints > 1 && COMPV_IS_ALIGNED_AVX(aX_h) && COMPV_IS_ALIGNED_AVX(aY_h) && COMPV_IS_ALIGNED_AVX(aZ_h) && COMPV_IS_ALIGNED_AVX(bX) && COMPV_IS_ALIGNED_AVX(bY) && COMPV_IS_ALIGNED_AVX(msePtr)) {
            COMPV_EXEC_IFDEF_INTRIN_X86(MathStatsMSE2DHomogeneous_64f = MathStatsMSE2DHomogeneous_64f_Intrin_AVX);
        }
#endif
        if (MathStatsMSE2DHomogeneous_64f) {
            MathStatsMSE2DHomogeneous_64f((const compv_float64_t*)aX_h, (const compv_float64_t*)aY_h, (const compv_float64_t*)aZ_h, (const compv_float64_t*)bX, (const compv_float64_t*)bY, (compv_float64_t*)msePtr, (compv_uscalar_t)numPoints);
            return COMPV_ERROR_CODE_S_OK;
        }
    }

    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
    T ex, ey, scale;

    for (size_t i = 0; i < numPoints; ++i) {
        // scale used to convert A from homogeneous to cartesian coordsys
        // z = 0 -> point a infinity (no division error is T is floating point numer)
        scale = T(1) / aZ_h[i];
        ex = (aX_h[i] * scale) - bX[i];
        ey = (aY_h[i] * scale) - bY[i];
        msePtr[i] = ((ex * ex) + (ey * ey));
    }
    return COMPV_ERROR_CODE_S_OK;
}

/*
Computes the variance: https://en.wikipedia.org/wiki/Variance
*/
template <class T>
COMPV_ERROR_CODE CompVMathStats<T>::variance(const T* data, size_t count, const T* mean1, T* var1)
{
    COMPV_CHECK_EXP_RETURN(!data || count < 2 || !mean1 || !var1, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

    if (std::is_same<T, compv_float64_t>::value) {
        void (*MathStatsVariance_64f)(const COMPV_ALIGNED(X) compv_float64_t* data, compv_uscalar_t count, const compv_float64_t* mean1, compv_float64_t* var1) = NULL;
        if (CompVCpu::isEnabled(compv::kCpuFlagSSE2) && count > 1 && COMPV_IS_ALIGNED_SSE(data)) {
            COMPV_EXEC_IFDEF_INTRIN_X86(MathStatsVariance_64f = MathStatsVariance_64f_Intrin_SSE2);
            COMPV_EXEC_IFDEF_ASM_X86(MathStatsVariance_64f = MathStatsVariance_64f_Asm_X86_SSE2);
        }
        if (CompVCpu::isEnabled(compv::kCpuFlagAVX) && count > 3 && COMPV_IS_ALIGNED_AVX(data)) {
            COMPV_EXEC_IFDEF_INTRIN_X86(MathStatsVariance_64f = MathStatsVariance_64f_Intrin_AVX);
            COMPV_EXEC_IFDEF_ASM_X86(MathStatsVariance_64f = MathStatsVariance_64f_Asm_X86_AVX);
        }
        if (MathStatsVariance_64f) {
            MathStatsVariance_64f((const compv_float64_t*)data, (compv_uscalar_t)count, (const compv_float64_t*)mean1, (compv_float64_t*)var1);
            return COMPV_ERROR_CODE_S_OK;
        }
    }

    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
    T dev, var = 0, mean = *mean1;
    for (size_t i = 0; i < count; ++i) {
        dev = (data[i] - mean);
        var += (dev * dev);
    }
    *var1 = T(var / (count - 1)); // -1 for Bessel's correction: https://en.wikipedia.org/wiki/Bessel%27s_correction
    return COMPV_ERROR_CODE_S_OK;
}

/*
Compute the standard deviation(std): https://en.wikipedia.org/wiki/Standard_deviation
std = sqrt(variance). For performance reasons we can use the varianc for comparison to save CPU cycles.
*/
template <class T>
COMPV_ERROR_CODE CompVMathStats<T>::stdev(const T* data, size_t count, const T* mean1, T* std1)
{
    T var;
    COMPV_CHECK_CODE_RETURN(CompVMathStats<T>::variance(data, count, mean1, &var));
    *std1 = T(COMPV_MATH_SQRT(var));
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
