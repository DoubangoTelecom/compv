/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/ccl/compv_core_ccl_lmser_result.h"
#include "compv/base/parallel/compv_parallel.h"
#include "compv/base/compv_cpu.h"
#include "compv/base/compv_memz.h"

#define COMPV_THIS_CLASSNAME	"ConnectedComponentLabelingResultLMSERImpl"

COMPV_NAMESPACE_BEGIN()

ConnectedComponentLabelingResultLMSERImpl::ConnectedComponentLabelingResultLMSERImpl()
{

}

ConnectedComponentLabelingResultLMSERImpl::~ConnectedComponentLabelingResultLMSERImpl()
{

}

size_t ConnectedComponentLabelingResultLMSERImpl::labelsCount() const /*override*/
{
	return 0;
}

COMPV_ERROR_CODE ConnectedComponentLabelingResultLMSERImpl::debugFlatten(CompVMatPtrPtr ptr32sLabels) const /*override*/
{
	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE ConnectedComponentLabelingResultLMSERImpl::extract(CompVConnectedComponentPointsVector& points, COMPV_CCL_EXTRACT_TYPE type COMPV_DEFAULT(COMPV_CCL_EXTRACT_TYPE_BLOB)) const /*override*/
{
	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE ConnectedComponentLabelingResultLMSERImpl::boundingBoxes(CompVConnectedComponentBoundingBoxesVector& boxes) const /*override*/
{
	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE ConnectedComponentLabelingResultLMSERImpl::moments(CompVConnectedComponentMoments& moments) const /*override*/
{
	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE ConnectedComponentLabelingResultLMSERImpl::reset()
{
	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE ConnectedComponentLabelingResultLMSERImpl::newObj(CompVConnectedComponentLabelingResultLMSERImplPtrPtr result)
{
	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
