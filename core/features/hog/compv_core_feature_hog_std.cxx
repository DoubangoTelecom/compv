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
#define COMPV_HOG_BUILD_BLOCK_DESC_SAMPLES_PER_THREAD			(1 * 1) // unit=cells

// Documentation (HOG Standard):
//	- https://en.wikipedia.org/wiki/Histogram_of_oriented_gradients
//  - https://www2.cs.duke.edu/courses/fall15/compsci527/notes/hog.pdf
//	- http://lear.inrialpes.fr/people/triggs/pubs/Dalal-cvpr05.pdf

COMPV_NAMESPACE_BEGIN()

static const compv_hog_floattype_t COMPV_HOG_EPSILON = compv_hog_floattype_t(1e-6);
static const compv_hog_floattype_t COMPV_HOG_EPSILON2 = COMPV_HOG_EPSILON * COMPV_HOG_EPSILON;

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
	const size_t nMagnitudeStride = magnitude->stride();
	const size_t nDirectionStride = direction->stride();
	gx = nullptr;
	gy = nullptr;

	// Compute Orientation binning: 
	//	- https://en.wikipedia.org/wiki/Histogram_of_oriented_gradients#Orientation_binning
	//	- https://www2.cs.duke.edu/courses/fall15/compsci527/notes/hog.pdf, Cell Orientation Histograms
	const int numCellsX = static_cast<int>(szWinSize.width / m_szCellSize.width);
	const int numCellsY = static_cast<int>(szWinSize.height / m_szCellSize.height);
	CompVMatPtr mapHist;
	const size_t nMapWidth = numCellsX * m_nbins;
	const size_t nMapHeight = numCellsY;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_hog_floattype_t>(&mapHist, nMapHeight, nMapWidth)); // numCellsY * numBinsX
	const size_t nMapHistStride = mapHist->stride();
	// Multithreading - Process
	auto funcPtrOrientationBinning = [&](const size_t cellYstart, const size_t cellYend) -> COMPV_ERROR_CODE {
		const compv_hog_floattype_t* magPtr = magnitude->ptr<compv_hog_floattype_t>(cellYstart * m_szCellSize.height);
		const compv_hog_floattype_t* dirPtr = direction->ptr<compv_hog_floattype_t>(cellYstart * m_szCellSize.height);
		compv_hog_floattype_t* mapHistPtr = mapHist->ptr<compv_hog_floattype_t>(cellYstart);
		const size_t nMagnitudeOffset = nMagnitudeStride * m_szCellSize.height;
		const size_t nDirectionOffset = nDirectionStride * m_szCellSize.height;
		int i;
		size_t x;
		for (size_t j = cellYstart; j < cellYend; ++j) {
			for (i = 0, x = 0; i < numCellsX; ++i, x += m_szCellSize.width) {
				COMPV_CHECK_CODE_RETURN(
					CompVHogStd::buildMapHistForSingleCell(&mapHistPtr[i * m_nbins], &magPtr[x], &dirPtr[x],
						m_szCellSize.width, m_szCellSize.height, nMagnitudeStride, nDirectionStride,
						m_bGradientSigned, nBinWidth, static_cast<int>(m_nbins)
				));
			}
			mapHistPtr += nMapHistStride;
			magPtr += nMagnitudeOffset;
			dirPtr += nDirectionOffset;
		}
		return COMPV_ERROR_CODE_S_OK;
	};
#if 1
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtrOrientationBinning,
		numCellsX,
		numCellsY,
		COMPV_HOG_BUILD_CELL_HIST_SAMPLES_PER_THREAD
	));
#else
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No MT-implementation could be found");
	COMPV_CHECK_CODE_RETURN(funcPtrOrientationBinning(
		0, numCellsY
	));
