/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/compv_math_stats.h"
#include "compv/base/math/compv_math_utils.h"
#include "compv/base/math/compv_math.h"

#include "compv/base/math/intrin/x86/compv_math_stats_intrin_sse2.h"
#include "compv/base/math/intrin/x86/compv_math_stats_intrin_avx.h"
#include "compv/base/math/intrin/arm/compv_math_stats_intrin_neon64.h"

#include <float.h> /* DBL_EPSILON */

#define COMPV_THIS_CLASSNAME	"CompVMathStats"

COMPV_NAMESPACE_BEGIN()

#if COMPV_ASM
#	if COMPV_ARCH_X86
	COMPV_EXTERNC void CompVMathStatsNormalize2DHartley_64f_Asm_X86_SSE2(const COMPV_ALIGNED(SSE) compv_float64_t* x, const COMPV_ALIGNED(SSE) compv_float64_t* y, compv_uscalar_t numPoints, compv_float64_t* tx1, compv_float64_t* ty1, compv_float64_t* s1);
	COMPV_EXTERNC void CompVMathStatsNormalize2DHartley_4_64f_Asm_X86_SSE2(const COMPV_ALIGNED(SSE) compv_float64_t* x, const COMPV_ALIGNED(SSE) compv_float64_t* y, compv_uscalar_t numPoints, compv_float64_t* tx1, compv_float64_t* ty1, compv_float64_t* s1);
	COMPV_EXTERNC void CompVMathStatsMSE2DHomogeneous_64f_Asm_X86_SSE2(const COMPV_ALIGNED(SSE) compv_float64_t* aX_h, const COMPV_ALIGNED(SSE) compv_float64_t* aY_h, const COMPV_ALIGNED(SSE) compv_float64_t* aZ_h, const COMPV_ALIGNED(SSE) compv_float64_t* bX, const COMPV_ALIGNED(SSE) compv_float64_t* bY, COMPV_ALIGNED(SSE) compv_float64_t* mse, compv_uscalar_t numPoints);
	COMPV_EXTERNC void CompVMathStatsMSE2DHomogeneous_4_64f_Asm_X86_SSE2(const COMPV_ALIGNED(SSE) compv_float64_t* aX_h, const COMPV_ALIGNED(SSE) compv_float64_t* aY_h, const COMPV_ALIGNED(SSE) compv_float64_t* aZ_h, const COMPV_ALIGNED(SSE) compv_float64_t* bX, const COMPV_ALIGNED(SSE) compv_float64_t* bY, COMPV_ALIGNED(SSE) compv_float64_t* mse, compv_uscalar_t numPoints);
	COMPV_EXTERNC void CompVMathStatsVariance_64f_Asm_X86_SSE2(const COMPV_ALIGNED(SSE) compv_float64_t* data, compv_uscalar_t count, const compv_float64_t* mean1, compv_float64_t* var1);
	COMPV_EXTERNC void CompVMathStatsVariance_64f_Asm_X86_AVX(const COMPV_ALIGNED(AVX) compv_float64_t* data, compv_uscalar_t count, const compv_float64_t* mean1, compv_float64_t* var1);
#	endif /* COMPV_ARCH_X86 */
#	if COMPV_ARCH_X64
	COMPV_EXTERNC void CompVMathStatsMSE2DHomogeneous_64f_Asm_X64_SSE2(const COMPV_ALIGNED(SSE) compv_float64_t* aX_h, const COMPV_ALIGNED(SSE) compv_float64_t* aY_h, const COMPV_ALIGNED(SSE) compv_float64_t* aZ_h, const COMPV_ALIGNED(SSE) compv_float64_t* bX, const COMPV_ALIGNED(SSE) compv_float64_t* bY, COMPV_ALIGNED(SSE) compv_float64_t* mse, compv_uscalar_t numPoints);
