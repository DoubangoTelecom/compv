/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MATH_MATRIX_H_)
#define _COMPV_BASE_MATH_MATRIX_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_mat.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_BASE_API CompVMatrix
{
public:
	static COMPV_ERROR_CODE mulAB(const CompVMatPtr &A, const CompVMatPtr &B, CompVMatPtrPtr R);
	static COMPV_ERROR_CODE mulABt(const CompVMatPtr &A, const CompVMatPtr &B, CompVMatPtrPtr R);
	static COMPV_ERROR_CODE mulAtA(const CompVMatPtr &A, CompVMatPtrPtr R);
	static COMPV_ERROR_CODE transpose(const CompVMatPtr &A, CompVMatPtrPtr R);
	static COMPV_ERROR_CODE isSymmetric(const CompVMatPtr &A, bool &symmetric);
	static COMPV_ERROR_CODE isEqual(const CompVMatPtr &A, const CompVMatPtr &B, bool &equal);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_MATH_MATRIX_H_ */

