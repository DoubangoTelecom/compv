/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/

#include "compv/features/edges/compv_feature_edge_dete.h"
#include "compv/math/compv_math_convlt.h"
#include "compv/math/compv_math_utils.h"

COMPV_NAMESPACE_BEGIN()

// https://en.wikipedia.org/wiki/Sobel_operator#Alternative_operators
static const int16_t ScharrGx_vt[3] = { 3, 10, 3 };
static const int16_t ScharrGx_hz[3] = { -1, 0, 1 };
// https://en.wikipedia.org/wiki/Sobel_operator
static const int16_t SobelGx_vt[3] = { 1, 2, 1 };
static const int16_t SobelGx_hz[3] = { 1, 0, -1 };
// https://en.wikipedia.org/wiki/Prewitt_operator
static const int16_t PrewittGx_vt[3] = { 1, 1, 1 };
static const int16_t PrewittGx_hz[3] = { -1, 0, 1 };

CompVEdgeDeteBASE::CompVEdgeDeteBASE(int id, const int16_t* kernelPtrVt, const int16_t* kernelPtrHz, size_t kernelSize)
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

CompVEdgeDeteBASE::~CompVEdgeDeteBASE()
{
	CompVMem::free((void**)&m_pGx);
	CompVMem::free((void**)&m_pGy);
	CompVMem::free((void**)&m_pG);
}

// overrides CompVSettable::set
COMPV_ERROR_CODE CompVEdgeDeteBASE::set(int id, const void* valuePtr, size_t valueSize)
{
	COMPV_CHECK_EXP_RETURN(valuePtr == NULL || valueSize == 0, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	switch (id) {
	case -1:
	default:
		return CompVSettable::set(id, valuePtr, valueSize);
	}
}

// overrides CompVEdgeDete::process
COMPV_ERROR_CODE CompVEdgeDeteBASE::process(const CompVPtr<CompVImage*>& image, CompVPtrArray(uint8_t)& egdes)
{
	COMPV_CHECK_EXP_RETURN(!image || image->getPixelFormat() != COMPV_PIXEL_FORMAT_GRAYSCALE, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // MT

	// Realloc pGx and pGy if image size changes
	if (image->getWidth() != m_nImageWidth || image->getHeight() != m_nImageHeight || image->getStride() != m_nImageStride) {
		CompVMem::free((void**)&m_pGx);
		CompVMem::free((void**)&m_pGy);
		CompVMem::free((void**)&m_pG);
		m_nImageWidth = static_cast<size_t>(image->getWidth());
		m_nImageHeight = static_cast<size_t>(image->getHeight());
		m_nImageStride = static_cast<size_t>(image->getStride());
	}
	// Convolution
	COMPV_CHECK_CODE_RETURN((CompVMathConvlt::convlt1<uint8_t, int16_t, int16_t>((const uint8_t*)image->getDataPtr(), m_nImageWidth, m_nImageStride, m_nImageHeight, m_pcKernelVt, m_pcKernelHz, m_nKernelSize, m_pGx)));
	COMPV_CHECK_CODE_RETURN((CompVMathConvlt::convlt1<uint8_t, int16_t, int16_t>((const uint8_t*)image->getDataPtr(), m_nImageWidth, m_nImageStride, m_nImageHeight, m_pcKernelHz, m_pcKernelVt, m_nKernelSize, m_pGy)));
	
	// Compute gradiant using L1 distance
	uint16_t gmax = 1;
	COMPV_CHECK_CODE_RETURN((CompVMathUtils::gradientL1<int16_t, uint16_t>(m_pGx, m_pGy, m_pG, gmax, m_nImageWidth, m_nImageHeight, m_nImageStride)));
	
	// Create edges buffer
	// edges must have same stride than m_pG (required by scaleAndClip)
	COMPV_CHECK_CODE_RETURN(CompVArray<uint8_t>::newObj(&egdes, m_nImageHeight, m_nImageWidth, COMPV_SIMD_ALIGNV_DEFAULT, m_nImageStride));

	// scale (normalization)
	float scale = 255.f / float(gmax);
	uint8_t* edgesPtr = const_cast<uint8_t*>(egdes->ptr());
	COMPV_CHECK_CODE_RETURN((CompVMathUtils::scaleAndClip<uint16_t, float, uint8_t>(m_pG, scale, edgesPtr, 0, 255, m_nImageWidth, m_nImageHeight, m_nImageStride)));

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVEdgeDeteBASE::newObjSobel(CompVPtr<CompVEdgeDete* >* dete)
{
	return CompVEdgeDeteBASE::newObj(dete, COMPV_SOBEL_ID);
}

COMPV_ERROR_CODE CompVEdgeDeteBASE::newObjScharr(CompVPtr<CompVEdgeDete* >* dete)
{
	return CompVEdgeDeteBASE::newObj(dete, COMPV_SCHARR_ID);
}

COMPV_ERROR_CODE CompVEdgeDeteBASE::newObjPrewitt(CompVPtr<CompVEdgeDete* >* dete)
{
	return CompVEdgeDeteBASE::newObj(dete, COMPV_PREWITT_ID);
}

COMPV_ERROR_CODE CompVEdgeDeteBASE::newObj(CompVPtr<CompVEdgeDete* >* dete, int id)
{
	COMPV_CHECK_EXP_RETURN(!dete, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	const int16_t *kernelPtrVt_ = NULL, *kernelPtrHz_ = NULL;
	size_t kernelSize_ = 0;
	switch (id)
	{
	case COMPV_SOBEL_ID: 
		kernelPtrVt_ = SobelGx_vt; 
		kernelPtrHz_ = SobelGx_hz;
		kernelSize_ = 3;
		break;
	case COMPV_SCHARR_ID:
		kernelPtrVt_ = ScharrGx_vt;
		kernelPtrHz_ = ScharrGx_hz;
		kernelSize_ = 3;
		break;
	case COMPV_PREWITT_ID:
		kernelPtrVt_ = PrewittGx_vt;
		kernelPtrHz_ = PrewittGx_hz;
		kernelSize_ = 3;
		break;
	default:
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		break;
	}

	CompVPtr<CompVEdgeDeteBASE* >dete_ = new CompVEdgeDeteBASE(id, kernelPtrVt_, kernelPtrHz_, kernelSize_);
	COMPV_CHECK_EXP_RETURN(!dete_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	*dete = *dete_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
