/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MATH_MOMENTS_H_)
#define _COMPV_BASE_MATH_MOMENTS_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_mat.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_BASE_API CompVMathMoments
{
public:
	static COMPV_ERROR_CODE rawFirstOrder(const CompVMatPtr& ptrIn, double(&moments)[3], bool binar = false);
	static COMPV_ERROR_CODE rawSecondOrder(const CompVMatPtr& ptrIn, double(&moments)[6], bool binar = false);
	static COMPV_ERROR_CODE skewness(const CompVMatPtr& ptrIn, double& skew, bool binar = false);
	static COMPV_ERROR_CODE orientation(const CompVMatPtr& ptrIn, double& theta, bool binar = false);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_MATH_MOMENTS_H_ */
