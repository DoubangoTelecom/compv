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

#define COMPV_FEATURE_DETE_EDGES_GRAD_MIN_SAMPLES_PER_THREAD	3 // must be >= 3 because of the convolution ("rowsOverlapCount")

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

	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;

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

	uint16_t gmax = 1;
	
	// Get Max number of threads
	CompVThreadDispatcherPtr threadDisp = CompVParallel::threadDispatcher();
	const size_t maxThreads = (threadDisp && !threadDisp->isMotherOfTheCurrentThread()) ? static_cast<size_t>(threadDisp->threadsCount()) : 1;
	const size_t threadsCount = COMPV_MATH_MIN(maxThreads, (m_nImageHeight / COMPV_FEATURE_DETE_EDGES_GRAD_MIN_SAMPLES_PER_THREAD));

	// Convolution + Gradient
	if (threadsCount > 1) {
		const size_t rowsOverlapCount = ((m_nKernelSize >> 1) << 1); // (kernelRadius times 2)
		const size_t rowsOverlapPad = rowsOverlapCount * m_nImageStride;
		const size_t countAny = (m_nImageHeight / threadsCount);
		const size_t countLast = countAny + (m_nImageHeight % threadsCount);
		const size_t countAnyTimesStride = countAny * m_nImageStride;
		const uint8_t* inPtr_ = image->ptr<const uint8_t>();
		int16_t* imgTmpGx = reinterpret_cast<int16_t*>(CompVMem::malloc(CompVMathConvlt::outputSizeInBytes<int16_t>(m_nImageStride, m_nImageHeight)));
		int16_t* imgTmpGy = reinterpret_cast<int16_t*>(CompVMem::malloc(CompVMathConvlt::outputSizeInBytes<int16_t>(m_nImageStride, m_nImageHeight)));
		uint16_t* gmaxTmp = reinterpret_cast<uint16_t*>(CompVMem::malloc(threadsCount * sizeof(uint16_t)));
		int16_t* tmpPtrGx_ = imgTmpGx;
		int16_t* tmpPtrGy_ = imgTmpGy;
		int16_t* outPtrGx_ = m_pGx;
		int16_t* outPtrGy_ = m_pGy;
		uint16_t* outPtrG_ = m_pG;
		CompVAsyncTaskIds taskIds;
		auto funcPtrFirst = [&](const uint8_t* ptrIn, int16_t* ptrOutGx, int16_t* ptrTmpGx, int16_t* ptrOutGy, int16_t* ptrTmpGy, uint16_t* ptrOutG, size_t h) -> COMPV_ERROR_CODE {
			// Convolution
			CompVMathConvlt::convlt1Hz<uint8_t, int16_t, int16_t>(ptrIn, ptrTmpGx, m_nImageWidth, h + rowsOverlapCount, m_nImageStride, m_pcKernelHz, m_nKernelSize);
			CompVMathConvlt::convlt1Hz<uint8_t, int16_t, int16_t>(ptrIn, ptrTmpGy, m_nImageWidth, h + rowsOverlapCount, m_nImageStride, m_pcKernelVt, m_nKernelSize);
			CompVMathConvlt::convlt1Vt<int16_t, int16_t, int16_t>(ptrTmpGx, ptrOutGx, m_nImageWidth, h + rowsOverlapCount, m_nImageStride, m_pcKernelVt, m_nKernelSize, true, false);
			CompVMathConvlt::convlt1Vt<int16_t, int16_t, int16_t>(ptrTmpGy, ptrOutGy, m_nImageWidth, h + rowsOverlapCount, m_nImageStride, m_pcKernelHz, m_nKernelSize, true, false);
			// Gradient without computing gmax(computing gmax not thread - safe because of overlappings)
			COMPV_CHECK_CODE_RETURN(err = (CompVMathUtils::gradientL1<int16_t, uint16_t>(ptrOutGx, ptrOutGy, ptrOutG, m_nImageWidth, h + rowsOverlapCount, m_nImageStride)));
			return COMPV_ERROR_CODE_S_OK;
		};
		auto funcPtrOthers = [&](const uint8_t* ptrIn, int16_t* ptrOutGx, int16_t* ptrTmpGx, int16_t* ptrOutGy, int16_t* ptrTmpGy, uint16_t* ptrOutG, size_t h, bool last) -> COMPV_ERROR_CODE {
			// Convolution
			CompVMathConvlt::convlt1Hz<uint8_t, int16_t, int16_t>(ptrIn - rowsOverlapPad, ptrTmpGx - rowsOverlapPad, m_nImageWidth, h + rowsOverlapCount, m_nImageStride, m_pcKernelHz, m_nKernelSize);
			CompVMathConvlt::convlt1Hz<uint8_t, int16_t, int16_t>(ptrIn - rowsOverlapPad, ptrTmpGy - rowsOverlapPad, m_nImageWidth, h + rowsOverlapCount, m_nImageStride, m_pcKernelVt, m_nKernelSize);
			CompVMathConvlt::convlt1Vt<int16_t, int16_t, int16_t>(ptrTmpGx - rowsOverlapPad, ptrOutGx - rowsOverlapPad, m_nImageWidth, h + rowsOverlapCount, m_nImageStride, m_pcKernelVt, m_nKernelSize, false, last);
			uint16_t* mt_g = ptrOutG - rowsOverlapPad;
			CompVMathConvlt::convlt1Vt<int16_t, int16_t, int16_t>(ptrTmpGy - rowsOverlapPad, ptrOutGy - rowsOverlapPad, m_nImageWidth, h + rowsOverlapCount, m_nImageStride, m_pcKernelHz, m_nKernelSize, false, last);
			// Gradient without computing gmax (computing gmax not thread-safe because of overlappings)
			COMPV_CHECK_CODE_RETURN(err = (CompVMathUtils::gradientL1<int16_t, uint16_t>(ptrOutGx - rowsOverlapPad, ptrOutGy - rowsOverlapPad, mt_g, m_nImageWidth, h + rowsOverlapCount, m_nImageStride)));
			return COMPV_ERROR_CODE_S_OK;
		};
		auto funcGmax = [&](const uint16_t* ptrG, size_t h, uint16_t *max) -> COMPV_ERROR_CODE {
			uint16_t max_ = 1;
			COMPV_CHECK_CODE_RETURN(err = (CompVMathUtils::max<uint16_t>(ptrG, m_nImageWidth, h, m_nImageStride, max_)));
			*max = max_;
			return COMPV_ERROR_CODE_S_OK;
		};

		COMPV_CHECK_EXP_BAIL(!imgTmpGx, (err = COMPV_ERROR_CODE_E_OUT_OF_MEMORY), "Failed to alloc imgTmpGx memory");
		COMPV_CHECK_EXP_BAIL(!imgTmpGy, (err = COMPV_ERROR_CODE_E_OUT_OF_MEMORY), "Failed to alloc imgTmpGy memory");
		taskIds.reserve(threadsCount);

		// first
		COMPV_CHECK_CODE_BAIL(err = threadDisp->invoke(std::bind(funcPtrFirst, inPtr_, outPtrGx_, tmpPtrGx_, outPtrGy_, tmpPtrGy_, outPtrG_, countAny), taskIds));
		inPtr_ += countAny * m_nImageStride;
		tmpPtrGx_ += countAnyTimesStride;
		outPtrGx_ += countAnyTimesStride;
		tmpPtrGy_ += countAnyTimesStride;
		outPtrGy_ += countAnyTimesStride;
		outPtrG_ += countAnyTimesStride;
		// others
		for (size_t threadIdx = 1; threadIdx < threadsCount - 1; ++threadIdx) {
			COMPV_CHECK_CODE_BAIL(err = threadDisp->invoke(std::bind(funcPtrOthers, inPtr_, outPtrGx_, tmpPtrGx_, outPtrGy_, tmpPtrGy_, outPtrG_, countAny, false), taskIds));
			inPtr_ += countAnyTimesStride;
			tmpPtrGx_ += countAnyTimesStride;
			outPtrGx_ += countAnyTimesStride;
			tmpPtrGy_ += countAnyTimesStride;
			outPtrGy_ += countAnyTimesStride;
			outPtrG_ += countAnyTimesStride;
		}
		// last
		COMPV_CHECK_CODE_BAIL(err = threadDisp->invoke(std::bind(funcPtrOthers, inPtr_, outPtrGx_, tmpPtrGx_, outPtrGy_, tmpPtrGy_, outPtrG_, countLast, true), taskIds));
		
		// wait
		COMPV_CHECK_CODE_BAIL(err = threadDisp->wait(taskIds));

		// Gmax (cannot be packed in convolution+gradient threads)
		for (size_t threadIdx = 0; threadIdx < threadsCount; ++threadIdx) {
			COMPV_CHECK_CODE_BAIL(err = threadDisp->invoke(std::bind(funcGmax, &m_pG[countAnyTimesStride * threadIdx], (threadIdx == (threadsCount - 1)) ? countAny : countLast, &gmaxTmp[threadIdx]), taskIds));
		}
		for (size_t threadIdx = 0; threadIdx < threadsCount; ++threadIdx) {
			COMPV_CHECK_CODE_BAIL(err = threadDisp->waitOne(taskIds[threadIdx]));
			gmax = COMPV_MATH_MAX(gmax, gmaxTmp[threadIdx]);
		}
bail:
		CompVMem::free(reinterpret_cast<void**>(&imgTmpGx));
		CompVMem::free(reinterpret_cast<void**>(&imgTmpGy));
		CompVMem::free(reinterpret_cast<void**>(&gmaxTmp));
		COMPV_CHECK_CODE_RETURN(err);
	}
	else {
		// Convolution
		COMPV_CHECK_CODE_RETURN((CompVMathConvlt::convlt1<uint8_t, int16_t, int16_t>(image->ptr<const uint8_t>(), m_nImageWidth, m_nImageHeight, m_nImageStride, m_pcKernelVt, m_pcKernelHz, m_nKernelSize, m_pGx)));
		COMPV_CHECK_CODE_RETURN((CompVMathConvlt::convlt1<uint8_t, int16_t, int16_t>(image->ptr<const uint8_t>(), m_nImageWidth, m_nImageHeight, m_nImageStride, m_pcKernelHz, m_pcKernelVt, m_nKernelSize, m_pGy)));

		// Compute gradiant using L1 distance
		COMPV_CHECK_CODE_RETURN((CompVMathUtils::gradientL1<int16_t, uint16_t>(m_pGx, m_pGy, m_pG, m_nImageWidth, m_nImageHeight, m_nImageStride, &gmax)));
	}

	// Create edges buffer
	// edges must have same stride than m_pG (required by scaleAndClip)
	COMPV_CHECK_CODE_RETURN(CompVImage::newObj8u(edges, COMPV_SUBTYPE_PIXELS_Y, m_nImageWidth, m_nImageHeight, m_nImageStride));

	// Testing info: for "opengl_programming_guide_8th_edition_200x258_gray" gmax should be equal to 1464

	// scale (normalization)
	float scale = 255.f / float(gmax);
	uint8_t* edgesPtr = (*edges)->ptr<uint8_t>();
	COMPV_CHECK_CODE_RETURN((CompVMathUtils::scaleAndClip<uint16_t, float, uint8_t>(m_pG, scale, edgesPtr, 0, 255, m_nImageWidth, m_nImageHeight, m_nImageStride)));

	return err;
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
