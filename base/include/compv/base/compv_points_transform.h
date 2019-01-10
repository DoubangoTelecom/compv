/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_POINTS_TRANSFORM_H_)
#define _COMPV_BASE_POINTS_TRANSFORM_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_debug.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_mat.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_BASE_API CompVPointsTransform
{
public:
	template<typename T2, typename T3>
	static COMPV_ERROR_CODE vector2Dto3D(const std::vector<CompVPoint<T2>, CompVAllocatorNoDefaultConstruct<CompVPoint<T2>> >& points2D, std::vector<CompVPoint<T3>, CompVAllocatorNoDefaultConstruct<CompVPoint<T3>> >& points3D)
	{
		COMPV_CHECK_EXP_RETURN(points2D.empty(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		points3D.clear();
		std::transform(points2D.begin(), points2D.end(), std::back_inserter(points3D), [](const CompVPoint<T2>& point2D) {
			return CompVPoint<T3>(point2D.x, point2D.y, 1);
		});
		return COMPV_ERROR_CODE_S_OK;
	}

	template<typename T>
	static COMPV_ERROR_CODE vectorToMat(const std::vector<CompVPoint<T>, CompVAllocatorNoDefaultConstruct<CompVPoint<T>> >& points, CompVMatPtrPtr mat)
	{
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
		return COMPV_ERROR_CODE_S_OK;
	}
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_POINTS_TRANSFORM_H_ */