#	endif /* COMPV_ARCH_X64 */
#	if COMPV_ARCH_ARM
	COMPV_EXTERNC void CompVMathStatsNormalize2DHartley_64f_Asm_NEON32(const COMPV_ALIGNED(NEON) compv_float64_t* x, const COMPV_ALIGNED(NEON) compv_float64_t* y, compv_uscalar_t numPoints, compv_float64_t* tx1, compv_float64_t* ty1, compv_float64_t* s1);
    COMPV_EXTERNC void CompVMathStatsNormalize2DHartley_4_64f_Asm_NEON32(const COMPV_ALIGNED(NEON) compv_float64_t* x, const COMPV_ALIGNED(NEON) compv_float64_t* y, compv_uscalar_t numPoints, compv_float64_t* tx1, compv_float64_t* ty1, compv_float64_t* s1);
	COMPV_EXTERNC void CompVMathStatsMSE2DHomogeneous_64f_Asm_NEON32(const COMPV_ALIGNED(NEON) compv_float64_t* aX_h, const COMPV_ALIGNED(NEON) compv_float64_t* aY_h, const COMPV_ALIGNED(NEON) compv_float64_t* aZ_h, const COMPV_ALIGNED(NEON) compv_float64_t* bX, const COMPV_ALIGNED(NEON) compv_float64_t* bY, COMPV_ALIGNED(NEON) compv_float64_t* mse, compv_uscalar_t numPoints);
	COMPV_EXTERNC void CompVMathStatsMSE2DHomogeneous_4_64f_Asm_NEON32(const COMPV_ALIGNED(NEON) compv_float64_t* aX_h, const COMPV_ALIGNED(NEON) compv_float64_t* aY_h, const COMPV_ALIGNED(NEON) compv_float64_t* aZ_h, const COMPV_ALIGNED(NEON) compv_float64_t* bX, const COMPV_ALIGNED(NEON) compv_float64_t* bY, COMPV_ALIGNED(NEON) compv_float64_t* mse, compv_uscalar_t numPoints);
	COMPV_EXTERNC void CompVMathStatsVariance_64f_Asm_NEON32(const COMPV_ALIGNED(NEON) compv_float64_t* data, compv_uscalar_t count, const compv_float64_t* mean1, compv_float64_t* var1);
#   endif /* COMPV_ARCH_ARM */
#	if COMPV_ARCH_ARM64
    COMPV_EXTERNC void CompVMathStatsNormalize2DHartley_64f_Asm_NEON64(const COMPV_ALIGNED(NEON) compv_float64_t* x, const COMPV_ALIGNED(NEON) compv_float64_t* y, compv_uscalar_t numPoints, compv_float64_t* tx1, compv_float64_t* ty1, compv_float64_t* s1);
    COMPV_EXTERNC void CompVMathStatsNormalize2DHartley_4_64f_Asm_NEON64(const COMPV_ALIGNED(NEON) compv_float64_t* x, const COMPV_ALIGNED(NEON) compv_float64_t* y, compv_uscalar_t numPoints, compv_float64_t* tx1, compv_float64_t* ty1, compv_float64_t* s1);
    COMPV_EXTERNC void CompVMathStatsMSE2DHomogeneous_64f_Asm_NEON64(const COMPV_ALIGNED(NEON) compv_float64_t* aX_h, const COMPV_ALIGNED(NEON) compv_float64_t* aY_h, const COMPV_ALIGNED(NEON) compv_float64_t* aZ_h, const COMPV_ALIGNED(NEON) compv_float64_t* bX, const COMPV_ALIGNED(NEON) compv_float64_t* bY, COMPV_ALIGNED(NEON) compv_float64_t* mse, compv_uscalar_t numPoints);
    COMPV_EXTERNC void CompVMathStatsMSE2DHomogeneous_4_64f_Asm_NEON64(const COMPV_ALIGNED(NEON) compv_float64_t* aX_h, const COMPV_ALIGNED(NEON) compv_float64_t* aY_h, const COMPV_ALIGNED(NEON) compv_float64_t* aZ_h, const COMPV_ALIGNED(NEON) compv_float64_t* bX, const COMPV_ALIGNED(NEON) compv_float64_t* bY, COMPV_ALIGNED(NEON) compv_float64_t* mse, compv_uscalar_t numPoints);
    COMPV_EXTERNC void CompVMathStatsVariance_64f_Asm_NEON64(const COMPV_ALIGNED(NEON) compv_float64_t* data, compv_uscalar_t count, const compv_float64_t* mean1, compv_float64_t* var1);
#   endif /* COMPV_ARCH_ARM64 */
#endif /* COMPV_ASM */

//
//	=== CompVMathStatsGeneric ===
//

