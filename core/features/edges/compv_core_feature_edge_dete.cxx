/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/features/edges/compv_core_feature_edge_dete.h"
#include "compv/base/math/compv_math_utils.h"
#include "compv/base/math/compv_math_convlt.h"
#include "compv/base/math/compv_math_gauss.h"
#include "compv/base/image/compv_image.h"
#include "compv/base/parallel/compv_parallel.h"

#define COMPV_THIS_CLASSNAME	"CompVCornerDeteEdgeBase"

COMPV_NAMESPACE_BEGIN()

CompVCornerDeteEdgeBase::CompVCornerDeteEdgeBase(int id, const int16_t* kernelPtrVt, const int16_t* kernelPtrHz, size_t kernelSize)
	: CompVEdgeDete(id)
	, m_pcKernelVt(kernelPtrVt)
	, m_pcKernelHz(kernelPtrHz)
	, m_nKernelSize(kernelSize)
	, m_nImageWidth(0)
	, m_nImageHeight(0)
	, m_nImageStride(0)
	, m_pGx(NULL)
	, m_pGy(NULL)
	, m_pG(NULL)
{

}

CompVCornerDeteEdgeBase::~CompVCornerDeteEdgeBase()
{
	CompVMem::free(reinterpret_cast<void**>(&m_pGx));
	CompVMem::free(reinterpret_cast<void**>(&m_pGy));
	CompVMem::free(reinterpret_cast<void**>(&m_pG));
}

// overrides CompVSettable::set
COMPV_ERROR_CODE CompVCornerDeteEdgeBase::set(int id, const void* valuePtr, size_t valueSize)
{
	COMPV_CHECK_EXP_RETURN(!valuePtr || !valueSize, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	switch (id) {
	case 0:
	default:
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Set with id %d not implemented", id);
		return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
	}
}

// overrides CompVEdgeDete::process
COMPV_ERROR_CODE CompVCornerDeteEdgeBase::process(const CompVMatPtr& image, CompVMatPtrPtr edges) /*Overrides(CompVEdgeDete)*/
{
	COMPV_CHECK_EXP_RETURN(!image || image->subType() != COMPV_SUBTYPE_PIXELS_Y || !edges, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Input image is null or not in grayscale format");

	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No MT implementation");

	// Realloc pGx and pGy if image size changes
	if (image->cols() != m_nImageWidth || image->rows() != m_nImageHeight || image->stride() != m_nImageStride) {
		CompVMem::free(reinterpret_cast<void**>(&m_pGx));
		CompVMem::free(reinterpret_cast<void**>(&m_pGy));
		CompVMem::free(reinterpret_cast<void**>(&m_pG));
		m_nImageWidth = image->cols();
		m_nImageHeight = image->rows();
		m_nImageStride = image->stride();
	}
	// Convolution
	COMPV_CHECK_CODE_RETURN((CompVMathConvlt::convlt1<uint8_t, int16_t, int16_t>(image->ptr<const uint8_t>(), m_nImageWidth, m_nImageHeight, m_nImageStride, m_pcKernelVt, m_pcKernelHz, m_nKernelSize, m_pGx)));
	COMPV_CHECK_CODE_RETURN((CompVMathConvlt::convlt1<uint8_t, int16_t, int16_t>(image->ptr<const uint8_t>(), m_nImageWidth, m_nImageHeight, m_nImageStride, m_pcKernelHz, m_pcKernelVt, m_nKernelSize, m_pGy)));

	// Compute gradiant using L1 distance
	uint16_t gmax = 1;
	COMPV_CHECK_CODE_RETURN((CompVMathUtils::gradientL1<int16_t, uint16_t>(m_pGx, m_pGy, m_pG, m_nImageWidth, m_nImageHeight, m_nImageStride, &gmax)));

	// Create edges buffer
	// edges must have same stride than m_pG (required by scaleAndClip)
	COMPV_CHECK_CODE_RETURN(CompVImage::newObj8u(edges, COMPV_SUBTYPE_PIXELS_Y, m_nImageWidth, m_nImageHeight, m_nImageStride));

	// scale (normalization)
	float scale = 255.f / float(gmax);
	uint8_t* edgesPtr = (*edges)->ptr<uint8_t>();
	COMPV_CHECK_CODE_RETURN((CompVMathUtils::scaleAndClip<uint16_t, float, uint8_t>(m_pG, scale, edgesPtr, 0, 255, m_nImageWidth, m_nImageHeight, m_nImageStride)));

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVCornerDeteEdgeBase::newObjSobel(CompVEdgeDetePtrPtr dete, float tLow COMPV_DEFAULT(0.68f), float tHigh COMPV_DEFAULT(0.68f*2.f), size_t kernSize COMPV_DEFAULT(3))
{
	return CompVCornerDeteEdgeBase::newObj(dete, COMPV_SOBEL_ID);
}

COMPV_ERROR_CODE CompVCornerDeteEdgeBase::newObjScharr(CompVEdgeDetePtrPtr dete, float tLow COMPV_DEFAULT(0.68f), float tHigh COMPV_DEFAULT(0.68f*2.f), size_t kernSize COMPV_DEFAULT(3))
{
	return CompVCornerDeteEdgeBase::newObj(dete, COMPV_SCHARR_ID);
}

COMPV_ERROR_CODE CompVCornerDeteEdgeBase::newObjPrewitt(CompVEdgeDetePtrPtr dete, float tLow COMPV_DEFAULT(0.68f), float tHigh COMPV_DEFAULT(0.68f*2.f), size_t kernSize COMPV_DEFAULT(3))
{
	return CompVCornerDeteEdgeBase::newObj(dete, COMPV_PREWITT_ID);
}

COMPV_ERROR_CODE CompVCornerDeteEdgeBase::newObj(CompVEdgeDetePtrPtr dete, int id, float tLow COMPV_DEFAULT(0.68f), float tHigh COMPV_DEFAULT(0.68f*2.f), size_t kernSize COMPV_DEFAULT(3))
{
	COMPV_CHECK_EXP_RETURN(!dete, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	const int16_t *kernelPtrVt_ = NULL, *kernelPtrHz_ = NULL;
	size_t kernelSize_ = 0;
	switch (id) {
	case COMPV_SOBEL_ID:
		kernelPtrVt_ = CompVSobel3x3Gx_vt;
		kernelPtrHz_ = CompVSobel3x3Gx_hz;
		kernelSize_ = 3;
		break;
	case COMPV_SCHARR_ID:
		kernelPtrVt_ = CompVScharrGx_vt;
		kernelPtrHz_ = CompVScharrGx_hz;
		kernelSize_ = 3;
		break;
	case COMPV_PREWITT_ID:
		kernelPtrVt_ = CompVPrewittGx_vt;
		kernelPtrHz_ = CompVPrewittGx_hz;
		kernelSize_ = 3;
		break;
	default:
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Invalid dete identifier");
		break;
	}

	CompVCornerDeteEdgeBasePtr dete_ = new CompVCornerDeteEdgeBase(id, kernelPtrVt_, kernelPtrHz_, kernelSize_);
	COMPV_CHECK_EXP_RETURN(!dete_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	*dete = *dete_;
	return COMPV_ERROR_CODE_S_OK;
}


COMPV_NAMESPACE_END()
