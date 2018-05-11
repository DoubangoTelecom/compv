/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/features/hog/compv_core_feature_hog_std.h"
#include "compv/base/compv_gradient_fast.h"
#include "compv/base/math/compv_math_utils.h"
#include "compv/base/parallel/compv_parallel.h"

#define COMPV_THIS_CLASSNAME	"CompVHogStd"

#define COMPV_HOG_BUILD_CELL_HIST_SAMPLES_PER_THREAD			(4 * 4) // unit=cells

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

	const CompVSizeSz szWinSize = CompVSizeSz(input->cols(), input->rows());
	COMPV_CHECK_EXP_RETURN(szWinSize.width < m_szBlockSize.width || szWinSize.height < m_szBlockSize.height, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "winSize < blockSize");

	// Compute output (features/descriptor) size
	size_t nOutSize;
	COMPV_CHECK_CODE_RETURN(CompVHOG::descriptorSize(
		szWinSize, // winSize
		m_szBlockSize, // blockSize,
		m_szBlockStride, // blockStride,
		m_szCellSize, // cellSize,
		m_nbins,
		&nOutSize));
	COMPV_DEBUG_VERBOSE_EX(COMPV_THIS_CLASSNAME, "Descriptor size = %zu", nOutSize);

	// Compute bin width
	const int nBinWidth = static_cast<int>((m_bGradientSigned ? 360 : 180) / m_nbins);

	// Compute magnitudes and directions: 
	//	- https://en.wikipedia.org/wiki/Histogram_of_oriented_gradients#Gradient_computation
	//	- https://www2.cs.duke.edu/courses/fall15/compsci527/notes/hog.pdf, Gradient
	CompVMatPtr gx, gy, magnitude, direction;
	COMPV_CHECK_CODE_RETURN(CompVGradientFast::gradX<compv_hog_floattype_t>(input, &gx));
	COMPV_CHECK_CODE_RETURN(CompVGradientFast::gradY<compv_hog_floattype_t>(input, &gy));
	COMPV_CHECK_CODE_RETURN(CompVGradientFast::magnitude(gx, gy, &magnitude));
	COMPV_CHECK_CODE_RETURN(CompVGradientFast::direction(gx, gy, &direction, true));
	const size_t magnitudeStride = magnitude->stride();
	const size_t directionStride = direction->stride();
	gx = nullptr;
	gy = nullptr;

	// Compute Orientation binning: 
	//	- https://en.wikipedia.org/wiki/Histogram_of_oriented_gradients#Orientation_binning
	//	- https://www2.cs.duke.edu/courses/fall15/compsci527/notes/hog.pdf, Cell Orientation Histograms
	const int numCellsY = static_cast<int>(szWinSize.height / m_szCellSize.height);
	const int numCellsX = static_cast<int>(szWinSize.width / m_szCellSize.width);
	std::vector<std::vector<compv_hog_vector_t> > vecCellsHist(numCellsY);
	auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		const compv_hog_floattype_t* magPtr = magnitude->ptr<compv_hog_floattype_t>(ystart);
		const compv_hog_floattype_t* dirPtr = direction->ptr<compv_hog_floattype_t>(ystart);
		int i;
		size_t x;
		for (size_t j = ystart; j < yend; ++j) {
			vecCellsHist[j].resize(numCellsX);
			for (i = 0, x = 0; i < numCellsX; ++i, x += m_szCellSize.width) {
				COMPV_CHECK_CODE_RETURN(
					CompVHogStd::buildCellHist(&magPtr[x], &dirPtr[x],
						m_szCellSize.width, m_szCellSize.height, magnitudeStride, directionStride,
					m_bGradientSigned, nBinWidth, static_cast<int>(m_nbins), vecCellsHist[j][i]
				));
			}
		}
		return COMPV_ERROR_CODE_S_OK;
	};
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtr,
		numCellsX,
		numCellsY,
		COMPV_HOG_BUILD_CELL_HIST_SAMPLES_PER_THREAD
	));
	
	// FIXME(dmi):
	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Remove next code");
	*output = direction;

	return COMPV_ERROR_CODE_S_OK;
}

//	- https://www2.cs.duke.edu/courses/fall15/compsci527/notes/hog.pdf, Cell Orientation Histograms
COMPV_ERROR_CODE CompVHogStd::buildCellHist(const compv_hog_floattype_t* magPtr, const compv_hog_floattype_t* dirPtr,
	const size_t& cellWidth, const size_t& cellHeight, const size_t& magStride, const size_t& dirStride,
	const bool gradientSigned, const int binWidth, const int binCount, compv_hog_vector_t& cellHist)
{
	// Private function, do not check input params

	cellHist.resize(binCount, 0);

	COMPV_DEBUG_INFO_CODE_TODO("Compute histogram according to //	- https://www2.cs.duke.edu/courses/fall15/compsci527/notes/hog.pdf, Cell Orientation Histograms");
	const compv_hog_floattype_t dirMax = static_cast<compv_hog_floattype_t>(gradientSigned ? 360 : 180);

	for (size_t j = 0; j < cellHeight; ++j) {
		for (size_t i = 0; i < cellWidth; ++i) {
			const int binIdx = static_cast<int>(((dirPtr[i] >= dirMax)
				? (dirPtr[i] - dirMax)
				: dirPtr[i]) / binWidth);
#if ((defined(_DEBUG) && _DEBUG != 0) || (defined(DEBUG) && DEBUG != 0))
			COMPV_ASSERT(binIdx >= 0 && binIdx < binCount);
#endif
			cellHist[binIdx] += magPtr[i];
		}
		magPtr += magStride;
		dirPtr += dirStride;
	}
	return COMPV_ERROR_CODE_S_OK;
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