class CompVMathStatsGeneric {
	public:
		/*
		2D Points normalization as described by Hartley. Used before computing Homography or Fundamental matrix.
		More info: https://en.wikipedia.org/wiki/Eight-point_algorithm#How_it_can_be_solved
		* tx1 / ty1: The X and Y translation values to be used to build the transformation matrix.
		* s1: The X and Y scaling factor to be used to build the transformation matrix.
		*/
		template<typename T>
		static COMPV_ERROR_CODE normalize2D_hartley(const T* x, const T* y, size_t numPoints, T* tx1, T* ty1, T* s1)
		{
			COMPV_CHECK_EXP_RETURN(!x || !y || !numPoints || !tx1 || !ty1 || !s1, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

#if 0 // TODO(dmi): add MT implementation
			if (numPoints > 100) {
				COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No MT implementation found");
			}
#endif

			if (std::is_same<T, compv_float64_t>::value) {
				void(*CompVMathStatsNormalize2DHartley_64f)(const COMPV_ALIGNED(V) compv_float64_t* x, const COMPV_ALIGNED(V) compv_float64_t* y, compv_uscalar_t numPoints, compv_float64_t* tx1, compv_float64_t* ty1, compv_float64_t* s1) = NULL;
#if COMPV_ARCH_X86
				if (CompVCpu::isEnabled(compv::kCpuFlagSSE2) && numPoints > 1 && COMPV_IS_ALIGNED_SSE(x) && COMPV_IS_ALIGNED_SSE(y)) {
					COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathStatsNormalize2DHartley_64f = CompVMathStatsNormalize2DHartley_64f_Intrin_SSE2);
					COMPV_EXEC_IFDEF_ASM_X86(CompVMathStatsNormalize2DHartley_64f = CompVMathStatsNormalize2DHartley_64f_Asm_X86_SSE2);
					if (numPoints == 4) { // Homography -> very common
						COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathStatsNormalize2DHartley_64f = CompVMathStatsNormalize2DHartley_4_64f_Intrin_SSE2);
						COMPV_EXEC_IFDEF_ASM_X86(CompVMathStatsNormalize2DHartley_64f = CompVMathStatsNormalize2DHartley_4_64f_Asm_X86_SSE2);
					}
				}
#	if 0 // TODO(dmi): AVX code not faster than SSE (not a surprise because it's deprecated code not fully optimized)
				if (CompVCpu::isEnabled(compv::kCpuFlagAVX) && numPoints > 3 && COMPV_IS_ALIGNED_AVX(x) && COMPV_IS_ALIGNED_AVX(y)) {
					COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // AVX code not faster than SSE
					COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathStatsNormalize2DHartley_64f = CompVMathStatsNormalize2DHartley_64f_Intrin_AVX);
					COMPV_EXEC_IFDEF_ASM_X86(CompVMathStatsNormalize2DHartley_64f = CompVMathStatsNormalize2DHartley_64f_Asm_X86_AVX);
					if (numPoints == 4) { // Homography -> very common
						COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathStatsNormalize2DHartley_64f = MathStatsNormalize2DHartley_4_64f_Intrin_AVX);
						COMPV_EXEC_IFDEF_ASM_X86(CompVMathStatsNormalize2DHartley_64f = MathStatsNormalize2DHartley_4_64f_Asm_X86_AVX);
					}
				}
#	endif
#elif COMPV_ARCH_ARM
				if (CompVCpu::isEnabled(compv::kCpuFlagARM_NEON) && numPoints > 1 && COMPV_IS_ALIGNED_NEON(x) && COMPV_IS_ALIGNED_NEON(y)) {
					COMPV_EXEC_IFDEF_ASM_ARM32(CompVMathStatsNormalize2DHartley_64f = CompVMathStatsNormalize2DHartley_64f_Asm_NEON32);
					COMPV_EXEC_IFDEF_INTRIN_ARM64(CompVMathStatsNormalize2DHartley_64f = CompVMathStatsNormalize2DHartley_64f_Intrin_NEON64);
					COMPV_EXEC_IFDEF_ASM_ARM64(CompVMathStatsNormalize2DHartley_64f = CompVMathStatsNormalize2DHartley_64f_Asm_NEON64);
					if (numPoints == 4) { // Homography -> very common
						COMPV_EXEC_IFDEF_ASM_ARM32(CompVMathStatsNormalize2DHartley_64f = CompVMathStatsNormalize2DHartley_4_64f_Asm_NEON32);
						COMPV_EXEC_IFDEF_INTRIN_ARM64(CompVMathStatsNormalize2DHartley_64f = CompVMathStatsNormalize2DHartley_4_64f_Intrin_NEON64);
						COMPV_EXEC_IFDEF_ASM_ARM64(CompVMathStatsNormalize2DHartley_64f = CompVMathStatsNormalize2DHartley_4_64f_Asm_NEON64);
					}
				}
