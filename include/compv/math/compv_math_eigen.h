/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_MATH_EIGEN_H_)
#define _COMPV_MATH_EIGEN_H_

#include "compv/compv_config.h"
#include "compv/compv_array.h"

#include <math.h>

COMPV_NAMESPACE_BEGIN()

template<class T>
class COMPV_API CompVEigen
{
public:
	static COMPV_ERROR_CODE findSymm(const CompVPtrArray(T) &S, CompVPtrArray(T) &D, CompVPtrArray(T) &V, bool sort = true);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_MATH_EIGEN_H_ */
