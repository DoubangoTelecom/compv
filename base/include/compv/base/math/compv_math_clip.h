/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MATH_CLIP_H_)
#define _COMPV_BASE_MATH_CLIP_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_mat.h"
#include "compv/base/math/compv_math.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class CompVMathClip
{
public:
	static COMPV_ERROR_CODE clip3(const CompVMatPtr& in, const double minn, const double maxx, CompVMatPtrPtr out);
	static COMPV_ERROR_CODE clip2(const CompVMatPtr& in, const double maxx, CompVMatPtrPtr out);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_MATH_CLIP_H_ */
