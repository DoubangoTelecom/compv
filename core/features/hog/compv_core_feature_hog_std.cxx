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

#include "compv/core/features/hog/intrin/x86/compv_core_feature_hog_common_norm_intrin_sse2.h"

#define COMPV_THIS_CLASSNAME	"CompVHogStd"

#define COMPV_HOG_BUILD_CELL_HIST_SAMPLES_PER_THREAD			(4 * 4) // unit=cells
#define COMPV_HOG_BUILD_BLOCK_DESC_SAMPLES_PER_THREAD			(1 * 1) // unit=cells

#define COMPV_HOG_FAST_BLOCK_9									1

// Documentation (HOG Standard):
//	- https://en.wikipedia.org/wiki/Histogram_of_oriented_gradients
//  - https://www2.cs.duke.edu/courses/fall15/compsci527/notes/hog.pdf
//	- http://lear.inrialpes.fr/people/triggs/pubs/Dalal-cvpr05.pdf
//	- https://www.learnopencv.com/histogram-of-oriented-gradients/

COMPV_NAMESPACE_BEGIN()

#if COMPV_ASM && COMPV_ARCH_X64
COMPV_EXTERNC void CompVHogCommonNormL1_9_32f_Asm_X64_SSE2(compv_float32_t* inOutPtr, const compv_float32_t* eps1, const compv_uscalar_t count);
COMPV_EXTERNC void CompVHogCommonNormL1Sqrt_9_32f_Asm_X64_SSE2(compv_float32_t* inOutPtr, const compv_float32_t* eps1, const compv_uscalar_t count);
COMPV_EXTERNC void CompVHogCommonNormL2_9_32f_Asm_X64_SSE2(compv_float32_t* inOutPtr, const compv_float32_t* eps_square1, const compv_uscalar_t count);
COMPV_EXTERNC void CompVHogCommonNormL2Hys_9_32f_Asm_X64_SSE2(compv_float32_t* inOutPtr, const compv_float32_t* eps_square1, const compv_uscalar_t count);
#endif /* COMPV_ASM && COMPV_ARCH_X64 */

static const compv_hog_floattype_t COMPV_HOG_EPSILON = compv_hog_floattype_t(1e-6);
static const compv_hog_floattype_t COMPV_HOG_EPSILON2 = COMPV_HOG_EPSILON * COMPV_HOG_EPSILON;

CompVHogStd::CompVHogStd(const CompVSizeSz& blockSize,
	const CompVSizeSz& blockStride,
	const CompVSizeSz& cellSize,
	const size_t nbins,
	const int blockNorm,
	const bool gradientSigned,
	const int interp)
	: CompVHOG(COMPV_HOGS_ID)
	, m_szBlockSize(blockSize)
	, m_szBlockStride(blockStride)
	, m_szCellSize(cellSize)
	, m_nbins(nbins)
	, m_nBlockNorm(blockNorm)
	, m_bGradientSigned(gradientSigned)
	, m_nInterp(interp)
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
		m_bGradientSigned = *reinterpret_cast<const bool*>(valuePtr);
		return COMPV_ERROR_CODE_S_OK;
	}
	case COMPV_HOG_SET_INT_BLOCK_NORM: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		const int blockNorm = *reinterpret_cast<const int*>(valuePtr);
		COMPV_CHECK_EXP_RETURN(
			blockNorm != COMPV_HOG_BLOCK_NORM_NONE &&
			blockNorm != COMPV_HOG_BLOCK_NORM_L1 &&
			blockNorm != COMPV_HOG_BLOCK_NORM_L1SQRT &&
			blockNorm != COMPV_HOG_BLOCK_NORM_L2 &&
			blockNorm != COMPV_HOG_BLOCK_NORM_L2HYS,
			COMPV_ERROR_CODE_E_INVALID_PARAMETER,
			"blockNorm must be equal to COMPV_HOG_BLOCK_NORM_xxxx"
		);
		m_nBlockNorm = blockNorm;
		return COMPV_ERROR_CODE_S_OK;
	}
	case COMPV_HOG_SET_INT_NBINS: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		const int nbins = *reinterpret_cast<const int*>(valuePtr);
		COMPV_CHECK_EXP_RETURN(nbins <= 1 || nbins > 360, COMPV_ERROR_CODE_E_OUT_OF_BOUND, "NBINS must be within [2-360]");
		m_nbins = nbins;
		return COMPV_ERROR_CODE_S_OK;
	}
	case COMPV_HOG_SET_INT_INTERPOLATION: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		const int interp = *reinterpret_cast<const int*>(valuePtr);
		COMPV_CHECK_EXP_RETURN(
			interp != COMPV_HOG_INTERPOLATION_NEAREST &&
			interp != COMPV_HOG_INTERPOLATION_BILINEAR,
			COMPV_ERROR_CODE_E_INVALID_PARAMETER,
			"interp must be equal to COMPV_HOG_INTERPOLATION_xxxx"
		);
		m_nInterp = interp;
		return COMPV_ERROR_CODE_S_OK;
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