#endif
				if (CompVMathStatsNormalize2DHartley_64f) {
					CompVMathStatsNormalize2DHartley_64f(reinterpret_cast<const compv_float64_t*>(x), reinterpret_cast<const compv_float64_t*>(y), static_cast<compv_uscalar_t>(numPoints), reinterpret_cast<compv_float64_t*>(tx1), reinterpret_cast<compv_float64_t*>(ty1), reinterpret_cast<compv_float64_t*>(s1));
					return COMPV_ERROR_CODE_S_OK;
				}
			}

			COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
			size_t i;
			T tx = 0, ty = 0, magnitude = 0, OneOverNumPoints = static_cast<T>(static_cast<T>(1) / numPoints), a, b;

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
				magnitude += static_cast<T>(COMPV_MATH_HYPOT_NAIVE(a, b));
			}
			magnitude *= OneOverNumPoints;

			// TODO(dmi): magnitude cannot be equal to zero (sum of positive values)
			*s1 = magnitude ? static_cast<T>(COMPV_MATH_SQRT_2 / magnitude) : static_cast<T>(COMPV_MATH_SQRT_2); // (sqrt(2) / magnitude)
			*tx1 = tx;
			*ty1 = ty;

			return COMPV_ERROR_CODE_S_OK;
		}

		// Computes Mean Squared Error (MSE) / Mean Squared Deviation (MSD) between A(x,y,z) in homogeneous coordsys and B(x,y) in cartesian coordsys.
		template<typename T>
		static COMPV_ERROR_CODE mse2D_homogeneous(CompVMatPtrPtr mse, const T* aX_h, const T* aY_h, const T* aZ_h, const T* bX, const T* bY, size_t numPoints)
		{
			COMPV_CHECK_EXP_RETURN(!aX_h || !aY_h || !aZ_h || !bX || !bY || !numPoints, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

#if 0 // TODO(dmi): add MT implementation
			if (numPoints > 100) {
				COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No MT implementation found");
			}
#endif

			COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<T>(mse, 1, numPoints));
			T* msePtr = (*mse)->ptr<T>();

			if (std::is_same<T, compv_float64_t>::value) {
				void(*CompVMathStatsMSE2DHomogeneous_64f)(const COMPV_ALIGNED(X) compv_float64_t* aX_h, const COMPV_ALIGNED(X) compv_float64_t* aY_h, const COMPV_ALIGNED(X) compv_float64_t* aZ_h, const COMPV_ALIGNED(X) compv_float64_t* bX, const COMPV_ALIGNED(X) compv_float64_t* bY, COMPV_ALIGNED(X) compv_float64_t* mse, compv_uscalar_t numPoints) = NULL;
#if COMPV_ARCH_X86
				if (CompVCpu::isEnabled(compv::kCpuFlagSSE2) && numPoints > 1 && COMPV_IS_ALIGNED_SSE(aX_h) && COMPV_IS_ALIGNED_SSE(aY_h) && COMPV_IS_ALIGNED_SSE(aZ_h) && COMPV_IS_ALIGNED_SSE(bX) && COMPV_IS_ALIGNED_SSE(bY) && COMPV_IS_ALIGNED_SSE(msePtr)) {
					COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathStatsMSE2DHomogeneous_64f = CompVMathStatsMSE2DHomogeneous_64f_Intrin_SSE2);
					COMPV_EXEC_IFDEF_ASM_X86(CompVMathStatsMSE2DHomogeneous_64f = CompVMathStatsMSE2DHomogeneous_64f_Asm_X86_SSE2);
					COMPV_EXEC_IFDEF_ASM_X64(CompVMathStatsMSE2DHomogeneous_64f = CompVMathStatsMSE2DHomogeneous_64f_Asm_X64_SSE2);
					if (numPoints == 4) { // Homography -> very common (not true!!)
						COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathStatsMSE2DHomogeneous_64f = CompVMathStatsMSE2DHomogeneous_4_64f_Intrin_SSE2);
						COMPV_EXEC_IFDEF_ASM_X86(CompVMathStatsMSE2DHomogeneous_64f = CompVMathStatsMSE2DHomogeneous_4_64f_Asm_X86_SSE2);
					}
				}
