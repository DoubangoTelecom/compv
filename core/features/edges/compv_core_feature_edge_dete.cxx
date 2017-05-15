/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/features/edges/compv_core_feature_edge_dete.h"
#include "compv/base/math/compv_math_utils.h"
#include "compv/base/math/compv_math_convlt.h"
#include "compv/base/math/compv_math_distance.h"
#include "compv/base/image/compv_image.h"
#include "compv/base/parallel/compv_parallel.h"

// Sobel implementation not optiz: https://github.com/DoubangoTelecom/compv/issues/127

#define COMPV_FEATURE_DETE_EDGES_GRAD_MIN_SAMPLES_PER_THREAD	3 // must be >= kernelSize because of the convolution ("rowsOverlapCount")

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

COMPV_ERROR_CODE CompVCornerDeteEdgeBase::set(int id, const void* valuePtr, size_t valueSize) /*Overrides(CompVCaps)*/
{
	COMPV_CHECK_EXP_RETURN(!valuePtr || !valueSize, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	switch (id) {
	case 0:
	default:
		COMPV_CHECK_CODE_RETURN(CompVCaps::set(id, valuePtr, valueSize));
		return COMPV_ERROR_CODE_S_OK;
	}
}

COMPV_ERROR_CODE CompVCornerDeteEdgeBase::process(const CompVMatPtr& image, CompVMatPtrPtr edges, CompVMatPtrPtr directions COMPV_DEFAULT(NULL)) /*Overrides(CompVEdgeDete)*/
{
	COMPV_CHECK_EXP_RETURN(!image || image->subType() != COMPV_SUBTYPE_PIXELS_Y || !edges, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Input image is null or not in grayscale format");

	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	CompVMatPtr gradDir = NULL;

	// Realloc pGx and pGy if image size changes
	if (!m_pGx || !m_pGy || !m_pG || image->cols() != m_nImageWidth || image->rows() != m_nImageHeight || image->stride() != m_nImageStride) {
		CompVMem::free(reinterpret_cast<void**>(&m_pGx));
		CompVMem::free(reinterpret_cast<void**>(&m_pGy));
		CompVMem::free(reinterpret_cast<void**>(&m_pG));
		m_nImageWidth = image->cols();
		m_nImageHeight = image->rows();
		m_nImageStride = image->stride();
		m_pGx = reinterpret_cast<int16_t*>(CompVMem::malloc(CompVMathConvlt::outputSizeInBytes<int16_t>(m_nImageStride, m_nImageHeight)));
		COMPV_CHECK_EXP_RETURN(!m_pGx, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
		m_pGy = reinterpret_cast<int16_t*>(CompVMem::malloc(CompVMathConvlt::outputSizeInBytes<int16_t>(m_nImageStride, m_nImageHeight)));
		COMPV_CHECK_EXP_RETURN(!m_pGy, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
		m_pG = reinterpret_cast<uint16_t*>(CompVMem::malloc(CompVMathConvlt::outputSizeInBytes<uint16_t>(m_nImageStride, m_nImageHeight)));
		COMPV_CHECK_EXP_RETURN(!m_pG, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	}

	// Create direction buffer
#if 0 // Result is correct but functions using it (e.g. hough-lines) not finished yet
	if (directions) {
		gradDir = *directions;
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float32_t>(&gradDir, m_nImageHeight, m_nImageWidth, m_nImageStride));
	}
#endif

	// Create edges buffer
	// edges must have same stride than m_pG (required by scaleAndClip)
	COMPV_CHECK_CODE_RETURN(CompVImage::newObj8u(edges, COMPV_SUBTYPE_PIXELS_Y, m_nImageWidth, m_nImageHeight, m_nImageStride));
	uint8_t* edgesPtr = (*edges)->ptr<uint8_t>();

	// Testing info: for "equirectangular_1282x720_gray" gmax should be equal to 1464
	uint16_t gmax = 1;

	// Get Max number of threads
	CompVThreadDispatcherPtr threadDisp = CompVParallel::threadDispatcher();
	const size_t maxThreads = (threadDisp && !threadDisp->isMotherOfTheCurrentThread()) ? static_cast<size_t>(threadDisp->threadsCount()) : 1;
	const size_t threadsCount = CompVThreadDispatcher::guessNumThreadsDividingAcrossY(m_nImageStride, m_nImageHeight, maxThreads, COMPV_MATH_MAX(m_nKernelSize, COMPV_FEATURE_DETE_EDGES_GRAD_MIN_SAMPLES_PER_THREAD));

	// Convolution + Gradient
	if (threadsCount > 1) {
		const size_t rowsOverlapCount = ((m_nKernelSize >> 1) << 1); // (kernelRadius times 2)
		const size_t rowsOverlapPad = rowsOverlapCount * m_nImageStride;
		const size_t countAny = (m_nImageHeight / threadsCount);
		const size_t countLast = countAny + (m_nImageHeight % threadsCount);
		const size_t countAnyTimesStride = countAny * m_nImageStride;
		const uint8_t* inPtr_ = image->ptr<const uint8_t>();
		compv_float32_t* gradDirPtr = gradDir ? gradDir->ptr<compv_float32_t>() : NULL;
		uint16_t* gmaxTmp = reinterpret_cast<uint16_t*>(CompVMem::malloc(threadsCount * sizeof(uint16_t)));
		int16_t* outPtrGx_ = m_pGx;
		int16_t* outPtrGy_ = m_pGy;
		uint16_t* outPtrG_ = m_pG;
		CompVAsyncTaskIds taskIds;
		compv_float32_t scale;
		//!\\ Important: Our tests showed (both x86 and arm)d that it's faster to alloc temp memory for each thread rather than sharing global one -> false sharing issue.
		// This is an issue for the convolution only because there is no way to make the writing cache-friendly.
		// No such issue when multithreading 'CompVMathConvlt::convlt1' (perf tests done), so don't try to change the function.
		// https://en.wikipedia.org/wiki/False_sharing
		auto funcConvolutionAndGradient = [&](const uint8_t* ptrIn, int16_t* ptrOutGx, int16_t* ptrOutGy, uint16_t* ptrOutG, compv_float32_t* ptrGradDir, size_t h, size_t threadIdx) -> COMPV_ERROR_CODE {
			// Convolution
			int16_t* imgTmp = reinterpret_cast<int16_t*>(CompVMem::malloc(CompVMathConvlt::outputSizeInBytes<int16_t>(m_nImageStride, h + rowsOverlapCount))); // local alloc to avoid false sharing
			COMPV_CHECK_EXP_RETURN(!imgTmp, (err = COMPV_ERROR_CODE_E_OUT_OF_MEMORY), "Failed to alloc imgTmp");
			const bool first = (threadIdx == 0);
			const bool last = (threadIdx == (threadsCount - 1));
			const size_t padding = first ? 0 : rowsOverlapPad;
			CompVMathConvlt::convlt1Hz<uint8_t, int16_t, int16_t>(ptrIn - padding, imgTmp, m_nImageWidth, h + rowsOverlapCount, m_nImageStride, m_pcKernelHz, m_nKernelSize, true);
			CompVMathConvlt::convlt1Vt<int16_t, int16_t, int16_t>(imgTmp, ptrOutGx - padding, m_nImageWidth, h + rowsOverlapCount, m_nImageStride, m_pcKernelVt, m_nKernelSize, first, last);
			CompVMathConvlt::convlt1Hz<uint8_t, int16_t, int16_t>(ptrIn - padding, imgTmp, m_nImageWidth, h + rowsOverlapCount, m_nImageStride, m_pcKernelVt, m_nKernelSize, true);
			CompVMathConvlt::convlt1Vt<int16_t, int16_t, int16_t>(imgTmp, ptrOutGy - padding, m_nImageWidth, h + rowsOverlapCount, m_nImageStride, m_pcKernelHz, m_nKernelSize, first, last);
			CompVMem::free((void**)&imgTmp);
			// Gradient using L1 distance (abs(gx) + abs(gy))
			COMPV_CHECK_CODE_RETURN(err = (CompVMathUtils::sumAbs<int16_t, uint16_t>(ptrOutGx - padding, ptrOutGy - padding, ptrOutG - padding, m_nImageWidth, h + rowsOverlapCount, m_nImageStride)));
			// Gradient directions
			if (ptrGradDir) {
				COMPV_CHECK_CODE_RETURN((CompVMathUtils::atan2<int16_t, compv_float32_t>(ptrOutGx - padding, ptrOutGy - padding, ptrGradDir - padding, m_nImageWidth, h + rowsOverlapCount, m_nImageStride)));
			}
			return COMPV_ERROR_CODE_S_OK;
		};
		auto funcGmax = [&](const uint16_t* ptrG, size_t h, uint16_t *max) -> COMPV_ERROR_CODE {
			uint16_t max_ = 1;
			COMPV_CHECK_CODE_RETURN(err = (CompVMathUtils::max<uint16_t>(ptrG, m_nImageWidth, h, m_nImageStride, max_)));
			*max = max_;
			return COMPV_ERROR_CODE_S_OK;
		};
		auto funcScaleAndClipPixel8 = [&](const uint16_t* ptrG, uint8_t* ptrEdges, compv_float32_t scale, size_t h) -> COMPV_ERROR_CODE {
			COMPV_CHECK_CODE_RETURN(err = (CompVMathUtils::scaleAndClipPixel8<uint16_t, compv_float32_t>(ptrG, scale, ptrEdges, m_nImageWidth, h, m_nImageStride)));
			return COMPV_ERROR_CODE_S_OK;
		};

		COMPV_CHECK_EXP_BAIL(!gmaxTmp, (err = COMPV_ERROR_CODE_E_OUT_OF_MEMORY), "Failed to alloc gmaxTmp memory");
		taskIds.reserve(threadsCount);

		// convolution + gradient
		for (size_t threadIdx = 0, index = 0; threadIdx < threadsCount; ++threadIdx, index += countAnyTimesStride) {
			COMPV_CHECK_CODE_BAIL(err = threadDisp->invoke(std::bind(funcConvolutionAndGradient, &inPtr_[index], &outPtrGx_[index], &outPtrGy_[index], &outPtrG_[index], (gradDirPtr ? &gradDirPtr[index] : NULL),
				(threadIdx == (threadsCount - 1)) ? countLast : countAny, threadIdx),
				taskIds));
		}
		COMPV_CHECK_CODE_BAIL(err = threadDisp->wait(taskIds));

		// Gmax (cannot be packed in convolution+gradient threads)
		taskIds.clear();
		for (size_t threadIdx = 0, index = 0; threadIdx < threadsCount; ++threadIdx, index += countAnyTimesStride) {
			COMPV_CHECK_CODE_BAIL(err = threadDisp->invoke(std::bind(funcGmax, &m_pG[index], (threadIdx == (threadsCount - 1)) ? countAny : countLast, &gmaxTmp[threadIdx]), taskIds));
		}
		for (size_t threadIdx = 0; threadIdx < threadsCount; ++threadIdx) {
			COMPV_CHECK_CODE_BAIL(err = threadDisp->waitOne(taskIds[threadIdx]));
			gmax = COMPV_MATH_MAX(gmax, gmaxTmp[threadIdx]);
		}

		// scale (normalization)
		scale = 255.f / compv_float32_t(gmax);
		taskIds.clear();
		for (size_t threadIdx = 0, index = 0; threadIdx < threadsCount; ++threadIdx, index += countAnyTimesStride) {
			COMPV_CHECK_CODE_BAIL(err = threadDisp->invoke(std::bind(funcScaleAndClipPixel8, &m_pG[index], &edgesPtr[index], scale, (threadIdx == (threadsCount - 1)) ? countAny : countLast), taskIds));
		}
		// wait for the execution after memory free
	bail:
		CompVMem::free(reinterpret_cast<void**>(&gmaxTmp));

		COMPV_CHECK_CODE_NOP(threadDisp->wait(taskIds));

		COMPV_CHECK_CODE_RETURN(err);
	}
	else {
		// Convolution
		COMPV_CHECK_CODE_RETURN((CompVMathConvlt::convlt1<uint8_t, int16_t, int16_t>(image->ptr<const uint8_t>(), m_nImageWidth, m_nImageHeight, m_nImageStride, m_pcKernelVt, m_pcKernelHz, m_nKernelSize, m_pGx)));
		COMPV_CHECK_CODE_RETURN((CompVMathConvlt::convlt1<uint8_t, int16_t, int16_t>(image->ptr<const uint8_t>(), m_nImageWidth, m_nImageHeight, m_nImageStride, m_pcKernelHz, m_pcKernelVt, m_nKernelSize, m_pGy)));
		// Gradient mag (abs(gx) + abs(gy))
		COMPV_CHECK_CODE_RETURN((CompVMathUtils::sumAbs<int16_t, uint16_t>(m_pGx, m_pGy, m_pG, m_nImageWidth, m_nImageHeight, m_nImageStride)));
		// Gradient directions
		if (gradDir) {
			COMPV_CHECK_CODE_RETURN((CompVMathUtils::atan2<int16_t, compv_float32_t>(m_pGx, m_pGy, gradDir->ptr<compv_float32_t>(), m_nImageWidth, m_nImageHeight, m_nImageStride)));
		}
		// gmax
		COMPV_CHECK_CODE_RETURN((CompVMathUtils::max<uint16_t>(m_pG, m_nImageWidth, m_nImageHeight, m_nImageStride, gmax)));
		// scale (normalization)
		compv_float32_t scale = 255.f / compv_float32_t(gmax);
		COMPV_CHECK_CODE_RETURN((CompVMathUtils::scaleAndClipPixel8<uint16_t, compv_float32_t>(m_pG, scale, edgesPtr, m_nImageWidth, m_nImageHeight, m_nImageStride)));
	}

	return err;
}

COMPV_ERROR_CODE CompVCornerDeteEdgeBase::newObjSobel(CompVEdgeDetePtrPtr dete, float tLow COMPV_DEFAULT(0.68f), float tHigh COMPV_DEFAULT(0.68f*2.f), size_t kernSize COMPV_DEFAULT(3))
{
#if defined(_DEBUG)
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Sobel implementation not fully optiz: https://github.com/DoubangoTelecom/compv/issues/127");
#endif
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
