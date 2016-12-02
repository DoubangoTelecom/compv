/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_MATH_TRANSFORM_H_)
#define _COMPV_MATH_TRANSFORM_H_

#include "compv/compv_config.h"
#include "compv/compv_array.h"

COMPV_NAMESPACE_BEGIN()

template<class T>
class COMPV_API CompVTransform
{
public:
    static COMPV_ERROR_CODE perspective2D(const CompVPtrArray(T) &src, const CompVPtrArray(T) &M, CompVPtrArray(T) &dst);
    static COMPV_ERROR_CODE homogeneousToCartesian2D(const CompVPtrArray(T) &src, CompVPtrArray(T) &dst);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_MATH_TRANSFORM_H_ */