#	if 0
				COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // AVX code not faster than SSE  (not a surprise because it's deprecated code not fully optimized)
				if (CompVCpu::isEnabled(compv::kCpuFlagAVX) && numPoints > 1 && COMPV_IS_ALIGNED_AVX(aX_h) && COMPV_IS_ALIGNED_AVX(aY_h) && COMPV_IS_ALIGNED_AVX(aZ_h) && COMPV_IS_ALIGNED_AVX(bX) && COMPV_IS_ALIGNED_AVX(bY) && COMPV_IS_ALIGNED_AVX(msePtr)) {
					COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathStatsMSE2DHomogeneous_64f = CompVMathStatsMSE2DHomogeneous_64f_Intrin_AVX);
				}
#	endif
#elif COMPV_ARCH_ARM
				if (CompVCpu::isEnabled(compv::kCpuFlagARM_NEON) && numPoints > 1 && COMPV_IS_ALIGNED_NEON(aX_h) && COMPV_IS_ALIGNED_NEON(aY_h) && COMPV_IS_ALIGNED_NEON(aZ_h) && COMPV_IS_ALIGNED_NEON(bX) && COMPV_IS_ALIGNED_NEON(bY) && COMPV_IS_ALIGNED_NEON(msePtr)) {
					COMPV_EXEC_IFDEF_ASM_ARM32(CompVMathStatsMSE2DHomogeneous_64f = CompVMathStatsMSE2DHomogeneous_64f_Asm_NEON32);
					COMPV_EXEC_IFDEF_INTRIN_ARM64(CompVMathStatsMSE2DHomogeneous_64f = CompVMathStatsMSE2DHomogeneous_64f_Intrin_NEON64);
					COMPV_EXEC_IFDEF_ASM_ARM64(CompVMathStatsMSE2DHomogeneous_64f = CompVMathStatsMSE2DHomogeneous_64f_Asm_NEON64);
					if (numPoints == 4) {
						COMPV_EXEC_IFDEF_ASM_ARM32(CompVMathStatsMSE2DHomogeneous_64f = CompVMathStatsMSE2DHomogeneous_4_64f_Asm_NEON32);
						COMPV_EXEC_IFDEF_INTRIN_ARM64(CompVMathStatsMSE2DHomogeneous_64f = CompVMathStatsMSE2DHomogeneous_4_64f_Intrin_NEON64);
						COMPV_EXEC_IFDEF_ASM_ARM64(CompVMathStatsMSE2DHomogeneous_64f = CompVMathStatsMSE2DHomogeneous_4_64f_Asm_NEON64);
					}
				}
