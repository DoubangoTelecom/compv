/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MATH_MORPH_H_)
#define _COMPV_BASE_MATH_MORPH_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_mat.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_BASE_API CompVMathMorph
{
public:
	static COMPV_ERROR_CODE buildStructuringElement(CompVMatPtrPtr strel, const CompVSizeSz size, COMPV_MATH_MORPH_STREL_TYPE type = COMPV_MATH_MORPH_STREL_TYPE_RECT);
	static COMPV_ERROR_CODE process(const CompVMatPtr& input, const CompVMatPtr& strel, CompVMatPtrPtr output, COMPV_MATH_MORPH_OP_TYPE opType, COMPV_BORDER_TYPE borderType = COMPV_BORDER_TYPE_REPLICATE);

private:
	static COMPV_ERROR_CODE buildStructuringElementInputPtrs(const CompVMatPtr& strel, const CompVMatPtr& input, CompVMatPtrPtr strelInputPtrs);
	static COMPV_ERROR_CODE addBordersVt(const CompVMatPtr input, CompVMatPtr output, const size_t strelHeight, const COMPV_BORDER_TYPE borderTypeTop, const COMPV_BORDER_TYPE borderTypeBottom);
	static COMPV_ERROR_CODE addBordersHz(const CompVMatPtr input, CompVMatPtr output, const size_t strelWidth, const COMPV_BORDER_TYPE borderType);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_MATH_MORPH_H_ */
