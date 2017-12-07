/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MATH_TRIG_H_)
#define _COMPV_BASE_MATH_TRIG_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_mat.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_BASE_API CompVMathTrig
{
public:
	template<typename FloatType>
	static COMPV_ERROR_CODE rodriguesVectorToMatrix(const FloatType(&vector)[3], CompVMatPtrPtr matrix);

	template<typename FloatType>
	static COMPV_ERROR_CODE rodriguesMatrixToVector(const CompVMatPtr& matrix, FloatType(&vector)[3]);
};

COMPV_TEMPLATE_EXTERN COMPV_BASE_API COMPV_ERROR_CODE CompVMathTrig::rodriguesVectorToMatrix(const compv_float64x3_t &vector, CompVMatPtrPtr matrix);
COMPV_TEMPLATE_EXTERN COMPV_BASE_API COMPV_ERROR_CODE CompVMathTrig::rodriguesVectorToMatrix(const compv_float32x3_t &vector, CompVMatPtrPtr matrix);

COMPV_TEMPLATE_EXTERN COMPV_BASE_API COMPV_ERROR_CODE CompVMathTrig::rodriguesMatrixToVector(const CompVMatPtr& matrix, compv_float64x3_t &vector);
COMPV_TEMPLATE_EXTERN COMPV_BASE_API COMPV_ERROR_CODE CompVMathTrig::rodriguesMatrixToVector(const CompVMatPtr& matrix, compv_float32x3_t &vector);

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_MATH_TRIG_H_ */