#endif
				if (CompVMathStatsMSE2DHomogeneous_64f) {
					CompVMathStatsMSE2DHomogeneous_64f(reinterpret_cast<const compv_float64_t*>(aX_h), reinterpret_cast<const compv_float64_t*>(aY_h), reinterpret_cast<const compv_float64_t*>(aZ_h), reinterpret_cast<const compv_float64_t*>(bX), reinterpret_cast<const compv_float64_t*>(bY), reinterpret_cast<compv_float64_t*>(msePtr), static_cast<compv_uscalar_t>(numPoints));
					return COMPV_ERROR_CODE_S_OK;
				}
			}

			COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
			T ex, ey, scale;
			for (size_t i = 0; i < numPoints; ++i) {
				// scale used to convert A from homogeneous to cartesian coordsys
				// z = 0 -> point a infinity (no division error is T is floating point number)
				scale = static_cast<T>(1) / aZ_h[i];
				ex = (aX_h[i] * scale) - bX[i];
				ey = (aY_h[i] * scale) - bY[i];
				msePtr[i] = ((ex * ex) + (ey * ey));
			}
			return COMPV_ERROR_CODE_S_OK;
		}

		template<typename T>
		static COMPV_ERROR_CODE mse2D(const CompVMatPtr& aPoints, const CompVMatPtr& bPoints, T& error)
		{
			COMPV_CHECK_EXP_RETURN(!aPoints || !bPoints || aPoints->cols() != bPoints->cols() || aPoints->rows() != bPoints->rows() || aPoints->rows() < 2 || aPoints->subType() != bPoints->subType(),
				COMPV_ERROR_CODE_E_INVALID_PARAMETER);
			COMPV_CHECK_EXP_RETURN(!aPoints->isRawTypeMatch<T>(), COMPV_ERROR_CODE_E_INVALID_CALL);

			COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
			error = 0;
			const size_t numPoints = bPoints->cols();
			T diffX, diffY;
			const T* aPointsX = aPoints->ptr<T>(0);
			const T* aPointsY = aPoints->ptr<T>(1);
			const T* bPointsX = bPoints->ptr<T>(0);
			const T* bPointsY = bPoints->ptr<T>(1);
			// MSE
			for (size_t i = 0; i < numPoints; ++i) {
				diffX = aPointsX[i] - bPointsX[i];
				diffY = aPointsY[i] - bPointsY[i];
				error += (diffX * diffX) + (diffY * diffY);
			}
			return COMPV_ERROR_CODE_S_OK;
		}

		// Computes the variance : https://en.wikipedia.org/wiki/Variance
		template<typename T>
		static COMPV_ERROR_CODE variance(const T* data, size_t count, T mean, T* var1)
		{
			COMPV_CHECK_EXP_RETURN(!data || count < 2 || !var1, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

#if 0 // TODO(dmi): add MT implementation
			if (count > 100) {
				COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No MT implementation found");
			}
#endif

			if (std::is_same<T, compv_float64_t>::value) {
				void(*CompVMathStatsVariance_64f)(const COMPV_ALIGNED(X) compv_float64_t* data, compv_uscalar_t count, const compv_float64_t* mean1, compv_float64_t* var1) = NULL;
#if COMPV_ARCH_X86
				if (CompVCpu::isEnabled(compv::kCpuFlagSSE2) && count > 1 && COMPV_IS_ALIGNED_SSE(data)) {
					COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathStatsVariance_64f = CompVMathStatsVariance_64f_Intrin_SSE2);
					COMPV_EXEC_IFDEF_ASM_X86(CompVMathStatsVariance_64f = CompVMathStatsVariance_64f_Asm_X86_SSE2);
				}
				if (CompVCpu::isEnabled(compv::kCpuFlagAVX) && count > 3 && COMPV_IS_ALIGNED_AVX(data)) {
					COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathStatsVariance_64f = CompVMathStatsVariance_64f_Intrin_AVX);
					COMPV_EXEC_IFDEF_ASM_X86(CompVMathStatsVariance_64f = CompVMathStatsVariance_64f_Asm_X86_AVX);
				}
#elif COMPV_ARCH_ARM
				if (CompVCpu::isEnabled(compv::kCpuFlagARM_NEON) && count > 1 && COMPV_IS_ALIGNED_NEON(data)) {
					COMPV_EXEC_IFDEF_ASM_ARM32(CompVMathStatsVariance_64f = CompVMathStatsVariance_64f_Asm_NEON32);
					COMPV_EXEC_IFDEF_INTRIN_ARM64(CompVMathStatsVariance_64f = CompVMathStatsVariance_64f_Intrin_NEON64);
					COMPV_EXEC_IFDEF_ASM_ARM64(CompVMathStatsVariance_64f = CompVMathStatsVariance_64f_Asm_NEON64);
				}
#endif
				if (CompVMathStatsVariance_64f) {
					CompVMathStatsVariance_64f(reinterpret_cast<const compv_float64_t*>(data), (compv_uscalar_t)count, reinterpret_cast<const compv_float64_t*>(&mean), reinterpret_cast<compv_float64_t*>(var1));
					return COMPV_ERROR_CODE_S_OK;
				}
			}

			COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
			T dev, var = 0;
			for (size_t i = 0; i < count; ++i) {
				dev = (data[i] - mean);
				var += (dev * dev);
			}
			*var1 = static_cast<T>(var / (count - 1)); // -1 for Bessel's correction: https://en.wikipedia.org/wiki/Bessel%27s_correction
			return COMPV_ERROR_CODE_S_OK;
		}

		// Compute the standard deviation(std) : https://en.wikipedia.org/wiki/Standard_deviation
		// std = sqrt(variance).For performance reasons we can use the varianc for comparison to save CPU cycles.
		template<typename T>
		static COMPV_ERROR_CODE stdev(const T* data, size_t count, T mean, T* std1)
		{
			T var;
			COMPV_CHECK_CODE_RETURN(variance(data, count, mean, &var));
			*std1 = static_cast<T>(COMPV_MATH_SQRT(var));
			return COMPV_ERROR_CODE_S_OK;
		}

		template<typename T>
		static COMPV_ERROR_CODE normL2(const CompVMatPtr& ptrIn, CompVMatPtrPtr ptrOut, const double maxVal COMPV_DEFAULT(1.0))
		{
			COMPV_CHECK_EXP_RETURN(!ptrIn || !ptrOut, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
			COMPV_CHECK_EXP_RETURN(!ptrIn->isRawTypeMatch<T>(), COMPV_ERROR_CODE_E_INVALID_CALL);

			COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No MT implementation found");

			CompVMatPtr ptrOut_ = *ptrOut;
			if (ptrOut_ != ptrIn) { // This function allows ptrIn to be equal to ptrOut
				COMPV_CHECK_CODE_RETURN(CompVMat::newObj(&ptrOut_, ptrIn));
			}
			const size_t width = ptrIn->cols();
			const size_t height = ptrIn->rows();
			const size_t stride = ptrIn->stride();

			T* inPtr = ptrIn->ptr<T>();
			T* outPtr = ptrOut_->ptr<T>();

			COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
			
			double sum = 0;
			for (size_t j = 0; j < height; ++j) {
				for (size_t i = 0; i < width; ++i) {
					sum += (inPtr[i] * inPtr[i]);
				}
				inPtr += stride;
			}
			const T scale = static_cast<T>((sum > DBL_EPSILON) ? (maxVal / std::sqrt(sum)) : 0.);

			inPtr = ptrIn->ptr<T>(); // restore
			for (size_t j = 0; j < height; ++j) {
				for (size_t i = 0; i < width; ++i) {
					outPtr[i] = inPtr[i] * scale;
				}
				inPtr += stride;
				outPtr += stride;
			}

			*ptrOut = ptrOut_;
			return COMPV_ERROR_CODE_S_OK;
		}
};



