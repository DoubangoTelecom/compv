/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MATH_TRANSFORM_H_)
#define _COMPV_BASE_MATH_TRANSFORM_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_mat.h"

COMPV_NAMESPACE_BEGIN()

template<class T>
class COMPV_BASE_API CompVMathTransform
{
public:
	static COMPV_ERROR_CODE perspective2D(CompVMatPtrPtr dst, const CompVMatPtr &src, const CompVMatPtr &M);
	static COMPV_ERROR_CODE homogeneousToCartesian2D(CompVMatPtrPtr dst, const CompVMatPtr &src);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_MATH_TRANSFORM_H_ */

