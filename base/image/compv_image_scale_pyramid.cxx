/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/compv_image_scale_pyramid.h"
#include "compv/base/image/compv_image.h"
#include "compv/base/parallel/compv_parallel.h"
#include "compv/base/compv_base.h"

#define COMPV_THIS_CLASSNAME	"CompVImageScalePyramid"

COMPV_NAMESPACE_BEGIN()

CompVImageScalePyramid::CompVImageScalePyramid(float fScaleFactor COMPV_DEFAULT(0.83f), size_t nLevels COMPV_DEFAULT(8), COMPV_SCALE_TYPE eScaleType COMPV_DEFAULT(COMPV_SCALE_TYPE_BILINEAR))
	: m_bValid(false)
	, m_fScaleFactor(fScaleFactor)
	, m_fScaleFactorsSum(1.f)
	, m_nLevels(nLevels)
	, m_eScaleType(eScaleType)
	, m_pImages(NULL)
	, m_pScaleFactors(NULL)
{
	COMPV_ASSERT(m_nLevels > 0 && m_fScaleFactor > 0); // never happen, we already checked it in newObj()
	m_pImages = reinterpret_cast<CompVMatPtrPtr>(CompVMem::calloc(m_nLevels, sizeof(CompVMatPtr)));
	if (!m_pImages) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Failed to allocate memory (%zu)", m_nLevels * sizeof(CompVMatPtr));
		return;
	}
	m_pScaleFactors = reinterpret_cast<float*>(CompVMem::calloc(m_nLevels, sizeof(float)));
	if (!m_pScaleFactors) {
		m_pScaleFactors = reinterpret_cast<float*>(CompVMem::calloc(m_nLevels, sizeof(float)));
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Failed to allocate memory (%zu)", m_nLevels * sizeof(float));
		return;
	}
	// Compute sum of all scale factors. This is used to generate the contribution for each level
	// For example level3 will contribute: ((K * sf^3) / sfs)
	float sf_ = m_fScaleFactor;
	m_pScaleFactors[0] = 1.f;
	for (size_t level = 1; level < m_nLevels; ++level, sf_ *= m_fScaleFactor) {
		m_pScaleFactors[level] = sf_;
		m_fScaleFactorsSum += sf_;
	}
	m_bValid = true;
}

CompVImageScalePyramid::~CompVImageScalePyramid()
{
	if (m_pImages) {
		for (size_t level = 0; level < m_nLevels; ++level) {
			m_pImages[level] = NULL;
		}
		CompVMem::free(reinterpret_cast<void**>(&m_pImages));
	}
	if (m_pScaleFactors) {
		CompVMem::free(reinterpret_cast<void**>(&m_pScaleFactors));
	}
}

// Must be thread safe
COMPV_ERROR_CODE CompVImageScalePyramid::process(const CompVMatPtr& inImage, size_t level COMPV_DEFAULT(COMPV_PYRAMID_LEVEL_ALL))
{
	COMPV_CHECK_EXP_RETURN(!inImage || (level >= m_nLevels && (level != COMPV_PYRAMID_LEVEL_ALL)), COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	// TODO(dmi): We have two options here:
	//	1/ imageN=scale(imageN-1,sf)
	//	2/ imageN=scale(imageOrig,sf)
	// Option 2 produce better result in terms of quality and matching rate. Also option 2 can be multithreaded
	// which is not the case for option 1.
	// This funtion is sometimes multithreaded (e.g. when called from ORB dete), make sure we are using option 2

#if 1 // Option 2
	if (level  == COMPV_PYRAMID_LEVEL_ALL) {
		size_t threadsCount;
		CompVThreadDispatcherPtr threadDisp = CompVParallel::threadDispatcher();
		size_t maxThreads = threadDisp ? static_cast<size_t>(threadDisp->threadsCount()) : 0;

		// Compute number of threads
		threadsCount = (threadDisp && !threadDisp->isMotherOfTheCurrentThread()) ? maxThreads : 1;

#if 0 // scale-based MT is faster
		// The scaling function is already multithreaded which means we can use single thread for the pyramid.
		// ... but we can do better.
		// Each time we call the scaling function we will start/stop the master threads which is no optimal.
#if COMPV_ARCH_ARM // no hyperthreading (threads # cores)
		const bool usePyramidMTInsteadOfScaleMT = (threadsCount > 1) && (m_nLevels >= (threadsCount >> 1)); // if true means we have enough levels to feed the master threads
#else 
		const bool usePyramidMTInsteadOfScaleMT = (threadsCount > 1) && (m_nLevels >= ((2 * threadsCount) / 3)); // if true means we have enough levels to feed the master threads
#endif
#else
		const bool usePyramidMTInsteadOfScaleMT = false;
#endif
		if (usePyramidMTInsteadOfScaleMT) {
			// Using pyramid-MT
			CompVAsyncTaskIds taskIds;
			auto funcPtr = [&](size_t lev) -> void {
				COMPV_CHECK_CODE_NOP(processLevel(inImage, lev));
			};
			// levelStart is used to make sure we won't schedule more than "threadsCount"
			size_t levelStart, lev, levelMax;
			for (levelStart = 0, levelMax = threadsCount; levelStart < m_nLevels; levelStart += threadsCount, levelMax += threadsCount) {
				for (lev = levelStart; lev < m_nLevels && lev < levelMax; ++lev) {
					COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtr, lev), taskIds), "Dispatching task failed");
				}
				COMPV_CHECK_CODE_RETURN(threadDisp->wait(taskIds), "Failed to wait for tasks execution");
			}			
		}
		else {
			// Using scale-MT
			for (size_t lev = 0; lev < m_nLevels; ++lev) {
				COMPV_CHECK_CODE_RETURN(processLevel(inImage, lev));
			}
		}
	}
	else {
		// Using scale-MT
		COMPV_CHECK_CODE_RETURN(processLevel(inImage, level));
	}