//
//	=== CompVMathStats ===
//



template<> COMPV_BASE_API COMPV_ERROR_CODE CompVMathStats::normalize2D_hartley(const compv_float32_t* x, const compv_float32_t* y, size_t numPoints, compv_float32_t* tx1, compv_float32_t* ty1, compv_float32_t* s1) {
	COMPV_CHECK_CODE_RETURN((CompVMathStatsGeneric::normalize2D_hartley<compv_float32_t>(x, y, numPoints, tx1, ty1, s1)));
	return COMPV_ERROR_CODE_S_OK;
}
template<> COMPV_BASE_API COMPV_ERROR_CODE CompVMathStats::normalize2D_hartley(const compv_float64_t* x, const compv_float64_t* y, size_t numPoints, compv_float64_t* tx1, compv_float64_t* ty1, compv_float64_t* s1) {
	COMPV_CHECK_CODE_RETURN((CompVMathStatsGeneric::normalize2D_hartley<compv_float64_t>(x, y, numPoints, tx1, ty1, s1)));
	return COMPV_ERROR_CODE_S_OK;
}

template<> COMPV_BASE_API COMPV_ERROR_CODE CompVMathStats::mse2D_homogeneous(CompVMatPtrPtr mse, const compv_float32_t* aX_h, const compv_float32_t* aY_h, const compv_float32_t* aZ_h, const compv_float32_t* bX, const compv_float32_t* bY, size_t numPoints) {
	COMPV_CHECK_CODE_RETURN((CompVMathStatsGeneric::mse2D_homogeneous<compv_float32_t>(mse, aX_h, aY_h, aZ_h, bX, bY, numPoints)));
	return COMPV_ERROR_CODE_S_OK;
}
template<> COMPV_BASE_API COMPV_ERROR_CODE CompVMathStats::mse2D_homogeneous(CompVMatPtrPtr mse, const compv_float64_t* aX_h, const compv_float64_t* aY_h, const compv_float64_t* aZ_h, const compv_float64_t* bX, const compv_float64_t* bY, size_t numPoints) {
	COMPV_CHECK_CODE_RETURN((CompVMathStatsGeneric::mse2D_homogeneous<compv_float64_t>(mse, aX_h, aY_h, aZ_h, bX, bY, numPoints)));
	return COMPV_ERROR_CODE_S_OK;
}


