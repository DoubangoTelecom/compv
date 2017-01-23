/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/image/scale/compv_imagescale_pyramid.h"
#include "compv/image/scale/compv_imagescale_common.h"
#include "compv/compv_engine.h"
#include "compv/compv_mem.h"

COMPV_NAMESPACE_BEGIN()

CompVImageScalePyramid::CompVImageScalePyramid(float fScaleFactor, int32_t nLevels, COMPV_SCALE_TYPE eScaleType /*= COMPV_SCALE_TYPE_BILINEAR*/)
    : m_bValid(false)
    , m_fScaleFactor(fScaleFactor)
    , m_fScaleFactorsSum(1.f)
    , m_nLevels(nLevels)
    , m_pImages(NULL)
    , m_pScaleFactors(NULL)
{
    COMPV_ASSERT(m_nLevels > 0 && m_fScaleFactor > 0); // never happen, we already checked it in newObj()
    m_pImages = (CompVPtr<CompVImage *>*)CompVMem::calloc(m_nLevels, sizeof(CompVPtr<CompVImage *>));
    if (!m_pImages) {
        COMPV_DEBUG_ERROR("Failed to allocate memory");
        return;
    }
    m_pScaleFactors = (float*)CompVMem::calloc(m_nLevels, sizeof(float));
    if (!m_pScaleFactors) {
        COMPV_DEBUG_ERROR("Failed to allocate memory");
        return;
    }
    // Compute sum of all scale factors. This is used to generate the contribution for each level
    // For example level3 will contribute: ((K * sf^3) / sfs)
    float sf_ = m_fScaleFactor;
    m_pScaleFactors[0] = 1.f;
    for (int32_t level = 1; level < m_nLevels; ++level, sf_ *= m_fScaleFactor) {
        m_pScaleFactors[level] = sf_;
        m_fScaleFactorsSum += sf_;
    }
    m_bValid = true;
}

CompVImageScalePyramid::~CompVImageScalePyramid()
{
    if (m_pImages) {
        for (int32_t i = 0; i < m_nLevels; ++i) {
            m_pImages[i] = NULL;
        }
        CompVMem::free((void**)&m_pImages);
    }
    if (m_pScaleFactors) {
        CompVMem::free((void**)&m_pScaleFactors);
    }
}

// Must be thread safe
COMPV_ERROR_CODE CompVImageScalePyramid::process(const CompVPtr<CompVImage*>& inImage, int32_t level /*= -1*/)
{
    COMPV_CHECK_EXP_RETURN(!inImage || (level >= m_nLevels), COMPV_ERROR_CODE_E_INVALID_PARAMETER);

    // TODO(dmi): We have two options here:
    //	1/ imageN=scale(imageN-1,sf)
    //	2/ imageN=scale(imageOrig,sf)
    // Option 2 produce better result in terms of quality and matching rate. Also option 2 can be multithreaded
    // which is not the case for option 1.
    // This funtion is sometimes multithreaded (e.g. when called from ORB dete), make sure we are using option 2

#if 1 // Option 2
    if (level < 0) {
        CompVPtr<CompVThreadDispatcher* >threadDisp = CompVEngine::getThreadDispatcher();
        int32_t threadsCount = 1;
        // Compute number of threads
        if (threadDisp && threadDisp->getThreadsCount() > 1 && !threadDisp->isMotherOfTheCurrentThread()) {
            threadsCount = threadDisp->getThreadsCount();
        }
        if (threadsCount > 1) {
            CompVPtr<CompVImageScalePyramid* >This = this;
            uint32_t threadIdx = threadDisp->getThreadIdxForNextToCurrentCore(); // start execution on the next CPU core
            // levelStart is used to make sure we won't schedule more than "threadsCount"
            int levelStart, lev, levelMax;
            for (levelStart = 0, levelMax = threadsCount; levelStart < m_nLevels; levelStart += threadsCount, levelMax += threadsCount) {
                for (lev = levelStart; lev < m_nLevels && lev < levelMax; ++lev) {
                    COMPV_CHECK_CODE_ASSERT(threadDisp->execute((uint32_t)(threadIdx + lev), COMPV_TOKENIDX0, CompVImageScalePyramid::processLevelAt_AsynExec,
                                            COMPV_ASYNCTASK_SET_PARAM_ASISS(*This, *inImage, lev),
                                            COMPV_ASYNCTASK_SET_PARAM_NULL()));
                }
                for (lev = levelStart; lev < m_nLevels && lev < levelMax; ++lev) {
                    COMPV_CHECK_CODE_ASSERT(threadDisp->wait((uint32_t)(threadIdx + lev), COMPV_TOKENIDX0));
                }
            }
        }
        else {
            for (int32_t lev = 0; lev < m_nLevels; ++lev) {
                COMPV_CHECK_CODE_RETURN(processLevelAt(inImage, lev));
            }
        }
    }
    else {
        COMPV_CHECK_CODE_RETURN(processLevelAt(inImage, level));
    }
#else
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // TODO(dmi): multi-thread
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

COMPV_ERROR_CODE CompVImageScalePyramid::getImage(int32_t level, CompVPtr<CompVImage *>* image)
{
    COMPV_CHECK_EXP_RETURN(!image || level >= m_nLevels, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    *image = m_pImages[level];
    return COMPV_ERROR_CODE_S_OK;
}

float CompVImageScalePyramid::getScaleFactor(int32_t level /*= COMPV_PYRAMOD_LEVEL_FIRST for level 0 it's always equal to 1.f*/)
{
    if (level >= m_nLevels) {
        COMPV_DEBUG_ERROR("Invalid level=%d", level);
        return 0.f;
    }
    return m_pScaleFactors[level];
}

// Private function
COMPV_ERROR_CODE CompVImageScalePyramid::processLevelAt(const CompVPtr<CompVImage*>& inImage, int32_t level)
{
    COMPV_CHECK_EXP_RETURN(!inImage || level < 0 || level >= m_nLevels, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    float sf = getScaleFactor(level);
    COMPV_CHECK_CODE_RETURN(inImage->scale(m_eScaleType, (int32_t)(inImage->getWidth() * sf), (int32_t)(inImage->getHeight() * sf), &m_pImages[level]));
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImageScalePyramid::processLevelAt_AsynExec(const struct compv_asynctoken_param_xs* pc_params)
{
    CompVImageScalePyramid*  This = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[0].pcParamPtr, CompVImageScalePyramid*);
    CompVImage* image = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[1].pcParamPtr, CompVImage*);
    int level = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[2].pcParamPtr, int);
    return This->processLevelAt(image, level);
}

COMPV_ERROR_CODE CompVImageScalePyramid::newObj(float fScaleFactor, int32_t nLevels, COMPV_SCALE_TYPE eScaleType, CompVPtr<CompVImageScalePyramid*>* pyramid)
{
    COMPV_CHECK_CODE_RETURN(CompVEngine::init());
    COMPV_CHECK_EXP_RETURN(pyramid == NULL || nLevels <= 0 || fScaleFactor <= 0, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVPtr<CompVImageScalePyramid*>pyramid_ = new CompVImageScalePyramid(fScaleFactor, nLevels, eScaleType);
    if (!pyramid_ || !pyramid_->m_bValid) {
        COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    }
    *pyramid = pyramid_;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