#else
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Worst results and not MT"); // TODO(dmi): multi-thread
	int32_t outWidth, outHeight, inWidth, inHeight;
	float sf;

	inWidth = inImage->getWidth();
	inHeight = inImage->getHeight();
	if (level < 0) {
		COMPV_CHECK_CODE_RETURN(inImage->scale(m_eScaleType, inWidth, inHeight, &m_pImages[0])); // level-0
		for (int32_t lev = 1; lev < m_nLevels; ++lev) {
			sf = getScaleFactor(lev);
			outWidth = (int32_t)(inWidth * sf);
			outHeight = (int32_t)(inHeight * sf);
			COMPV_CHECK_CODE_RETURN(m_pImages[lev - 1]->scale(m_eScaleType, outWidth, outHeight, &m_pImages[lev]));
		}
	}
	else {
		sf = getScaleFactor(level);
		outWidth = (int32_t)(inWidth * sf);
		outHeight = (int32_t)(inHeight * sf);
		COMPV_CHECK_CODE_RETURN(m_pImages[level - 1]->scale(m_eScaleType, outWidth, outHeight, &m_pImages[level]));
	}
#endif
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImageScalePyramid::image(size_t level, CompVMatPtrPtr image)
{
	COMPV_CHECK_EXP_RETURN(!image || level >= m_nLevels, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	*image = m_pImages[level];
	return COMPV_ERROR_CODE_S_OK;
}

float CompVImageScalePyramid::scaleFactor(size_t level COMPV_DEFAULT(COMPV_PYRAMID_LEVEL_FIRST)/*for level 0 it's always equal to 1.f*/)
{
	if (level >= m_nLevels) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Invalid level=%zu", level);
		return 0.f;
	}
	return m_pScaleFactors[level];
}

// Private function
COMPV_ERROR_CODE CompVImageScalePyramid::processLevel(const CompVMatPtr& inImage, size_t level)
{
	const float sf = scaleFactor(level);
	COMPV_CHECK_CODE_RETURN(CompVImage::scale(inImage, &m_pImages[level], static_cast<size_t>(inImage->cols() * sf), static_cast<size_t>(inImage->rows() * sf), m_eScaleType));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImageScalePyramid::newObj(CompVImageScalePyramidPtrPtr pyramid, float fScaleFactor COMPV_DEFAULT(0.83f), size_t nLevels COMPV_DEFAULT(8), COMPV_SCALE_TYPE eScaleType COMPV_DEFAULT(COMPV_SCALE_TYPE_BILINEAR))
{
	COMPV_CHECK_CODE_RETURN(CompVBase::init());
	COMPV_CHECK_EXP_RETURN(pyramid == NULL || nLevels <= 0 || fScaleFactor <= 0, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVImageScalePyramidPtr pyramid_ = new CompVImageScalePyramid(fScaleFactor, nLevels, eScaleType);
	COMPV_CHECK_EXP_RETURN(!pyramid_ || !pyramid_->m_bValid, COMPV_ERROR_CODE_E_OUT_OF_MEMORY, "Out of memory");
	
	*pyramid = pyramid_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()