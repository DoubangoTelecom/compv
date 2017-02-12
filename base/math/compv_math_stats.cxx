/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/compv_math_stats.h"
#include "compv/base/math/compv_math_utils.h"

#define COMPV_THIS_CLASSNAME	"CompVMathStats"

COMPV_NAMESPACE_BEGIN()

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

	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
	if (numPoints > 100) {
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No MT implementation found");
	}
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

	*s1 = magnitude ? static_cast<T>(COMPV_MATH_SQRT_2 / magnitude) : static_cast<T>(COMPV_MATH_SQRT_2); // (sqrt(2) / magnitude)
	*tx1 = tx;
	*ty1 = ty;

	return COMPV_ERROR_CODE_S_OK;
}

// Computes Mean Squared Error (MSE) / Mean Squared Deviation (MSD) between A(x,y,z) in homogeneous coordsys and B(x,y) in cartesian coordsys.
template <class T>
COMPV_ERROR_CODE CompVMathStats<T>::mse2D_homogeneous(CompVMatPtrPtr mse, const T* aX_h, const T* aY_h, const T* aZ_h, const T* bX, const T* bY, size_t numPoints)
{
	COMPV_CHECK_EXP_RETURN(!aX_h || !aY_h || !aZ_h || !bX || !bY || !numPoints, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
	if (numPoints > 100) {
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No MT implementation found");
	}
	
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<T>(mse, 1, numPoints));
	T* msePtr = (*mse)->ptr<T>();
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

// Computes the variance : https://en.wikipedia.org/wiki/Variance
template <class T>
COMPV_ERROR_CODE CompVMathStats<T>::variance(const T* data, size_t count, const T* mean1, T* var1)
{
	COMPV_CHECK_EXP_RETURN(!data || count < 2 || !mean1 || !var1, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
	if (count > 100) {
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No MT implementation found");
	}

	T dev, var = 0, mean = *mean1;
	for (size_t i = 0; i < count; ++i) {
		dev = (data[i] - mean);
		var += (dev * dev);
	}
	*var1 = static_cast<T>(var / (count - 1)); // -1 for Bessel's correction: https://en.wikipedia.org/wiki/Bessel%27s_correction
	return COMPV_ERROR_CODE_S_OK;
}

// Compute the standard deviation(std) : https://en.wikipedia.org/wiki/Standard_deviation
// std = sqrt(variance).For performance reasons we can use the varianc for comparison to save CPU cycles.
template <class T>
COMPV_ERROR_CODE CompVMathStats<T>::stdev(const T* data, size_t count, const T* mean1, T* std1)
{
	T var;
	COMPV_CHECK_CODE_RETURN(CompVMathStats<T>::variance(data, count, mean1, &var));
	*std1 = static_cast<T>(COMPV_MATH_SQRT(var));
	return COMPV_ERROR_CODE_S_OK;
}

template class CompVMathStats<int32_t >;
template class CompVMathStats<compv_float64_t >;
template class CompVMathStats<compv_float32_t >;

COMPV_NAMESPACE_END()
