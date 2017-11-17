/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
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

COMPV_NAMESPACE_BEGIN()

class COMPV_BASE_API CompVImageThreshold
{
public:
	static COMPV_ERROR_CODE otsu(const CompVMatPtr& input, double& threshold, CompVMatPtrPtr output = nullptr);
	static COMPV_ERROR_CODE global(const CompVMatPtr& input, CompVMatPtrPtr output, const double threshold);
	static COMPV_ERROR_CODE adaptive(const CompVMatPtr& input, CompVMatPtrPtr output, const size_t blockSize, const double delta, const double maxVal = 255, bool invert = false);
	static COMPV_ERROR_CODE adaptive(const CompVMatPtr& input, CompVMatPtrPtr output, const CompVMatPtr& kernel, const double delta, const double maxVal = 255, bool invert = false);
	static COMPV_ERROR_CODE kernelMean(const size_t blockSize, CompVMatPtrPtr kernel);
private:
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_IMAGE_THRESHOLD_H_ */
