/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MATH_EIGEN_H_)
#define _COMPV_BASE_MATH_EIGEN_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_mat.h"

COMPV_NAMESPACE_BEGIN()

template<class T>
class COMPV_BASE_API CompVMathEigen
{
public:
	static COMPV_ERROR_CODE findSymm(const CompVMatPtr &S, CompVMatPtrPtr D, CompVMatPtrPtr Q, bool sort = true, bool rowVectors = false, bool forceZerosInD = true);
	static COMPV_ERROR_CODE find2x2(const T(&A)[4], T(&D)[4], T(&Q)[4], bool sort = true, bool norm = true);
	static T epsilon();
	static bool isCloseToZero(T a);

private:
	static COMPV_ERROR_CODE maxAbsOffDiagSymm(const T* rowPtr, const size_t row, size_t *col, T* max);
	static void jacobiAngles(const CompVMatPtr &S, size_t ith, size_t jth, T *c, T *s);
	static void jacobiAngles_Left(const CompVMatPtr &S, size_t ith, size_t jth, T *c, T *s);
	static void extract2Cols(const CompVMatPtr &A, size_t a_col0, size_t a_col1, CompVMatPtr &R);
	static void extract1Col(const CompVMatPtr &A, size_t a_col0, CompVMatPtr &R, size_t r_row);
	static void insert2Cols(const CompVMatPtr &A, CompVMatPtr &R, size_t r_col0, size_t r_col1);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_MATH_EIGEN_H_ */

