/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/calib/compv_core_calib_utils.h"
#include "compv/base/math/compv_math_utils.h"
#include "compv/base/math/compv_math_stats.h"
#include "compv/base/math/compv_math_matrix.h"
#include "compv/base/math/compv_math_transform.h"
#include "compv/base/image/compv_image.h"
#include "compv/base/image/compv_image_remap.h"

// Some documentation used in this implementation:
//	- [1] A Flexible New Technique for Camera Calibration: https://github.com/DoubangoTelecom/compv/blob/master/documentation/Camera%20calibration/Zhang_CamCal.pdf
//	- [2] Zhang's Camera Calibration Algorithm: In-Depth Tutorial and Implementation: https://github.com/DoubangoTelecom/compv/blob/master/documentation/Camera%20calibration/Burger-CameraCalibration-20160516.pdf
//	- [3] Photogrammetry I - 16b - DLT & Camera Calibration (2015): https://www.youtube.com/watch?v=Ou9Uj75DJX0

COMPV_NAMESPACE_BEGIN()

//
//	CompVCalibUtilsGeneric
//

template<class T>
class CompVCalibUtilsGeneric {
public:
	static COMPV_ERROR_CODE initUndistMap(const CompVMatPtr& K, const CompVMatPtr& d, CompVMatPtrPtr map, const CompVRectFloat32& roi)
	{
		// Internal function, no need to check *some* input parameters. Up to the caller.

		COMPV_CHECK_EXP_RETURN(!K || !d || K->subType() != d->subType(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		COMPV_CHECK_EXP_RETURN(K->rows() != 3 || K->cols() != 3, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "K must be (3x3) matrix");
		COMPV_CHECK_EXP_RETURN(d->rows() < 2 || d->cols() != 1, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "d must be (1x4+) vector");

		// Camera matrix
		const T fx = *K->ptr<const T>(0, 0); // alpha
		const T fy = *K->ptr<const T>(1, 1); // beta
		const T cx = *K->ptr<const T>(0, 2);
		const T cy = *K->ptr<const T>(1, 2);
		const T skew = *K->ptr<const T>(0, 1); // gamma

		// Radial and tangential distortion coefficients
		const T k1 = *d->ptr<const T>(0, 0);
		const T k2 = *d->ptr<const T>(1, 0);
		const T p1 = d->rows() > 2 ? *d->ptr<const T>(2, 0) : static_cast<T>(0);
		const T p2 = d->rows() > 3 ? *d->ptr<const T>(3, 0) : static_cast<T>(0);

		const size_t img_start_x = COMPV_MATH_ROUNDFU_2_NEAREST_INT(roi.left, size_t);
		const size_t img_start_y = COMPV_MATH_ROUNDFU_2_NEAREST_INT(roi.top, size_t);
		const size_t img_end_x = COMPV_MATH_ROUNDFU_2_NEAREST_INT(roi.right, size_t);
		const size_t img_end_y = COMPV_MATH_ROUNDFU_2_NEAREST_INT(roi.bottom, size_t);
		const size_t img_width = (img_end_x - img_start_x) + 1;
		const size_t img_height = (img_end_y - img_start_y) + 1;
		const size_t coords_num = (img_width * img_height);

		/* Compute K-inv */
		T detKinv = (fx * fy);
		COMPV_CHECK_EXP_RETURN(!detKinv, COMPV_ERROR_CODE_E_INVALID_CALL, "Camera matrix is singular");
		detKinv = (1 / detKinv);
		const T kinv11 = (fy * detKinv);
		const T kinv21 = -(skew * detKinv);
		const T kinv31 = ((skew * cy) - (fy * cx)) * detKinv;
		const T kinv22 = (fx * detKinv);
		const T kinv32 = -(cy * fx * detKinv);

		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<T>(map, 3, coords_num));
		T* mapX = (*map)->ptr<T>(0);
		T* mapY = (*map)->ptr<T>(1);
		T* mapZ = (*map)->ptr<T>(2); // always 1

		// TODO(dmi): the next loop isn't optimized but this isn't an issue because this function should
		// be called once

		// The next code is the same as dist2DPoints. We're distorting the (u,v) coords and filling a map with the distorted coords.
		// At position (u,v) the map contains (xd,yd) coords. With (xd,yd) = distorted coords.
		// The map is applied to a distorted image:
		//		Map function Map(u,v) = (xd, yd)
		//		Undist function: undistortedImage(u,v) = distortedImage(Map(u,v)) = distortedImage(xd, yd)
		
		T x, y, x2, y2, r2, r4, a1, a2, a3, rdist;
		for (size_t j = img_start_y, k = 0; j <= img_end_y; ++j) {
			for (size_t i = img_start_x; i <= img_end_x; ++i, ++k) {
				/* [2] 5.1 step 1: map = mul(Kinv, indices) */
				x = static_cast<T>(i);
				y = static_cast<T>(j);
				x = (kinv11 * x) + (kinv21 * y) + kinv31;
				y = (kinv22 * y) + kinv32;

#if 0 // z, always equal to 1 last row for kinv is "0, 0, 1"
				// https://youtu.be/Ou9Uj75DJX0?t=25m34s (1)
				COMPV_DEBUG_INFO_CODE_TODO("Kinv last line should be 0 0 1 which means z is always equal 1, remove next code");
				z = z ? (static_cast<T>(1) / z) : static_cast<T>(1);
				x *= z;
				y *= z;
#endif
				/* [2] 5.1 step 2: Applying distorsion */

				// https://youtu.be/Ou9Uj75DJX0?t=25m34s (2) or general form: https://en.wikipedia.org/wiki/Distortion_(optics)#Software_correction
				x2 = (x * x);
				y2 = (y * y);
				r2 = x2 + y2;
				r4 = r2 * r2;
				// add radial distortion
				rdist = 1 + k1 * r2 + k2 * r4;
				x *= rdist;
				y *= rdist;
				// add tangential distortion
				a1 = 2 * (x * y);
				a2 = r2 + (2 * x2);
				a3 = r2 + (2 * y2);
				x += p1 * a1 + p2 * a2;
				y += p1 * a3 + p2 * a1;

				// https://youtu.be/Ou9Uj75DJX0?t=25m34s (3)
				/* [2] 5.1 step 3: normalized = mul(K, normalized) */
				mapX[k] = (x * fx) + (skew * y) + cx;
				mapY[k] = (y * fy) + cy;
				mapZ[k] = static_cast<T>(1);
			}
		}
		return COMPV_ERROR_CODE_S_OK;
	}

	// inPoints.z is expected to be equal to 1 and there is no R and T matrices (equal to zero)
	static COMPV_ERROR_CODE dist2DPoints(const CompVMatPtr& inPoints, const CompVMatPtr& K, const CompVMatPtr& d, CompVMatPtrPtr outPoints)
	{
		COMPV_CHECK_EXP_RETURN(!inPoints || inPoints->isEmpty() || inPoints->rows() < 2 || !K || !d || !outPoints, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		COMPV_CHECK_EXP_RETURN(K->rows() != 3 || K->cols() != 3, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "K must be (3x3) matrix");
		COMPV_CHECK_EXP_RETURN(d->rows() < 2 || d->cols() != 1, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "d must be (1x4+) vector");
		COMPV_CHECK_EXP_RETURN(inPoints->subType() != K->subType() || inPoints->subType() != d->subType(), COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Subtype mismatch");

		// Called several times in Adas project
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No MT implementation found");

		CompVPointFloat32Vector::const_iterator it_intersections;
		size_t index;
		const size_t numPoints = inPoints->cols();

		// If 'inPoints' == 'outPoints' this is very good for us beacuse we
		// can avoid creating new CompVMat object
		if (inPoints != *outPoints) {
			COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<T>(outPoints, 2, numPoints));
		}

		T* outPointsX = (*outPoints)->ptr<T>(0);
		T* outPointsY = (*outPoints)->ptr<T>(1);

		const T* inPointsX = inPoints->ptr<T>(0);
		const T* inPointsY = inPoints->ptr<T>(1);

		// Camera matrix
		const T fx = *K->ptr<const T>(0, 0);
		const T fy = *K->ptr<const T>(1, 1);
		const T cx = *K->ptr<const T>(0, 2);
		const T cy = *K->ptr<const T>(1, 2);
		const T skew = *K->ptr<const T>(0, 1);

		// Radial and tangential distortion coefficients
		const T k1 = *d->ptr<const T>(0, 0);
		const T k2 = *d->ptr<const T>(1, 0);
		const T p1 = d->rows() > 2 ? *d->ptr<const T>(2, 0) : static_cast<T>(0);
		const T p2 = d->rows() > 3 ? *d->ptr<const T>(3, 0) : static_cast<T>(0);

		/* Compute K-inv */
		T detKinv = (fx * fy);
		COMPV_CHECK_EXP_RETURN(!detKinv, COMPV_ERROR_CODE_E_INVALID_CALL, "Camera matrix is singular");
		detKinv = (1 / detKinv);
		const T kinv11 = (fy * detKinv);
		const T kinv21 = -(skew * detKinv);
		const T kinv31 = ((skew * cy) - (fy * cx)) * detKinv;
		const T kinv22 = (fx * detKinv);
		const T kinv32 = -(cy * fx * detKinv);

		T x, y;
		T x2, y2, r2, r4, a1, a2, a3, rdist;
		for (index = 0; index < numPoints; ++index) {
			/* [2] 5.1 step 1: p = mul(Kinv, p) */
			x = inPointsX[index];
			y = inPointsY[index];
			x = (kinv11 * x) + (kinv21 * y) + kinv31;
			y = (kinv22 * y) + kinv32;

#if 0 // z, always equal to 1 last row for kinv is "0, 0, 1"
			// https://youtu.be/Ou9Uj75DJX0?t=25m34s (1)
			COMPV_DEBUG_INFO_CODE_TODO("Kinv last line should be 0 0 1 which means z is always equal 1, remove next code");
			z = z ? (static_cast<T>(1) / z) : static_cast<T>(1);
			x *= z;
			y *= z;
#endif

			/* [2] 5.1 step 2: Applying distorsion */
			x2 = (x * x);
			y2 = (y * y);
			r2 = x2 + y2;
			r4 = r2 * r2;
			// add radial distortion
			rdist = (1 + k1 * r2 + k2 * r4);
			x *= rdist;
			y *= rdist;
			// add tangential distortion
			a1 = 2 * (x * y);
			a2 = r2 + (2 * x2);
			a3 = r2 + (2 * y2);
			x += p1 * a1 + p2 * a2;
			y += p1 * a3 + p2 * a1;

			/* [2] 5.1 step 3: normalized = mul(K, normalized) */
			outPointsX[index] = (x * fx) + (skew * y) + cx;
			outPointsY[index] = (y * fy) + cy;
		}

		return COMPV_ERROR_CODE_S_OK;
	}
};




//
//	CompVCalibUtils
//

// inPoints must be 2D homogeneous coord. 
COMPV_ERROR_CODE CompVCalibUtils::proj2D(const CompVMatPtr& inPoints, const CompVMatPtr& K, const CompVMatPtr& d, const CompVMatPtr& R, const CompVMatPtr&t, CompVMatPtrPtr outPoints)
{
	COMPV_CHECK_EXP_RETURN(!inPoints || inPoints->isEmpty() || inPoints->rows() != 3 || !K || !d || !outPoints || !R || !t, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_EXP_RETURN(K->rows() != 3 || K->cols() != 3, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "K must be (3x3) matrix");
	COMPV_CHECK_EXP_RETURN(d->rows() < 2 || d->cols() != 1, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "d must be (1x4+) vector");
	COMPV_CHECK_EXP_RETURN(R->rows() != 3 || R->cols() != 3, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "R must be (3x3) matrix");
	COMPV_CHECK_EXP_RETURN(t->rows() != 1 || t->cols() != 3, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "R must be (1x3) vector");

	CompVPointFloat32Vector::const_iterator it_intersections;
	size_t index;
	const size_t numPoints = inPoints->cols();

	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float64_t>(outPoints, 3, numPoints));

	compv_float64_t* outPointsX = (*outPoints)->ptr<compv_float64_t>(0);
	compv_float64_t* outPointsY = (*outPoints)->ptr<compv_float64_t>(1);
	compv_float64_t* outPointsZ = (*outPoints)->ptr<compv_float64_t>(2);

	const compv_float64_t* inPointsX = inPoints->ptr<compv_float64_t>(0);
	const compv_float64_t* inPointsY = inPoints->ptr<compv_float64_t>(1);
	const compv_float64_t* inPointsZ = inPoints->ptr<compv_float64_t>(2);

	// Camera matrix
	const compv_float64_t fx = *K->ptr<const compv_float64_t>(0, 0);
	const compv_float64_t fy = *K->ptr<const compv_float64_t>(1, 1);
	const compv_float64_t cx = *K->ptr<const compv_float64_t>(0, 2);
	const compv_float64_t cy = *K->ptr<const compv_float64_t>(1, 2);
	const compv_float64_t skew = *K->ptr<const compv_float64_t>(0, 1);

	// Radial and tangential distortion coefficients
	const compv_float64_t k1 = *d->ptr<const compv_float64_t>(0, 0);
	const compv_float64_t k2 = *d->ptr<const compv_float64_t>(1, 0);
	const compv_float64_t p1 = d->rows() > 2 ? *d->ptr<const compv_float64_t>(2, 0) : 0.0;
	const compv_float64_t p2 = d->rows() > 3 ? *d->ptr<const compv_float64_t>(3, 0) : 0.0;

	// Rotation matrix
	const compv_float64_t R0 = *R->ptr<const compv_float64_t>(0, 0);
	const compv_float64_t R1 = *R->ptr<const compv_float64_t>(0, 1);
	const compv_float64_t R2 = *R->ptr<const compv_float64_t>(0, 2);
	const compv_float64_t R3 = *R->ptr<const compv_float64_t>(1, 0);
	const compv_float64_t R4 = *R->ptr<const compv_float64_t>(1, 1);
	const compv_float64_t R5 = *R->ptr<const compv_float64_t>(1, 2);
	const compv_float64_t R6 = *R->ptr<const compv_float64_t>(2, 0);
	const compv_float64_t R7 = *R->ptr<const compv_float64_t>(2, 1);
	const compv_float64_t R8 = *R->ptr<const compv_float64_t>(2, 2);

	// Translation vector
	const compv_float64_t tx = *t->ptr<const compv_float64_t>(0, 0);
	const compv_float64_t ty = *t->ptr<const compv_float64_t>(0, 1);
	const compv_float64_t tz = *t->ptr<const compv_float64_t>(0, 2);

	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");

	compv_float64_t xp, yp, zp;
	compv_float64_t x, y, z;
	compv_float64_t x2, y2, r2, r4, a1, a2, a3, rdist;
	for (index = 0; index < numPoints; ++index) {
		xp = inPointsX[index];
		yp = inPointsY[index];
		zp = inPointsZ[index];

		// Apply R and t -> inPoints = mul(Rt, inPoints)
		x = R0 * xp + R1 * yp + R2 * zp + tx;
		y = R3 * xp + R4 * yp + R5 * zp + ty;
		z = R6 * xp + R7 * yp + R8 * zp + tz;

		// Starting here we're warping. Same code as undist

		// https://youtu.be/Ou9Uj75DJX0?t=25m34s (1)
		z = z ? (1.0 / z) : 1.0;
		x *= z;
		y *= z;

		// https://youtu.be/Ou9Uj75DJX0?t=25m34s (2) or general form: https://en.wikipedia.org/wiki/Distortion_(optics)#Software_correction
		x2 = (x * x);
		y2 = (y * y);
		r2 = x2 + y2;
		r4 = r2 * r2;
		// add radial distortion
		rdist = 1 + k1 * r2 + k2 * r4;
		x *= rdist;
		y *= rdist;
		// add tangential distortion
		a1 = 2 * (x * y);
		a2 = r2 + (2 * x2);
		a3 = r2 + (2 * y2);
		x += p1 * a1 + p2 * a2;
		y += p1 * a3 + p2 * a1;

		// https://youtu.be/Ou9Uj75DJX0?t=25m34s (3) -> outPoints = mul(K, outPoints)
		outPointsX[index] = (x * fx) + (skew * y) + cx;
		outPointsY[index] = (y * fy) + cy;
		outPointsZ[index] = 1.0;
	}

	return COMPV_ERROR_CODE_S_OK;
}

// aPoints and bPoints must be 2D homogeneous coord. 
COMPV_ERROR_CODE CompVCalibUtils::proj2DError(const CompVMatPtr& aPoints, const CompVMatPtr& bPoints, compv_float64_t& error)
{
	CompVMathStats<compv_float64_t>::mse2D(aPoints, bPoints, error); // Mean Square Error
	error = std::sqrt(error / static_cast<compv_float64_t>(aPoints->cols())); // mean(sqrt)
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVCalibUtils::proj2DError(const CompVCalibContex& context, compv_float64_t& error)
{
	COMPV_CHECK_EXP_RETURN(!context.K || !context.d || context.planes.empty(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	error = 0;

	CompVMatPtr reprojected;
	compv_float64_t *intersectionsX, *intersectionsY;
	size_t i;
	CompVMatPtr intersections;
	CompVPointFloat32Vector::const_iterator it_intersections;
	compv_float64_t e;

	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float64_t>(&intersections,
		context.planes.begin()->pattern->rows(),
		context.planes.begin()->pattern->cols(),
		context.planes.begin()->pattern->stride()));
	intersectionsX = intersections->ptr<compv_float64_t>(0);
	intersectionsY = intersections->ptr<compv_float64_t>(1);
	const size_t numPointsPerPlan = intersections->cols();

	for (CompVCalibCameraPlanVector::const_iterator i_plans = context.planes.begin(); i_plans < context.planes.end(); ++i_plans) {
		COMPV_CHECK_CODE_RETURN(CompVCalibUtils::proj2D(i_plans->pattern, context.K, context.d, i_plans->R, i_plans->t, &reprojected));

		COMPV_CHECK_EXP_RETURN(i_plans->intersections.size() != reprojected->cols() || intersections->cols() != reprojected->cols(), COMPV_ERROR_CODE_E_INVALID_STATE);
		for (i = 0, it_intersections = i_plans->intersections.begin(); i < numPointsPerPlan; ++i, ++it_intersections) {
			intersectionsX[i] = static_cast<compv_float64_t>(it_intersections->x);
			intersectionsY[i] = static_cast<compv_float64_t>(it_intersections->y);
		}

		COMPV_CHECK_CODE_RETURN(CompVCalibUtils::proj2DError(reprojected, intersections, e));
		error += e;
	}

	error /= static_cast<compv_float64_t>(context.planes.size());

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVCalibUtils::proj2DError(const CompVCalibCameraPlanVector& planes, const CompVMatPtr& K, const CompVMatPtr& d, const std::vector<CompVMatPtr>& R, const std::vector<CompVMatPtr>& t, compv_float64_t& error)
{
	COMPV_CHECK_EXP_RETURN(planes.empty() || !K || !d || R.empty() || t.empty() || R.size() != t.size(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	error = 0;

	CompVMatPtr reprojected;
	compv_float64_t *intersectionsX, *intersectionsY;
	size_t i;
	CompVMatPtr intersections;
	CompVPointFloat32Vector::const_iterator it_intersections;
	std::vector<CompVMatPtr>::const_iterator it_R = R.begin();
	std::vector<CompVMatPtr>::const_iterator it_t = t.begin();
	compv_float64_t e;

	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float64_t>(&intersections,
		planes.begin()->pattern->rows(),
		planes.begin()->pattern->cols(),
		planes.begin()->pattern->stride()));
	intersectionsX = intersections->ptr<compv_float64_t>(0);
	intersectionsY = intersections->ptr<compv_float64_t>(1);
	const size_t numPointsPerPlan = intersections->cols();

	for (CompVCalibCameraPlanVector::const_iterator i_plans = planes.begin(); i_plans < planes.end(); ++i_plans, ++it_R, ++it_t) {
		COMPV_CHECK_CODE_RETURN(CompVCalibUtils::proj2D(i_plans->pattern, K, d, *it_R, *it_t, &reprojected));

		COMPV_CHECK_EXP_RETURN(i_plans->intersections.size() != reprojected->cols() || intersections->cols() != reprojected->cols(), COMPV_ERROR_CODE_E_INVALID_STATE);
		for (i = 0, it_intersections = i_plans->intersections.begin(); i < numPointsPerPlan; ++i, ++it_intersections) {
			intersectionsX[i] = static_cast<compv_float64_t>(it_intersections->x);
			intersectionsY[i] = static_cast<compv_float64_t>(it_intersections->y);
		}

		COMPV_CHECK_CODE_RETURN(CompVCalibUtils::proj2DError(reprojected, intersections, e));
		error += e;
	}

	error /= static_cast<compv_float64_t>(planes.size());

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVCalibUtils::initUndistMap(const CompVSizeSz& imageSize, const CompVMatPtr& K, const CompVMatPtr& d, CompVMatPtrPtr map, const CompVRectFloat32* imageROI COMPV_DEFAULT(nullptr))
{
	// Other parameters will be checked in the generic function
	COMPV_CHECK_EXP_RETURN(!K || !d || K->subType() != d->subType(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	CompVRectFloat32 roi;
	if (imageROI) {
		COMPV_CHECK_EXP_RETURN(imageROI->left < 0.f || imageROI->right < 0.f || imageROI->top < 0.f || imageROI->bottom < 0.f
			|| imageROI->left > imageROI->right || imageROI->top > imageROI->bottom
			|| imageROI->right >= imageSize.width || imageROI->bottom >= imageSize.height,
			COMPV_ERROR_CODE_E_INVALID_PARAMETER, "ROI is invalid");
		roi = *imageROI;
	}
	else {
		roi.left = 0;
		roi.right = static_cast<compv_float32_t>(imageSize.width - 1);
		roi.top = 0;
		roi.bottom = static_cast<compv_float32_t>(imageSize.height - 1);
	}
	
	switch (K->subType()) {
	case COMPV_SUBTYPE_RAW_FLOAT32: COMPV_CHECK_CODE_RETURN((CompVCalibUtilsGeneric<compv_float32_t>::initUndistMap(K, d, map, roi))); break;
	case COMPV_SUBTYPE_RAW_FLOAT64: COMPV_CHECK_CODE_RETURN((CompVCalibUtilsGeneric<compv_float64_t>::initUndistMap(K, d, map, roi))); break;
	default: COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED, "Camera intrinsics and distortion coefficients must be floats (32f) or doubles (64f)");  break;
	}
	return COMPV_ERROR_CODE_S_OK;
}

// Function not optimized, should call the overrided one with a mapping indices as parameter
COMPV_ERROR_CODE CompVCalibUtils::undist2DImage(const CompVMatPtr& imageIn, const CompVMatPtr& K, const CompVMatPtr& d, CompVMatPtrPtr imageOut, COMPV_INTERPOLATION_TYPE interpType COMPV_DEFAULT(COMPV_INTERPOLATION_TYPE_BILINEAR))
{
	// Other parameters will be checked in the generic function
	COMPV_CHECK_EXP_RETURN(!imageIn || imageIn->isEmpty(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	// This function isn't optimized but there is an overrided one which is optimized (see below)
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("This function isn't optimized because it generate the map indices for each call.\n You should call CompVCalibUtils::initUndistMap then CompVImageRemap::process(map).");

	CompVMatPtr map;
	COMPV_CHECK_CODE_RETURN(CompVCalibUtils::initUndistMap(CompVSizeSz(imageIn->cols(), imageIn->rows()), K, d, &map));
	COMPV_CHECK_CODE_RETURN(CompVImageRemap::process(imageIn, imageOut, map, interpType));

	return COMPV_ERROR_CODE_S_OK;
}

// map -> must be generated using CompVCalibUtils::initUndistMap
COMPV_ERROR_CODE CompVCalibUtils::undist2DImage(const CompVMatPtr& imageIn, const CompVMatPtr& map, CompVMatPtrPtr imageOut, COMPV_INTERPOLATION_TYPE interpType COMPV_DEFAULT(COMPV_INTERPOLATION_TYPE_BILINEAR), const CompVRectFloat32* imageInROI COMPV_DEFAULT(nullptr))
{
	// Other parameters will be checked in the generic function
	COMPV_CHECK_EXP_RETURN(!imageIn || imageIn->isEmpty() || !map, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	// If 'imageInROI' is non-null then, the map should have been created using the same roi

	// Do not check that map and imageIn have the same size, this will be done in CompVImageRemap::process
	COMPV_CHECK_CODE_RETURN(CompVImageRemap::process(imageIn, imageOut, map, interpType, imageInROI));

	return COMPV_ERROR_CODE_S_OK;
}

// inPoints.z is expected to be equal to 1 and there is no R and T matrices (equal to zero)
COMPV_ERROR_CODE CompVCalibUtils::dist2DPoints(const CompVMatPtr& inPoints, const CompVMatPtr& K, const CompVMatPtr& d, CompVMatPtrPtr outPoints)
{
	// Other parameters will be checked in the generic function
	COMPV_CHECK_EXP_RETURN(!K || !d || K->subType() != d->subType(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	switch (K->subType()) {
	case COMPV_SUBTYPE_RAW_FLOAT32: COMPV_CHECK_CODE_RETURN((CompVCalibUtilsGeneric<compv_float32_t>::dist2DPoints(inPoints, K, d, outPoints))); break;
	case COMPV_SUBTYPE_RAW_FLOAT64: COMPV_CHECK_CODE_RETURN((CompVCalibUtilsGeneric<compv_float64_t>::dist2DPoints(inPoints, K, d, outPoints))); break;
	default: COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED, "Camera intrinsics and distortion coefficients must be floats (32f) or doubles (64f)");  break;
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVCalibUtils::dist2DPoints(CompVMatPtr& inOutPoints, const CompVMatPtr& K, const CompVMatPtr& d)
{
	COMPV_CHECK_CODE_RETURN(CompVCalibUtils::dist2DPoints(inOutPoints, K, d, &inOutPoints));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
