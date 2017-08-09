/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_CORE_CALIB_UTILS_H_)
#define _COMPV_CORE_CALIB_UTILS_H_

#include "compv/core/compv_core_config.h"
#include "compv/core/calib/compv_core_calib_common.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_CORE_API CompVCalibUtils
{
public:
	static COMPV_ERROR_CODE proj2D(const CompVMatPtr& inPoints, const CompVMatPtr& K, const CompVMatPtr& d, const CompVMatPtr& R, const CompVMatPtr&t, CompVMatPtrPtr outPoints);
	static COMPV_ERROR_CODE proj2DError(const CompVMatPtr& aPoints, const CompVMatPtr& bPoints, compv_float64_t& error);
	static COMPV_ERROR_CODE proj2DError(const CompVCalibContex& context, compv_float64_t& error);
	static COMPV_ERROR_CODE proj2DError(const CompVCalibCameraPlanVector& planes, const CompVMatPtr& K, const CompVMatPtr& d, const std::vector<CompVMatPtr>& R, const std::vector<CompVMatPtr>& t, compv_float64_t& error);
	static COMPV_ERROR_CODE undist2DImage(const CompVMatPtr& imageIn, const CompVMatPtr& K, const CompVMatPtr& d, CompVMatPtrPtr imageOut);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_CORE_CALIB_UTILS_H_ */
