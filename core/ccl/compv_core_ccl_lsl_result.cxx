/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/ccl/compv_core_ccl_lsl_result.h"
#include "compv/base/compv_debug.h"

#define COMPV_THIS_CLASSNAME "CompVConnectedComponentLabelingResultLSLImpl"

COMPV_NAMESPACE_BEGIN()

CompVConnectedComponentLabelingResultLSLImpl::CompVConnectedComponentLabelingResultLSLImpl()
	: m_szInput(CompVSizeSz(0, 0))
{
}

CompVConnectedComponentLabelingResultLSLImpl::~CompVConnectedComponentLabelingResultLSLImpl()
{

}

size_t CompVConnectedComponentLabelingResultLSLImpl::labelsCount() const
{
	return (m_ptrxNa ? m_ptrxNa->cols() : 0);
}

COMPV_ERROR_CODE CompVConnectedComponentLabelingResultLSLImpl::debugFlatten(CompVMatPtrPtr ptr32sLabels) const
{
	COMPV_CHECK_EXP_RETURN(!ptr32sLabels, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_EXP_RETURN(
		!m_szInput.width || !m_szInput.height ||
		!m_ptrxNa ||
		!m_ptr32sERA || m_ptr32sERA->rows() != m_szInput.height ||
		!m_ptr32sA || m_ptr32sA->rows() != 1 ||
		!m_ptr16sRLC || m_ptr16sRLC->rows() != m_szInput.height || m_ptr16sRLC->cols() >= m_szInput.width ||
		!m_ptr16sNer || m_ptr16sNer->cols() != m_szInput.height,
		COMPV_ERROR_CODE_E_INVALID_STATE
	);

	COMPV_DEBUG_INFO_CODE_FOR_TESTING("This function is needed for debugging only and you should not call it");
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No MT implementation found");

	// Create EA using same size and stride as ER
	CompVMatPtr ptr32sEA = *ptr32sLabels;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjStrideless<int32_t>(&ptr32sEA, m_szInput.height, m_szInput.width));
	COMPV_CHECK_CODE_RETURN(ptr32sEA->zero_all());

	const int16_t* ner = m_ptr16sNer->ptr<const int16_t>();
	const int16_t* RLC = m_ptr16sRLC->ptr<const int16_t>();
	const int32_t* ERA = m_ptr32sERA->ptr();
	const int32_t* A = m_ptr32sA->ptr<const int32_t>();
	int32_t* EA = ptr32sEA->ptr<int32_t>();
	const size_t ERA_stride = m_ptr32sERA->stride();
	const size_t EA_stride = ptr32sEA->stride();
	const size_t RLC_stride = m_ptr16sRLC->stride();

	// #3 and #5 merged 
	// step #3: First absolute labeling
	// step #5: Second absolute labeling
	for (size_t j = 0; j < m_szInput.height; ++j) {
		COMPV_CHECK_CODE_RETURN(ptr32sEA->zero_row(j)); // background is "0"
		const int16_t ner1 = ner[j];
		for (int16_t er = 1; er < ner1; er += 2) {
			const int32_t ea = ERA[er];
			if (ea) { // "0" is background
				const int32_t a = A[ea];
				const int16_t er0 = RLC[er - 1];
				const int16_t er1 = RLC[er];
				for (int16_t e = er0; e < er1; ++e) {
					EA[e] = a;
				}
			}
			else {
				int kaka = 0;
			}
		}
		RLC += RLC_stride;
		EA += EA_stride;
		ERA += ERA_stride;
	}

	*ptr32sLabels = ptr32sEA;

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
