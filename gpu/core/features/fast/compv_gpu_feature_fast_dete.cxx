/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/gpu/core/features/fast/compv_gpu_feature_fast_dete.h"
#include "compv/gpu/compv_gpu.h"

COMPV_NAMESPACE_BEGIN()

newObjGpuCornerDeteFAST CompVGpuCornerDeteFAST::s_ptrNewObj = NULL;

CompVGpuCornerDeteFAST::CompVGpuCornerDeteFAST()
{
}

CompVGpuCornerDeteFAST::~CompVGpuCornerDeteFAST()
{
}

COMPV_ERROR_CODE CompVGpuCornerDeteFAST::newObj(CompVGpuCornerDeteFASTPtrPtr fast)
{
	COMPV_CHECK_CODE_RETURN(CompVGpu::init());
	COMPV_CHECK_EXP_RETURN(!fast, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_EXP_RETURN(!s_ptrNewObj, COMPV_ERROR_CODE_E_INVALID_CALL, "No handler");
	COMPV_CHECK_CODE_RETURN(s_ptrNewObj(fast));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