COMPV_ERROR_CODE CompVMathStats::mse2D(const CompVMatPtr& aPoints, const CompVMatPtr& bPoints, double& error)
{
	COMPV_CHECK_EXP_RETURN(!aPoints | !bPoints || aPoints->subType() != bPoints->subType(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	switch (aPoints->subType()) {
	case COMPV_SUBTYPE_RAW_FLOAT64:
		COMPV_CHECK_CODE_RETURN((CompVMathStatsGeneric::mse2D<compv_float64_t>(aPoints, bPoints, error)));
		return COMPV_ERROR_CODE_S_OK;
	case COMPV_SUBTYPE_RAW_FLOAT32: {
		compv_float32_t _error;
		COMPV_CHECK_CODE_RETURN((CompVMathStatsGeneric::mse2D<compv_float32_t>(aPoints, bPoints, _error)));
		error = _error;
		return COMPV_ERROR_CODE_S_OK;
	}
	default:
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
		return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
	}
}

template<> COMPV_BASE_API COMPV_ERROR_CODE CompVMathStats::variance(const compv_float32_t* data, size_t count, compv_float32_t mean, compv_float32_t* var1) {
	COMPV_CHECK_CODE_RETURN((CompVMathStatsGeneric::variance<compv_float32_t>(data, count, mean, var1)));
	return COMPV_ERROR_CODE_S_OK;
}
template<> COMPV_BASE_API COMPV_ERROR_CODE CompVMathStats::variance(const compv_float64_t* data, size_t count, compv_float64_t mean, compv_float64_t* var1) {
	COMPV_CHECK_CODE_RETURN((CompVMathStatsGeneric::variance<compv_float64_t>(data, count, mean, var1)));
	return COMPV_ERROR_CODE_S_OK;
}

template<> COMPV_BASE_API COMPV_ERROR_CODE CompVMathStats::stdev(const compv_float32_t* data, size_t count, compv_float32_t mean, compv_float32_t* std1) {
	COMPV_CHECK_CODE_RETURN((CompVMathStatsGeneric::stdev<compv_float32_t>(data, count, mean, std1)));
	return COMPV_ERROR_CODE_S_OK;
}
template<> COMPV_BASE_API COMPV_ERROR_CODE CompVMathStats::stdev(const compv_float64_t* data, size_t count, compv_float64_t mean, compv_float64_t* std1) {
	COMPV_CHECK_CODE_RETURN((CompVMathStatsGeneric::stdev<compv_float64_t>(data, count, mean, std1)));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMathStats::normL2(const CompVMatPtr& ptrIn, CompVMatPtrPtr ptrOut, const double maxVal COMPV_DEFAULT(1.0))
{
	COMPV_CHECK_EXP_RETURN(!ptrIn || !ptrOut, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	switch (ptrIn->subType()) {
	case COMPV_SUBTYPE_RAW_FLOAT64:
		COMPV_CHECK_CODE_RETURN((CompVMathStatsGeneric::normL2<compv_float64_t>(ptrIn, ptrOut, maxVal)));
		return COMPV_ERROR_CODE_S_OK;
	case COMPV_SUBTYPE_RAW_FLOAT32:
		COMPV_CHECK_CODE_RETURN((CompVMathStatsGeneric::normL2<compv_float32_t>(ptrIn, ptrOut, maxVal)));
		return COMPV_ERROR_CODE_S_OK;
	default:
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
		return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
	}
}

COMPV_NAMESPACE_END()
