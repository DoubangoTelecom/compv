/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/ccl/compv_core_ccl_lsl_result.h"

#define COMPV_THIS_CLASSNAME "CompVConnectedComponentLabelingResultLSLImpl"

COMPV_NAMESPACE_BEGIN()

CompVConnectedComponentLabelingResultLSLImpl::CompVConnectedComponentLabelingResultLSLImpl()
{

}

CompVConnectedComponentLabelingResultLSLImpl::~CompVConnectedComponentLabelingResultLSLImpl()
{

}

size_t CompVConnectedComponentLabelingResultLSLImpl::labelsCount() const
{
	return 0;
}

COMPV_ERROR_CODE CompVConnectedComponentLabelingResultLSLImpl::debugFlatten(CompVMatPtrPtr labels) const
{
	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVConnectedComponentLabelingResultLSLImpl::extract(std::vector<CompVMatPtr>& points) const
{
	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVConnectedComponentLabelingResultLSLImpl::boundingBoxes() const
{
	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVConnectedComponentLabelingResultLSLImpl::firstOrderMoment() const
{
	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVConnectedComponentLabelingResultLSLImpl::newObj(CompVConnectedComponentLabelingResultLSLImplPtrPtr result)
{
	COMPV_CHECK_EXP_RETURN(!result, COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	CompVConnectedComponentLabelingResultLSLImplPtr result_ = new CompVConnectedComponentLabelingResultLSLImpl();
	COMPV_CHECK_EXP_RETURN(!result_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

	*result = result_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