#endif

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

	// Descriptor blocks + Block normalization
	//	- https://en.wikipedia.org/wiki/Histogram_of_oriented_gradients#Descriptor_blocks
	//	- https://en.wikipedia.org/wiki/Histogram_of_oriented_gradients#Block_normalization
	//	- https://www2.cs.duke.edu/courses/fall15/compsci527/notes/hog.pdf - Block Normalization
	int numBlocksX = 0;
	int numBlocksY = 0;
	for (size_t blockX = 0; blockX <= szWinSize.width - m_szBlockSize.width; blockX += m_szBlockStride.width) ++numBlocksX;
	for (size_t blockY = 0; blockY <= szWinSize.height - m_szBlockSize.height; blockY += m_szBlockStride.height) ++numBlocksY;
	const size_t numBlocks = numBlocksX * numBlocksY;
	COMPV_ASSERT(numBlocks > 0);

	COMPV_ASSERT(!(m_szBlockSize.width % m_szCellSize.width));
	COMPV_ASSERT(!(m_szBlockSize.height % m_szCellSize.height));
	const size_t numCellsPerBlockX = (m_szBlockSize.width / m_szCellSize.width);
	const size_t numCellsPerBlockY = (m_szBlockSize.height / m_szCellSize.height);
	const size_t numBinsPerBlockX = (numCellsPerBlockX * m_nbins);
	const size_t nBlockBinsCount = numCellsPerBlockY * numBinsPerBlockX;
	const size_t nBlocksTotalBinsCount = numBlocks * nBlockBinsCount;
	COMPV_ASSERT(nBlocksTotalBinsCount == nOutSize);
	const size_t nMaxBlockX = szWinSize.width - m_szBlockSize.width;

	COMPV_ASSERT(!(m_szCellSize.width % m_szBlockStride.width) && !(m_szCellSize.height % m_szBlockStride.height));
	const CompVSizeSz szBlockStrideInCellsCount = { (m_szCellSize.width / m_szBlockStride.width), (m_szCellSize.height / m_szBlockStride.height) };

	// Create output
	CompVMatPtr output_ = *output;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_hog_floattype_t>(&output_, 1, nOutSize));

	// Set norm function
	COMPV_ERROR_CODE(*normFunc)(compv_hog_floattype_t* inOutPtr, const size_t& count) = nullptr;
	switch (m_nBlockNorm) {
	case COMPV_HOG_BLOCK_NORM_L1: normFunc = CompVHogStd::normL1; break;
	case COMPV_HOG_BLOCK_NORM_L1SQRT: normFunc = CompVHogStd::normL1Sqrt; break;
	case COMPV_HOG_BLOCK_NORM_L2: normFunc = CompVHogStd::normL2; break;
	case COMPV_HOG_BLOCK_NORM_L2HYS: normFunc = CompVHogStd::normL2Hys; break;
	case COMPV_HOG_BLOCK_NORM_NONE: break;
	default: COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED, "Supported norm functions: none,L1"); break;
	}
	
	// Multithreading - Process
	auto funcPtrBlockDescriptorAndNorm = [&](const size_t blockYstart, const size_t blockYend) -> COMPV_ERROR_CODE {
		compv_hog_floattype_t* outputPtr = output_->ptr<compv_hog_floattype_t>(0, (blockYstart * (numBlocksX * nBlockBinsCount)));
		const compv_hog_floattype_t* mapHistPtr = mapHist->ptr<const compv_hog_floattype_t>((blockYstart * szBlockStrideInCellsCount.height));
		const size_t nBlockStrideInBinsXCount = szBlockStrideInCellsCount.width * m_nbins;
		const size_t nMapHistStrideBlock = nMapHistStride * szBlockStrideInCellsCount.height;
		for (size_t blockY = blockYstart; blockY < blockYend; ++blockY) {
			for (size_t blockX = 0, binX = 0; blockX <= nMaxBlockX; blockX += m_szBlockStride.width, binX += nBlockStrideInBinsXCount) {
				// Build vector for a block at position (blocX, blockY)
				COMPV_CHECK_CODE_RETURN(CompVHogStd::buildOutputForSingleBlock(
					&mapHistPtr[binX], outputPtr,
					numCellsPerBlockY, numBinsPerBlockX, nMapHistStride
				));
				// Normalize the output vector
				if (normFunc) {
					COMPV_CHECK_CODE_RETURN(normFunc(outputPtr, nBlockBinsCount));
				}
				outputPtr += nBlockBinsCount;
			}
			mapHistPtr += nMapHistStrideBlock;
		}
		return COMPV_ERROR_CODE_S_OK;
	};
#if 1
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtrBlockDescriptorAndNorm,
		numBlocksX,
		numBlocksY,
		COMPV_HOG_BUILD_BLOCK_DESC_SAMPLES_PER_THREAD
	));
#else
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No MT-implementation could be found");
	COMPV_CHECK_CODE_RETURN(funcPtrBlockDescriptorAndNorm(
		0, numBlocksY
	));
#endif

	*output = output_;
	return COMPV_ERROR_CODE_S_OK;
}

// Compute Orientation binning: 
//	- https://en.wikipedia.org/wiki/Histogram_of_oriented_gradients#Orientation_binning
//	- https://www2.cs.duke.edu/courses/fall15/compsci527/notes/hog.pdf, Cell Orientation Histograms
COMPV_ERROR_CODE CompVHogStd::buildMapHistForSingleCell(compv_hog_floattype_t* mapHistPtr, const compv_hog_floattype_t* magPtr, const compv_hog_floattype_t* dirPtr,
	const size_t& cellWidth, const size_t& cellHeight, const size_t& magStride, const size_t& dirStride,
	const bool gradientSigned, const int binWidth, const int binCount
)
{
	// Private function, do not check input params

	// Reset histogram for the current cell
	for (int i = 0; i < binCount; ++i) {
		mapHistPtr[i] = 0;
	}

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
			mapHistPtr[binIdx] += magPtr[i];
		}
		magPtr += magStride;
		dirPtr += dirStride;
	}
	return COMPV_ERROR_CODE_S_OK;
}

