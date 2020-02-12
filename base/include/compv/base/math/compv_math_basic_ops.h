/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MATH_BASIC_OPS_H_)
#define _COMPV_BASE_MATH_BASIC_OPS_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_mat.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_BASE_API CompVMathBasicOps
{
public:
	static COMPV_ERROR_CODE subs(const CompVMatPtr& a, const CompVMatPtr& b, CompVMatPtrPtr r);
	static COMPV_ERROR_CODE adds(const CompVMatPtr& a, const CompVMatPtr& b, CompVMatPtrPtr r);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_MATH_BASIC_OPS_H_ */