// IMPORTANT: this function *must be* thread-safe (do not modify members) as the HOG descriptor
// instance will be shared (see ultimateText and ultimateADAS projects)
COMPV_ERROR_CODE CompVHogStd::process(const CompVMatPtr& input, CompVMatPtrPtr output) /*Overrides(CompVHOG)*/
{
	COMPV_CHECK_EXP_RETURN(!input || !output, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_EXP_RETURN(input->elmtInBytes() != sizeof(uint8_t) || input->planeCount() != 1, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "input must be 8U_1D (e.g. grayscale image)");

	// TODO(dmi): for now we only support winSize = full image. In ultimateADAS and ultimateText the input already contains the full image to identify (thanks to connected components)
	// To add support for slidding window just split the code: "pre-process" to compute mapHist and "post-process" to compute the features.
	//		make sure not to compute the histogram map (requires magnitude and direction) several times when the windows are overlapping.
	const CompVSizeSz szWinSize = CompVSizeSz(input->cols(), input->rows());
	COMPV_CHECK_EXP_RETURN(szWinSize.width < m_szBlockSize.width || szWinSize.height < m_szBlockSize.height, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "winSize < blockSize");
	if (!input->isMemoryOwed()) { // Probably called bound() to extract a window for slidding
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("For slidding window you should change this function to compute magnitude and direction one time");
	}

	// Compute bin width associated values
	const int nBinWidth = static_cast<int>((m_bGradientSigned ? 360 : 180) / m_nbins);
	const compv_hog_floattype_t fBinWidth = static_cast<compv_hog_floattype_t>(nBinWidth);
	const compv_hog_floattype_t fBinWidthScale = 1 / fBinWidth;

	const compv_hog_floattype_t thetaMax = static_cast<compv_hog_floattype_t>(m_bGradientSigned ? 360 : 180);

	COMPV_ASSERT(!(m_szCellSize.width % m_szBlockStride.width) && !(m_szCellSize.height % m_szBlockStride.height));
	const CompVSizeFloat32 szStrideInCellsCount = { (m_szBlockStride.width / float(m_szCellSize.width)), (m_szBlockStride.height / float(m_szCellSize.height)) };
	COMPV_ASSERT(szStrideInCellsCount.width <= 1 && szStrideInCellsCount.height <= 1);

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
	int numCellsX = static_cast<int>(szWinSize.width / m_szCellSize.width / szStrideInCellsCount.width);
	int numCellsY = static_cast<int>(szWinSize.height / m_szCellSize.height / szStrideInCellsCount.height);
	CompVMatPtr mapHist;
	const size_t nMapWidth = numCellsX * m_nbins;
	const size_t nMapHeight = numCellsY;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_hog_floattype_t>(&mapHist, nMapHeight, nMapWidth)); // numCellsY * numBinsX
	const size_t nMapHistStride = mapHist->stride();
	// Multithreading - Process
	auto funcPtrOrientationBinning = [&](const size_t cellYstart, const size_t cellYend) -> COMPV_ERROR_CODE {
		const size_t yOffset = static_cast<size_t>(m_szCellSize.height * szStrideInCellsCount.height);
		const size_t yGuard = (((cellYend - 1) * yOffset) + m_szCellSize.height) > input->rows(); // Last(y) goes beyond the end?
		const compv_hog_floattype_t Yindex = compv_hog_floattype_t(cellYstart * yOffset);
		const compv_hog_floattype_t* magPtr = magnitude->ptr<const compv_hog_floattype_t>(static_cast<size_t>(Yindex));
		const compv_hog_floattype_t* dirPtr = direction->ptr<const compv_hog_floattype_t>(static_cast<size_t>(Yindex));
		compv_hog_floattype_t* mapHistPtr = mapHist->ptr<compv_hog_floattype_t>(cellYstart);
		const size_t xOffset = static_cast<size_t>(m_szCellSize.width * szStrideInCellsCount.width);
		const size_t xGuard = (((numCellsX - 1) * xOffset) + m_szCellSize.width) > input->cols(); // Last(x) goes beyond the end?
		const size_t nMagnitudeOffset = static_cast<size_t>(nMagnitudeStride * yOffset);
		const size_t nDirectionOffset = static_cast<size_t>(nDirectionStride * yOffset);
		int i;
		size_t x;
		for (size_t j = cellYstart; j < cellYend - yGuard; ++j) {
			for (i = 0, x = 0; i < numCellsX - xGuard; ++i, x += xOffset) {
				COMPV_CHECK_CODE_RETURN(
					CompVHogStd::buildMapHistForSingleCell(&mapHistPtr[i * m_nbins], &magPtr[x], &dirPtr[x],
						m_szCellSize.width, m_szCellSize.height, nMagnitudeStride, nDirectionStride,
						thetaMax, fBinWidth, fBinWidthScale, m_nbins, m_nInterp
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

	// Create output
	CompVMatPtr output_ = *output;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_hog_floattype_t>(&output_, 1, nOutSize));

	// Set norm function
	void(*fptr_norm)(compv_hog_floattype_t* inOutPtr, const compv_hog_floattype_t* eps1, const compv_uscalar_t count) = nullptr;
	const compv_hog_floattype_t eps = (m_nBlockNorm == COMPV_HOG_BLOCK_NORM_L2 || m_nBlockNorm == COMPV_HOG_BLOCK_NORM_L2HYS)
		? COMPV_HOG_EPSILON2 : COMPV_HOG_EPSILON;
	switch (m_nBlockNorm) {
	case COMPV_HOG_BLOCK_NORM_L1: fptr_norm = (nBlockBinsCount == 9) ? fptrs_norm.L1_9 : fptrs_norm.L1; break;
	case COMPV_HOG_BLOCK_NORM_L1SQRT: fptr_norm = (nBlockBinsCount == 9) ? fptrs_norm.L1Sqrt_9 : fptrs_norm.L1Sqrt; break;
	case COMPV_HOG_BLOCK_NORM_L2: fptr_norm = (nBlockBinsCount == 9) ? fptrs_norm.L2_9 : fptrs_norm.L2; break;
	case COMPV_HOG_BLOCK_NORM_L2HYS: fptr_norm = (nBlockBinsCount == 9) ? fptrs_norm.L2Hys_9 : fptrs_norm.L2Hys; break;
	case COMPV_HOG_BLOCK_NORM_NONE: break;
	default: COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED, "Supported norm functions: none,L1"); break;
	}
	
	// Multithreading - Process
	auto funcPtrBlockDescriptorAndNorm = [&](const size_t blockYstart, const size_t blockYend) -> COMPV_ERROR_CODE {
		compv_hog_floattype_t* outputPtr = output_->ptr<compv_hog_floattype_t>(0, (blockYstart * (numBlocksX * nBlockBinsCount)));
		const size_t Yindex = blockYstart * COMPV_MATH_ROUNDFU_2_NEAREST_INT(szStrideInCellsCount.height, size_t);
		const compv_hog_floattype_t* mapHistPtr = mapHist->ptr<const compv_hog_floattype_t>(Yindex);
		const size_t xBinOffset = m_nbins * COMPV_MATH_ROUNDFU_2_NEAREST_INT(szStrideInCellsCount.width, size_t);
		COMPV_ASSERT(xBinOffset && (numBlocksX * xBinOffset) <= mapHist->cols());
		COMPV_ASSERT((numBlocksY * (numBlocksX * nBlockBinsCount)) <= output_->cols());
		for (size_t blockY = blockYstart; blockY < blockYend; ++blockY) {
			for (size_t blockX = 0, binX = 0; blockX <= nMaxBlockX; blockX += m_szBlockStride.width, binX += xBinOffset) {
				// Build vector for a block at position (blocX, blockY)
				COMPV_CHECK_CODE_RETURN(CompVHogStd::buildOutputForSingleBlock(
					&mapHistPtr[binX], outputPtr,
					numCellsPerBlockY, numBinsPerBlockX, nMapHistStride
				));
				// Normalize the output vector
				if (fptr_norm) {
					fptr_norm(outputPtr, &eps, nBlockBinsCount);
				}
				outputPtr += nBlockBinsCount;
			}
			mapHistPtr += nMapHistStride;
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
	const compv_hog_floattype_t& thetaMax, const compv_hog_floattype_t& binWidth, const compv_hog_floattype_t& scaleBinWidth, const size_t& binCount, const int& interp
)
{
	// Private function, do not check input params

	// Reset histogram for the current cell
	for (size_t i = 0; i < binCount; ++i) {
		mapHistPtr[i] = 0;
	}

	if (interp == COMPV_HOG_INTERPOLATION_BILINEAR) {
#		if ((defined(_DEBUG) && _DEBUG != 0) || (defined(DEBUG) && DEBUG != 0)) || 1
		const int binIdxMax = static_cast<int>(binCount - 1);
#		endif
		for (size_t j = 0; j < cellHeight; ++j) {
			for (size_t i = 0; i < cellWidth; ++i) {
				const compv_hog_floattype_t theta = (dirPtr[i] > thetaMax) ? (dirPtr[i] - thetaMax) : dirPtr[i];
#				if ((defined(_DEBUG) && _DEBUG != 0) || (defined(DEBUG) && DEBUG != 0))
				COMPV_ASSERT(theta >= 0 && theta <= thetaMax);
#				endif
				// Bilinear interpolation: https://www2.cs.duke.edu/courses/fall15/compsci527/notes/hog.pdf, Cell Orientation Histograms
				// Also see: https://www.learnopencv.com/histogram-of-oriented-gradients/, Step 3 : Calculate Histogram of Gradients in 8×8 cells
				// Signed grad with 9 bins (width = 40) :  
				//	- legs:	[20, 60, 100, 140, 180, 220, 260, 300, 340]
				//	- leg 20:  [0   -  40] , note: 0 is same as 360 (wraps around)
				//	- leg 60:  [40  -  80]
				//	- leg 100: [80  - 120]
				//	- leg 140: [120 - 160]
				//	- leg 180: [160 - 200]
				//	- leg 220: [200 - 240]
				//	- leg 260: [240 - 280]
				//	- leg 300: [280 - 320]
				//	- leg 340: [320 - 360] , note: 360 is the same as 0 (wraps around)
				//	with 20 degrees on the right and left of each leg				
				const int binIdx = static_cast<int>((theta * scaleBinWidth) - 0.5f);
#				if ((defined(_DEBUG) && _DEBUG != 0) || (defined(DEBUG) && DEBUG != 0))
				// binIdx = COMPV_MATH_CLIP3(0, binIdxMax, binIdx);
				COMPV_ASSERT(binIdx >= 0 && binIdx <= binIdxMax);
#				endif
				const compv_hog_floattype_t diff = ((theta - (binIdx * binWidth)) * scaleBinWidth) - 0.5f;
#				if 0 // This code is branchless and could be used for SIMD accel
				COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Next (see #else) code faster");
				const compv_hog_floattype_t avv = std::abs(magPtr[i] * diff);
				const int binIdxNext = binIdx + ((diff >= 0) ? 1 : -1);
				mapHistPtr[binIdxNext < 0 ? binIdxMax : (binIdxNext > binIdxMax ? 0 : binIdxNext)] += avv;
				mapHistPtr[binIdx] += (magPtr[i] - avv);
#				else
				const compv_hog_floattype_t vv = magPtr[i] * diff;
				if (diff >= 0) {
					mapHistPtr[binIdx == binIdxMax ? 0 : (binIdx + 1)] += vv;
					mapHistPtr[binIdx] += (magPtr[i] - vv);
				}
				else {
					mapHistPtr[binIdx ? (binIdx - 1) : binIdxMax] -= vv;
					mapHistPtr[binIdx] += (magPtr[i] + vv);
				}
#				endif
			}
			magPtr += magStride;
			dirPtr += dirStride;
		}
	}
	else if (interp == COMPV_HOG_INTERPOLATION_NEAREST) {
		for (size_t j = 0; j < cellHeight; ++j) {
			for (size_t i = 0; i < cellWidth; ++i) {
				const compv_hog_floattype_t theta = (dirPtr[i] > thetaMax) ? (dirPtr[i] - thetaMax) : dirPtr[i];
				const int binIdx = static_cast<int>(theta * scaleBinWidth);
				mapHistPtr[binIdx] += magPtr[i];
			}
			magPtr += magStride;
			dirPtr += dirStride;
		}
	}
	else {
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED, "Invalid interpolation type");
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

COMPV_ERROR_CODE CompVHogStd::newObj(
	CompVHOGPtrPtr hog,
	const CompVSizeSz& blockSize COMPV_DEFAULT(CompVSizeSz(16, 16)),
	const CompVSizeSz& blockStride COMPV_DEFAULT(CompVSizeSz(8, 8)),
	const CompVSizeSz& cellSize COMPV_DEFAULT(CompVSizeSz(8, 8)),
	const size_t nbins COMPV_DEFAULT(9),
	const int blockNorm COMPV_DEFAULT(COMPV_HOG_BLOCK_NORM_L2HYS),
	const bool gradientSigned COMPV_DEFAULT(true),
	const int interp COMPV_DEFAULT(COMPV_HOG_INTERPOLATION_BILINEAR))
{
	COMPV_CHECK_EXP_RETURN(!hog, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_CODE_RETURN(CompVHOG::checkParams(blockSize, blockStride, cellSize, nbins, blockNorm, gradientSigned));
	CompVHogStdPtr hog_ = new CompVHogStd(blockSize, blockStride, cellSize, nbins, blockNorm, gradientSigned, interp);
	COMPV_CHECK_EXP_RETURN(!hog_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

#if COMPV_ARCH_X86
	if (CompVCpu::isEnabled(compv::kCpuFlagSSE2)) {
		/* == L1 == */
		COMPV_EXEC_IFDEF_INTRIN_X86(hog_->fptrs_norm.L1 = CompVHogCommonNormL1_32f_Intrin_SSE2);
#		if COMPV_HOG_FAST_BLOCK_9
		COMPV_EXEC_IFDEF_INTRIN_X86(hog_->fptrs_norm.L1_9 = CompVHogCommonNormL1_9_32f_Intrin_SSE2);
		COMPV_EXEC_IFDEF_ASM_X64(hog_->fptrs_norm.L1_9 = CompVHogCommonNormL1_9_32f_Asm_X64_SSE2);
#		else
		COMPV_EXEC_IFDEF_INTRIN_X86(hog_->fptrs_norm.L1_9 = CompVHogCommonNormL1_32f_Intrin_SSE2);
#		endif		

		/* == L1Sqrt == */
		COMPV_EXEC_IFDEF_INTRIN_X86(hog_->fptrs_norm.L1Sqrt = CompVHogCommonNormL1Sqrt_32f_Intrin_SSE2);
#		if COMPV_HOG_FAST_BLOCK_9
		COMPV_EXEC_IFDEF_INTRIN_X86(hog_->fptrs_norm.L1Sqrt_9 = CompVHogCommonNormL1Sqrt_9_32f_Intrin_SSE2);
		COMPV_EXEC_IFDEF_ASM_X64(hog_->fptrs_norm.L1Sqrt_9 = CompVHogCommonNormL1Sqrt_9_32f_Asm_X64_SSE2);
#		else
		COMPV_EXEC_IFDEF_INTRIN_X86(hog_->fptrs_norm.L1Sqrt_9 = CompVHogCommonNormL1Sqrt_32f_Intrin_SSE2);
#		endif

		/* == L2 == */
		COMPV_EXEC_IFDEF_INTRIN_X86(hog_->fptrs_norm.L2 = CompVHogCommonNormL2_32f_Intrin_SSE2);
#		if COMPV_HOG_FAST_BLOCK_9
		COMPV_EXEC_IFDEF_INTRIN_X86(hog_->fptrs_norm.L2_9 = CompVHogCommonNormL2_9_32f_Intrin_SSE2);
		COMPV_EXEC_IFDEF_ASM_X64(hog_->fptrs_norm.L2_9 = CompVHogCommonNormL2_9_32f_Asm_X64_SSE2);
#		else
		COMPV_EXEC_IFDEF_INTRIN_X86(hog_->fptrs_norm.L2_9 = CompVHogCommonNormL2_32f_Intrin_SSE2);
#		endif

		/* == L2Hys == */
		COMPV_EXEC_IFDEF_INTRIN_X86(hog_->fptrs_norm.L2Hys = CompVHogCommonNormL2Hys_32f_Intrin_SSE2);
#		if COMPV_HOG_FAST_BLOCK_9
		COMPV_EXEC_IFDEF_INTRIN_X86(hog_->fptrs_norm.L2Hys_9 = CompVHogCommonNormL2Hys_9_32f_Intrin_SSE2);
		COMPV_EXEC_IFDEF_ASM_X64(hog_->fptrs_norm.L2Hys_9 = CompVHogCommonNormL2Hys_9_32f_Asm_X64_SSE2);
#		else
		COMPV_EXEC_IFDEF_INTRIN_X86(hog_->fptrs_norm.L2Hys_9 = CompVHogCommonNormL2Hys_32f_Intrin_SSE2);
#		endif
	}
#elif COMPV_ARCH_ARM
#endif

	*hog = *hog_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
