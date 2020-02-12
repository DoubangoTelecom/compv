/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_IMAGE_THRESHOLD_H_)
#define _COMPV_BASE_IMAGE_THRESHOLD_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_mat.h"
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class CompVImageThreshold
{
public:
	static COMPV_ERROR_CODE otsu(const CompVMatPtr& input, double& threshold, CompVMatPtrPtr output = nullptr);
	static COMPV_ERROR_CODE global(const CompVMatPtr& input, CompVMatPtrPtr output, const double& threshold);
	static COMPV_ERROR_CODE adaptive(const CompVMatPtr& input, CompVMatPtrPtr output, const size_t& blockSize, const double& delta, const double& maxVal = 255, bool invert = false);
	static COMPV_ERROR_CODE adaptive(const CompVMatPtr& input, CompVMatPtrPtr output, const CompVMatPtr& kernel, const double& delta, const double& maxVal = 255, bool invert = false);
private:
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_IMAGE_THRESHOLD_H_ */
