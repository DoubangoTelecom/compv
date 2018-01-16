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

CompVConnectedComponentLabelingResultLMSERImpl::CompVConnectedComponentLabelingResultLMSERImpl()
	: m_bBoundingBoxesComputed(false)
{

}

CompVConnectedComponentLabelingResultLMSERImpl::~CompVConnectedComponentLabelingResultLMSERImpl()
{

}

size_t CompVConnectedComponentLabelingResultLMSERImpl::labelsCount() const /*override*/
{
	return m_vecRegions.size();
}

COMPV_ERROR_CODE CompVConnectedComponentLabelingResultLMSERImpl::debugFlatten(CompVMatPtrPtr ptr32sLabels) const /*override*/
{
	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVConnectedComponentLabelingResultLMSERImpl::extract(CompVConnectedComponentPointsVector& points, COMPV_CCL_EXTRACT_TYPE type COMPV_DEFAULT(COMPV_CCL_EXTRACT_TYPE_BLOB)) const /*override*/
{
	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}

const CompVConnectedComponentLabelingRegionMserVector& CompVConnectedComponentLabelingResultLMSERImpl::points() const /*override*/
{
	return m_vecRegions;
}

const CompVConnectedComponentLabelingRegionMserVector& CompVConnectedComponentLabelingResultLMSERImpl::boundingBoxes() const /*override*/
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No MT implementation could be found");

	if (m_bBoundingBoxesComputed || m_vecRegions.empty()) {
		return m_vecRegions;
	}

	CompVConnectedComponentLabelingResultLMSERImplPtr This = const_cast<CompVConnectedComponentLabelingResultLMSERImpl*>(this);
	CompVConnectedComponentLabelingRegionMserVector& vecRegions = This->vecRegions();

	for (CompVConnectedComponentLabelingRegionMserVector::iterator i = vecRegions.begin(); i < vecRegions.end(); ++i) {
		CompVConnectedComponentBoundingBox& bb = (*i).boundingBox;
		CompVConnectedComponentPoints& pp = (*i).points;
		bb.left = pp.begin()->x;
		bb.right = pp.begin()->x;
		bb.top = pp.begin()->y;
		bb.bottom = pp.begin()->y;
		for (CompVConnectedComponentPoints::const_iterator j = pp.begin() + 1; j < pp.end(); ++j) {
			bb.left = COMPV_MATH_MIN(bb.left, j->x);
			bb.top = COMPV_MATH_MIN(bb.top, j->y);
			bb.right = COMPV_MATH_MAX(bb.right, j->x);
			bb.bottom = COMPV_MATH_MAX(bb.bottom, j->y);
		}
	}
	
	This->m_bBoundingBoxesComputed = true;

	return m_vecRegions;
}

COMPV_ERROR_CODE CompVConnectedComponentLabelingResultLMSERImpl::reset()
{
	m_vecRegions.clear();
	m_bBoundingBoxesComputed = false;
	COMPV_CHECK_CODE_RETURN(m_StackMem.reset());
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVConnectedComponentLabelingResultLMSERImpl::newObj(CompVConnectedComponentLabelingResultLMSERImplPtrPtr result)
{
	COMPV_CHECK_EXP_RETURN(!result, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVConnectedComponentLabelingResultLMSERImplPtr result_ = new CompVConnectedComponentLabelingResultLMSERImpl();
	COMPV_CHECK_EXP_RETURN(!result_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

	*result = result_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
