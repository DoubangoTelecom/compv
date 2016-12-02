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
    static COMPV_ERROR_CODE findSymm(const CompVPtrArray(T) &S, CompVPtrArray(T) &D, CompVPtrArray(T) &Q, bool sort = true, bool rowVectors = false, bool forceZerosInD = true);
    static T epsilon();
    static bool isCloseToZero(T a);

private:
    static void jacobiAngles(const CompVPtrArray(T) &S, size_t ith, size_t jth, T *c, T *s);
    static void jacobiAngles_Left(const CompVPtrArray(T) &S, size_t ith, size_t jth, T *c, T *s);
    static void extract2Cols(const CompVPtrArray(T) &A, size_t a_col0, size_t a_col1, CompVPtrArray(T) &R);
    static void insert2Cols(const CompVPtrArray(T) &A, CompVPtrArray(T) &R, size_t r_col0, size_t r_col1);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_MATH_EIGEN_H_ */