// Descriptor blocks + Block normalization
//	- https://en.wikipedia.org/wiki/Histogram_of_oriented_gradients#Descriptor_blocks
//	- https://en.wikipedia.org/wiki/Histogram_of_oriented_gradients#Block_normalization
//	- https://www2.cs.duke.edu/courses/fall15/compsci527/notes/hog.pdf - Block Normalization
COMPV_ERROR_CODE CompVHogStd::buildOutputForSingleBlock(
	const compv_hog_floattype_t* mapHistPtr, compv_hog_floattype_t* outputPtr,
	const size_t& numCellsPerBlockY, const size_t& numBinsPerBlockX, const size_t& mapHistStride
)
{
	// Private function, do not check input params
	
	const size_t numBinsPerBlockX8 = numBinsPerBlockX & -8;
	size_t binX;
	for (size_t blockCellY = 0; blockCellY < numCellsPerBlockY; ++blockCellY) {
		// TODO(dmi): norm : none
		for (binX = 0; binX < numBinsPerBlockX8; binX += 8) {
			outputPtr[binX] = mapHistPtr[binX];
			outputPtr[binX + 1] = mapHistPtr[binX + 1];
			outputPtr[binX + 2] = mapHistPtr[binX + 2];
			outputPtr[binX + 3] = mapHistPtr[binX + 3];
			outputPtr[binX + 4] = mapHistPtr[binX + 4];
			outputPtr[binX + 5] = mapHistPtr[binX + 5];
			outputPtr[binX + 6] = mapHistPtr[binX + 6];
			outputPtr[binX + 7] = mapHistPtr[binX + 7];
		}
		for (; binX < numBinsPerBlockX; ++binX) {
			outputPtr[binX] = mapHistPtr[binX];
		}
		outputPtr += numBinsPerBlockX;
		mapHistPtr += mapHistStride;
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVHogStd::normL1(compv_hog_floattype_t* inOutPtr, const size_t& count)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");

	// Compute den = sum(vector) + eps
	compv_hog_floattype_t den = 0; // vector contains small values -> no need to use double for accumulation
	for (size_t i = 0; i < count; ++i) {
		den += inOutPtr[i]; // no need for abs because hist are always >= 0
	}
	den = 1 / (den + COMPV_HOG_EPSILON);
	// Compute norm = v * (1 / den)
	for (size_t i = 0; i < count; ++i) {
		inOutPtr[i] *= den;
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVHogStd::normL1Sqrt(compv_hog_floattype_t* inOutPtr, const size_t& count)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");

	// Compute den = sum(vector) + eps
	compv_hog_floattype_t den = 0; // vector contains small values -> no need to use double for accumulation
	for (size_t i = 0; i < count; ++i) {
		den += inOutPtr[i]; // no need for abs because hist are always >= 0
	}
	den = 1 / (den + COMPV_HOG_EPSILON);
	// Compute norm = sqrt(v * (1 / den))
	for (size_t i = 0; i < count; ++i) {
		inOutPtr[i] = std::sqrt(inOutPtr[i] * den);
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVHogStd::normL2(compv_hog_floattype_t* inOutPtr, const size_t& count)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");

	// Compute den = L2(vector)^2 + eps^2
	compv_hog_floattype_t den = 0; // vector contains small values -> no need to use double for accumulation
	for (size_t i = 0; i < count; ++i) {
		den += inOutPtr[i] * inOutPtr[i];
	}
	den = 1 / std::sqrt(den + COMPV_HOG_EPSILON2);
	// Compute norm = v * (1 / den)
	for (size_t i = 0; i < count; ++i) {
		inOutPtr[i] *= den;
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVHogStd::normL2Hys(compv_hog_floattype_t* inOutPtr, const size_t& count)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");

	COMPV_CHECK_CODE_RETURN(CompVHogStd::normL2(inOutPtr, count));
	for (size_t i = 0; i < count; ++i) {
		if (inOutPtr[i] > 0.2f) {
			inOutPtr[i] = 0.2f;
		}
	}
	COMPV_CHECK_CODE_RETURN(CompVHogStd::normL2(inOutPtr, count));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVHogStd::newObj(
	CompVHOGPtrPtr hog,
	const CompVSizeSz& blockSize COMPV_DEFAULT(CompVSizeSz(16, 16)),
	const CompVSizeSz& blockStride COMPV_DEFAULT(CompVSizeSz(8, 8)),
	const CompVSizeSz& cellSize COMPV_DEFAULT(CompVSizeSz(8, 8)),
	const size_t nbins COMPV_DEFAULT(9),
	const int blockNorm COMPV_DEFAULT(COMPV_HOG_BLOCK_NORM_L2HYS),
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
