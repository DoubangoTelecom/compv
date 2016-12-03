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

COMPV_ERROR_CODE CompVImageUtils::getSizeForPixelFormat(COMPV_SUBTYPE ePixelFormat, size_t width, size_t height, size_t *size)
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

COMPV_ERROR_CODE CompVImageUtils::getCompSizeForPixelFormat(COMPV_SUBTYPE ePixelFormat, size_t width, size_t height, size_t compId, size_t *size)
{
    COMPV_CHECK_EXP_RETURN(!size || compId < 0 || compId > COMPV_MAX_COMP_COUNT, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    switch (ePixelFormat) {
    case COMPV_SUBTYPE_PIXELS_RGB24:
    case COMPV_SUBTYPE_PIXELS_BGR24:
    case COMPV_SUBTYPE_PIXELS_RGBA32:
    case COMPV_SUBTYPE_PIXELS_BGRA32:
    case COMPV_SUBTYPE_PIXELS_ABGR32:
    case COMPV_SUBTYPE_PIXELS_ARGB32:
    case COMPV_SUBTYPE_PIXELS_Y:
        *size = width * height;
        return COMPV_ERROR_CODE_S_OK;
    case COMPV_SUBTYPE_PIXELS_YUV420P: // Y, U, V
        COMPV_CHECK_EXP_RETURN(compId > 3, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
        if (compId == 0) *size = (width * height);
        else  *size = ((width * height) >> 2); // Width and Height and subsampled : 2x2
        return COMPV_ERROR_CODE_S_OK;
	case COMPV_SUBTYPE_PIXELS_NV12: // Y, UV
	case COMPV_SUBTYPE_PIXELS_NV21: // Y, VU
		COMPV_CHECK_EXP_RETURN(compId > 2, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		if (compId == 0) *size = (width * height);
		else  *size = ((width * height) >> 1); // Width and Height and subsampled : 2x2 but UV is packed
		return COMPV_ERROR_CODE_S_OK;
	case COMPV_SUBTYPE_PIXELS_YUYV422:
		COMPV_CHECK_EXP_RETURN(compId > 3, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		if (compId == 0) *size = (width * height);
		else  *size = ((width >> 1) * height); // Width and Height and subsampled : 2x1
		return COMPV_ERROR_CODE_S_OK;
    default:
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT);
		return COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT;
    }
}

COMPV_ERROR_CODE CompVImageUtils::getCompSizeForPixelFormat(COMPV_SUBTYPE ePixelFormat, size_t compId, size_t imgWidth, size_t imgHeight, size_t *compWidth, size_t *compHeight)
{
    COMPV_CHECK_EXP_RETURN(!imgWidth || !imgHeight || !compWidth || !compHeight || compId < 0 || compId > 4, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    switch (ePixelFormat) {
    case COMPV_SUBTYPE_PIXELS_RGB24:
    case COMPV_SUBTYPE_PIXELS_BGR24:
    case COMPV_SUBTYPE_PIXELS_RGBA32:
    case COMPV_SUBTYPE_PIXELS_BGRA32:
    case COMPV_SUBTYPE_PIXELS_ABGR32:
    case COMPV_SUBTYPE_PIXELS_ARGB32:
    case COMPV_SUBTYPE_PIXELS_Y:
        *compWidth = imgWidth;
        *compHeight = imgHeight;
        return COMPV_ERROR_CODE_S_OK;
    case COMPV_SUBTYPE_PIXELS_YUV420P: // Y, U, V
        COMPV_CHECK_EXP_RETURN(compId > 3, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
        if (compId == 0)  *compWidth = imgWidth, *compHeight = imgHeight;
        else  *compWidth = imgWidth >> 1, *compHeight = imgHeight >> 1; // Width and Height and subsampled : 2x2
        return COMPV_ERROR_CODE_S_OK;
	case COMPV_SUBTYPE_PIXELS_NV12: // Y, UV
	case COMPV_SUBTYPE_PIXELS_NV21: // Y, VU
		COMPV_CHECK_EXP_RETURN(compId > 2, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		if (compId == 0)  *compWidth = imgWidth, *compHeight = imgHeight;
		else  *compWidth = imgWidth >> 1, *compHeight = imgHeight >> 1; // Width and Height and subsampled : 2x2, UV packed
		return COMPV_ERROR_CODE_S_OK;
	case COMPV_SUBTYPE_PIXELS_YUYV422:
		COMPV_CHECK_EXP_RETURN(compId > 3, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		if (compId == 0) *compWidth = imgWidth, *compHeight = imgHeight;
		else *compWidth = imgWidth >> 1, *compHeight = imgHeight; // Width and Height and subsampled : 2x1
		return COMPV_ERROR_CODE_S_OK;
    default:
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT);
		return COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT;
    }
}

COMPV_ERROR_CODE CompVImageUtils::getBitsCountForPixelFormat(COMPV_SUBTYPE ePixelFormat, size_t* bitsCount)
{
    COMPV_CHECK_EXP_RETURN(!bitsCount, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    switch (ePixelFormat) {
    case COMPV_SUBTYPE_PIXELS_RGB24:
    case COMPV_SUBTYPE_PIXELS_BGR24:
        *bitsCount = 3 << 3;
        return COMPV_ERROR_CODE_S_OK;

    case COMPV_SUBTYPE_PIXELS_RGBA32:
    case COMPV_SUBTYPE_PIXELS_BGRA32:
    case COMPV_SUBTYPE_PIXELS_ABGR32:
    case COMPV_SUBTYPE_PIXELS_ARGB32:
        *bitsCount = 4 << 3;
        return COMPV_ERROR_CODE_S_OK;
    case COMPV_SUBTYPE_PIXELS_YUV420P:
	case COMPV_SUBTYPE_PIXELS_NV12:
	case COMPV_SUBTYPE_PIXELS_NV21:
        *bitsCount = 12;
        return COMPV_ERROR_CODE_S_OK;
	case COMPV_SUBTYPE_PIXELS_YUYV422:
		*bitsCount = 16;
		return COMPV_ERROR_CODE_S_OK;
    case COMPV_SUBTYPE_PIXELS_Y:
        *bitsCount = 8;
        return COMPV_ERROR_CODE_S_OK;
    default:
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT);
		return COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT;
    }
}

COMPV_ERROR_CODE CompVImageUtils::getCompCount(COMPV_SUBTYPE ePixelFormat, size_t *compCount)
{
    COMPV_CHECK_EXP_RETURN(!compCount, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    switch (ePixelFormat) {
    case COMPV_SUBTYPE_PIXELS_RGB24:
    case COMPV_SUBTYPE_PIXELS_BGR24:
        *compCount = 3;
        return COMPV_ERROR_CODE_S_OK;
    case COMPV_SUBTYPE_PIXELS_RGBA32:
    case COMPV_SUBTYPE_PIXELS_BGRA32:
    case COMPV_SUBTYPE_PIXELS_ABGR32:
    case COMPV_SUBTYPE_PIXELS_ARGB32:
        *compCount = 4;
        return COMPV_ERROR_CODE_S_OK;
    case COMPV_SUBTYPE_PIXELS_YUV420P:
	case COMPV_SUBTYPE_PIXELS_YUYV422:
        *compCount = 3;
		return COMPV_ERROR_CODE_S_OK;
	case COMPV_SUBTYPE_PIXELS_NV12:
	case COMPV_SUBTYPE_PIXELS_NV21:
		*compCount = 2; // Y, then UV packed -> Semi-Planar
        return COMPV_ERROR_CODE_S_OK;
    case COMPV_SUBTYPE_PIXELS_Y:
        *compCount = 1;
        return COMPV_ERROR_CODE_S_OK;
    default:
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT);
        return COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT;
    }
}

COMPV_ERROR_CODE CompVImageUtils::getCompPacked(COMPV_SUBTYPE ePixelFormat, bool *packed)
{
    COMPV_CHECK_EXP_RETURN(!packed, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    switch (ePixelFormat) {
    case COMPV_SUBTYPE_PIXELS_RGB24:
    case COMPV_SUBTYPE_PIXELS_BGR24:
    case COMPV_SUBTYPE_PIXELS_RGBA32:
    case COMPV_SUBTYPE_PIXELS_BGRA32:
    case COMPV_SUBTYPE_PIXELS_ABGR32:
    case COMPV_SUBTYPE_PIXELS_ARGB32:
	case COMPV_SUBTYPE_PIXELS_YUYV422:
        *packed = true;
        return COMPV_ERROR_CODE_S_OK;
	case COMPV_SUBTYPE_PIXELS_Y:
    case COMPV_SUBTYPE_PIXELS_YUV420P:
	case COMPV_SUBTYPE_PIXELS_NV12: // Semi-Planar
	case COMPV_SUBTYPE_PIXELS_NV21: // Semi-Planar
        *packed = false; // Planar
        return COMPV_ERROR_CODE_S_OK;
    default:
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT);
		return COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT;
    }
}


COMPV_NAMESPACE_END()