/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/compv_image_utils.h"
#include "compv/base/math/compv_math_utils.h"
#include "compv/base/compv_mem.h"

COMPV_NAMESPACE_BEGIN()

COMPV_ERROR_CODE CompVImageUtils::getBestStride(size_t stride, size_t *bestStride)
{
    COMPV_CHECK_EXP_RETURN(!bestStride, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    *bestStride = (int32_t)CompVMem::alignForward(stride, COMPV_SIMD_ALIGNV_DEFAULT);
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImageUtils::getSizeForPixelFormat(COMPV_PIXEL_FORMAT ePixelFormat, size_t width, size_t height, size_t *size)
{
    COMPV_CHECK_EXP_RETURN(!size, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
    size_t bitsCount;
    COMPV_CHECK_CODE_RETURN(err_ = CompVImageUtils::getBitsCountForPixelFormat(ePixelFormat, &bitsCount));
    if (bitsCount & 7) {
        if (bitsCount == 12) { // 12/8 = 1.5 = 3/2
            *size = (((width * height) * 3) >> 1);
        }
        else {
            float f = ((float)bitsCount) / 8.f;
            *size = COMPV_MATH_ROUNDFU_2_INT(((width * height) * f), int32_t);
        }
    }
    else {
        *size = (width * height) * (bitsCount >> 3);
    }
    return err_;
}

COMPV_ERROR_CODE CompVImageUtils::getCompSizeForPixelFormat(COMPV_PIXEL_FORMAT ePixelFormat, size_t width, size_t height, size_t compId, size_t *size)
{
    COMPV_CHECK_EXP_RETURN(!size || compId < 0 || compId > 4, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    switch (ePixelFormat) {
    case COMPV_PIXEL_FORMAT_R8G8B8:
    case COMPV_PIXEL_FORMAT_B8G8R8:
    case COMPV_PIXEL_FORMAT_R8G8B8A8:
    case COMPV_PIXEL_FORMAT_B8G8R8A8:
    case COMPV_PIXEL_FORMAT_A8B8G8R8:
    case COMPV_PIXEL_FORMAT_A8R8G8B8:
    case COMPV_PIXEL_FORMAT_GRAYSCALE:
        *size = width * height;
        return COMPV_ERROR_CODE_S_OK;
    case COMPV_PIXEL_FORMAT_I420:
        COMPV_CHECK_EXP_RETURN(compId > 3, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
        if (compId == 0) {
            *size = (width * height);
        }
        else {
            *size = ((width * height) >> 4);
        }
        return COMPV_ERROR_CODE_S_OK;
    default:
        COMPV_DEBUG_ERROR("Invalid pixel format");
        return COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT;
    }
}

COMPV_ERROR_CODE CompVImageUtils::getCompSizeForPixelFormat(COMPV_PIXEL_FORMAT ePixelFormat, size_t compId, size_t imgWidth, size_t imgHeight, size_t *compWidth, size_t *compHeight)
{
    COMPV_CHECK_EXP_RETURN(!imgWidth || !imgHeight || !compWidth || !compHeight || compId < 0 || compId > 4, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    switch (ePixelFormat) {
    case COMPV_PIXEL_FORMAT_R8G8B8:
    case COMPV_PIXEL_FORMAT_B8G8R8:
    case COMPV_PIXEL_FORMAT_R8G8B8A8:
    case COMPV_PIXEL_FORMAT_B8G8R8A8:
    case COMPV_PIXEL_FORMAT_A8B8G8R8:
    case COMPV_PIXEL_FORMAT_A8R8G8B8:
    case COMPV_PIXEL_FORMAT_GRAYSCALE:
        *compWidth = imgWidth;
        *compHeight = imgHeight;
        return COMPV_ERROR_CODE_S_OK;
    case COMPV_PIXEL_FORMAT_I420:
        COMPV_CHECK_EXP_RETURN(compId > 3, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
        if (compId == 0) {
            *compWidth = imgWidth, *compHeight = imgHeight;
        }
        else {
            *compWidth = imgWidth >> 1, *compHeight = imgHeight >> 1;
        }
        return COMPV_ERROR_CODE_S_OK;
    default:
        COMPV_DEBUG_ERROR("Invalid pixel format");
        return COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT;
    }
}

COMPV_ERROR_CODE CompVImageUtils::getBitsCountForPixelFormat(COMPV_PIXEL_FORMAT ePixelFormat, size_t* bitsCount)
{
    COMPV_CHECK_EXP_RETURN(!bitsCount, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    switch (ePixelFormat) {
    case COMPV_PIXEL_FORMAT_R8G8B8:
    case COMPV_PIXEL_FORMAT_B8G8R8:
        *bitsCount = 3 << 3;
        return COMPV_ERROR_CODE_S_OK;

    case COMPV_PIXEL_FORMAT_R8G8B8A8:
    case COMPV_PIXEL_FORMAT_B8G8R8A8:
    case COMPV_PIXEL_FORMAT_A8B8G8R8:
    case COMPV_PIXEL_FORMAT_A8R8G8B8:
        *bitsCount = 4 << 3;
        return COMPV_ERROR_CODE_S_OK;
    case COMPV_PIXEL_FORMAT_I420:
        *bitsCount = 12;
        return COMPV_ERROR_CODE_S_OK;
    case COMPV_PIXEL_FORMAT_GRAYSCALE:
        *bitsCount = 8;
        return COMPV_ERROR_CODE_S_OK;
    default:
        COMPV_DEBUG_ERROR("Invalid pixel format");
        return COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT;
    }
}

COMPV_ERROR_CODE CompVImageUtils::getCompCount(COMPV_PIXEL_FORMAT ePixelFormat, size_t *compCount)
{
    COMPV_CHECK_EXP_RETURN(!compCount, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    switch (ePixelFormat) {
    case COMPV_PIXEL_FORMAT_R8G8B8:
    case COMPV_PIXEL_FORMAT_B8G8R8:
        *compCount = 3;
        return COMPV_ERROR_CODE_S_OK;

    case COMPV_PIXEL_FORMAT_R8G8B8A8:
    case COMPV_PIXEL_FORMAT_B8G8R8A8:
    case COMPV_PIXEL_FORMAT_A8B8G8R8:
    case COMPV_PIXEL_FORMAT_A8R8G8B8:
        *compCount = 4;
        return COMPV_ERROR_CODE_S_OK;
    case COMPV_PIXEL_FORMAT_I420:
        *compCount = 3;
        return COMPV_ERROR_CODE_S_OK;
    case COMPV_PIXEL_FORMAT_GRAYSCALE:
        *compCount = 1;
        return COMPV_ERROR_CODE_S_OK;
    default:
        COMPV_DEBUG_ERROR("Invalid pixel format");
        return COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT;
    }
}

COMPV_ERROR_CODE CompVImageUtils::getCompInterleaved(COMPV_PIXEL_FORMAT ePixelFormat, bool *interleaved)
{
    COMPV_CHECK_EXP_RETURN(!interleaved, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    switch (ePixelFormat) {
    case COMPV_PIXEL_FORMAT_R8G8B8:
    case COMPV_PIXEL_FORMAT_B8G8R8:
    case COMPV_PIXEL_FORMAT_R8G8B8A8:
    case COMPV_PIXEL_FORMAT_B8G8R8A8:
    case COMPV_PIXEL_FORMAT_A8B8G8R8:
    case COMPV_PIXEL_FORMAT_A8R8G8B8:
    case COMPV_PIXEL_FORMAT_GRAYSCALE:
        *interleaved = true;
        return COMPV_ERROR_CODE_S_OK;
    case COMPV_PIXEL_FORMAT_I420:
        *interleaved = false;
        return COMPV_ERROR_CODE_S_OK;
    default:
        COMPV_DEBUG_ERROR("Invalid pixel format");
        return COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT;
    }
}


COMPV_NAMESPACE_END()