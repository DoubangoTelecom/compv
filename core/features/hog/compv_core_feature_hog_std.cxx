/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/features/hog/compv_core_feature_hog_std.h"
#include "compv/base/time/compv_time.h"
#include "compv/base/image/compv_image.h"
#include "compv/base/math/compv_math_utils.h"
#include "compv/base/parallel/compv_parallel.h"

#define COMPV_THIS_CLASSNAME	"CompVHogStd"

// Documentation (HOG Standard):
//	- https://en.wikipedia.org/wiki/Histogram_of_oriented_gradients
//  - https://www2.cs.duke.edu/courses/fall15/compsci527/notes/hog.pdf
//	- http://lear.inrialpes.fr/people/triggs/pubs/Dalal-cvpr05.pdf

COMPV_NAMESPACE_BEGIN()

CompVHogStd::CompVHogStd(const CompVSizeSz& blockSize,
	const CompVSizeSz& blockStride,
	const CompVSizeSz& cellSize,
	const size_t nbins,
	const int blockNorm,
	const bool gradientSigned)
	: CompVHOG(COMPV_HOGS_ID)
	, m_szBlockSize(blockSize)
	, m_szBlockStride(blockStride)
	, m_szCellSize(cellSize)
	, m_nbins(nbins)
	, m_nBlockNorm(blockNorm)
	, m_bGradientSigned(gradientSigned)
{

}

CompVHogStd::~CompVHogStd()
{

}

COMPV_ERROR_CODE CompVHogStd::set(int id, const void* valuePtr, size_t valueSize) /*Overrides(CompVCaps)*/
{
	COMPV_CHECK_EXP_RETURN(!valuePtr || !valueSize, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	switch (id) {
	case COMPV_HOG_SET_BOOL_GRADIENT_SIGNED: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(bool), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		/* *reinterpret_cast<const bool*>(valuePtr); */
		return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
	}
	case COMPV_HOG_SET_INT_BLOCK_NORM: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		const int blockNorm = *reinterpret_cast<const int*>(valuePtr);
		return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
	}
	case COMPV_HOG_SET_INT_NBINS: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int) || *reinterpret_cast<const int*>(valuePtr) <= 0, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		const int nbins = *reinterpret_cast<const int*>(valuePtr);
		return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
	}
	default: {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Set with id %d not implemented", id);
		return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
	}
	}
}

COMPV_ERROR_CODE CompVHogStd::get(int id, const void** valuePtrPtr, size_t valueSize) /*Overrides(CompVCaps)*/
{
	COMPV_CHECK_EXP_RETURN(!valuePtrPtr || valueSize == 0, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
#if 0
	switch (id) {
	default:
#endif
		return CompVCaps::get(id, valuePtrPtr, valueSize);
#if 0
	}
#endif
}

COMPV_ERROR_CODE CompVHogStd::process(const CompVMatPtr& input, CompVMatPtrPtr output) /*Overrides(CompVHOG)*/
{
	COMPV_CHECK_EXP_RETURN(!input || !output, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_EXP_RETURN(input->elmtInBytes() != sizeof(uint8_t) || input->planeCount() != 1, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "input must be 8U_1D (e.g. grayscale image)");

	return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
}

COMPV_ERROR_CODE CompVHogStd::newObj(
	CompVHOGPtrPtr hog,
	const CompVSizeSz& blockSize COMPV_DEFAULT(CompVSizeSz(16, 16)),
	const CompVSizeSz& blockStride COMPV_DEFAULT(CompVSizeSz(8, 8)),
	const CompVSizeSz& cellSize COMPV_DEFAULT(CompVSizeSz(8, 8)),
	const size_t nbins COMPV_DEFAULT(9),
	const int blockNorm COMPV_DEFAULT(COMPV_HOG_BLOCK_NORM_L2HYST),
	const bool gradientSigned COMPV_DEFAULT(true))
{
	COMPV_CHECK_EXP_RETURN(!hog, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_CODE_RETURN(CompVHOG::checkParams(blockSize, blockStride, cellSize, nbins, blockNorm, gradientSigned));
	CompVHOGPtr hog_ = new CompVHogStd(blockSize, blockStride, cellSize, nbins, blockNorm, gradientSigned);
	COMPV_CHECK_EXP_RETURN(!hog_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

	*hog = *hog_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
