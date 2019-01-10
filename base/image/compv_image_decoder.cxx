/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/compv_image_decoder.h"
#include "compv/base/compv_base.h"
#include "compv/base/compv_fileutils.h"

COMPV_NAMESPACE_BEGIN()

#if defined(HAVE_LIBJPEG)
extern COMPV_ERROR_CODE libjpegDecodeFile(const char* filePath, CompVMatPtrPtr mat);
extern COMPV_ERROR_CODE libjpegDecodeInfo(const char* filePath, CompVImageInfo& info);
#endif

bool CompVImageDecoder::s_bInitialize = false;
CompVDecodeFileFuncPtr CompVImageDecoder::s_funcptrDecodeFileJpeg = NULL;
CompVDecodeInfoFuncPtr CompVImageDecoder::s_funcptrDecodeInfoJpeg = NULL;

#define kModuleNameImageDecoder "ImageDecoder"

COMPV_ERROR_CODE CompVImageDecoder::init()
{
    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;

    if (!CompVImageDecoder::s_bInitialize) {
        COMPV_DEBUG_INFO_EX(kModuleNameImageDecoder, "Initializing image decoder...");

        /* Setting decoders function pointers in CompVBase is deprecated (to avoid linking against 3rd-party libraries in base). *Must* use CompVDrawing.*/
#if defined(HAVE_LIBJPEG)
        CompVImageDecoder::s_funcptrDecodeFileJpeg = libjpegDecodeFile;
        CompVImageDecoder::s_funcptrDecodeInfoJpeg = libjpegDecodeInfo;
#endif /* HAVE_LIBJPEG */

        COMPV_CHECK_CODE_BAIL(err_ = COMPV_ERROR_CODE_S_OK);

        CompVImageDecoder::s_bInitialize = true;
    }

bail:
    return err_;
}

COMPV_ERROR_CODE CompVImageDecoder::deInit()
{
    if (CompVImageDecoder::s_bInitialize) {
        CompVImageDecoder::s_funcptrDecodeFileJpeg = NULL;
        CompVImageDecoder::s_funcptrDecodeInfoJpeg = NULL;

        CompVImageDecoder::s_bInitialize = false;
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImageDecoder::setFuncPtrs(COMPV_IMAGE_FORMAT format, CompVDecodeFileFuncPtr funcptrDecodeFile, CompVDecodeInfoFuncPtr funcptrDecodeInfo)
{
    COMPV_DEBUG_INFO_EX(kModuleNameImageDecoder, "Setting decode function pointers for JPEG");
    COMPV_CHECK_EXP_RETURN(!CompVBase::isInitialized(), COMPV_ERROR_CODE_E_NOT_INITIALIZED);
    switch (format) {
    case COMPV_IMAGE_FORMAT_JPEG:
        CompVImageDecoder::s_funcptrDecodeFileJpeg = funcptrDecodeFile;
        CompVImageDecoder::s_funcptrDecodeInfoJpeg = funcptrDecodeInfo;
        break;
    case COMPV_IMAGE_FORMAT_RAW:
    case COMPV_IMAGE_FORMAT_BITMAP:
    case COMPV_IMAGE_FORMAT_PNG:
    default:
        COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_IMAGE_FORMAT);
        break;
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImageDecoder::decodeFile(const char* filePath, CompVMatPtrPtr mat)
{
    COMPV_CHECK_EXP_RETURN(!CompVBase::isInitialized(), COMPV_ERROR_CODE_E_NOT_INITIALIZED);
    if (CompVFileUtils::empty(filePath) || !CompVFileUtils::exists(filePath)) {
        COMPV_DEBUG_ERROR_EX(kModuleNameImageDecoder, "File is empty or doesn't exist: %s", filePath);
        return COMPV_ERROR_CODE_E_INVALID_PARAMETER;
    }
    COMPV_IMAGE_FORMAT format_ = CompVFileUtils::getImageFormat(filePath);
    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
    switch (format_) {
    case COMPV_IMAGE_FORMAT_JPEG:
        if (CompVImageDecoder::s_funcptrDecodeFileJpeg) {
            return CompVImageDecoder::s_funcptrDecodeFileJpeg(filePath, mat);
        }
        else {
            COMPV_DEBUG_ERROR_EX(kModuleNameImageDecoder, "Cannot find jpeg decoder for file: %s", filePath);
            COMPV_CHECK_CODE_BAIL(err_ = COMPV_ERROR_CODE_E_DECODER_NOT_FOUND);
        }
        break;
    case COMPV_IMAGE_FORMAT_RAW:
    case COMPV_IMAGE_FORMAT_BITMAP:
    case COMPV_IMAGE_FORMAT_PNG:
    default:
        COMPV_DEBUG_ERROR_EX(kModuleNameImageDecoder, "Cannot find decoder for file: %s, format: %d", filePath, format_);
        COMPV_CHECK_CODE_BAIL(err_ = COMPV_ERROR_CODE_E_DECODER_NOT_FOUND);
        break;
    }

bail:
    return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
}

COMPV_ERROR_CODE  CompVImageDecoder::decodeInfo(const char* filePath, CompVImageInfo& info)
{
	COMPV_CHECK_EXP_RETURN(!CompVBase::isInitialized(), COMPV_ERROR_CODE_E_NOT_INITIALIZED);
    if (CompVFileUtils::empty(filePath) || !CompVFileUtils::exists(filePath)) {
        COMPV_DEBUG_ERROR_EX(kModuleNameImageDecoder, "File is empty or doesn't exist: %s", filePath);
        return COMPV_ERROR_CODE_E_INVALID_PARAMETER;
    }
    COMPV_IMAGE_FORMAT format_ = CompVFileUtils::getImageFormat(filePath);
    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
    switch (format_) {
    case COMPV_IMAGE_FORMAT_JPEG:
        if (CompVImageDecoder::s_funcptrDecodeInfoJpeg) {
            return CompVImageDecoder::s_funcptrDecodeInfoJpeg(filePath, info);
        }
        else {
            COMPV_DEBUG_ERROR_EX(kModuleNameImageDecoder, "Cannot find jpeg decoder for file: %s", filePath);
            COMPV_CHECK_CODE_BAIL(err_ = COMPV_ERROR_CODE_E_DECODER_NOT_FOUND);
        }
        break;
    case COMPV_IMAGE_FORMAT_RAW:
    case COMPV_IMAGE_FORMAT_BITMAP:
    case COMPV_IMAGE_FORMAT_PNG:
    default:
        COMPV_DEBUG_ERROR_EX(kModuleNameImageDecoder, "Cannot find decoder for file: %s, format: %d", filePath, format_);
        COMPV_CHECK_CODE_BAIL(err_ = COMPV_ERROR_CODE_E_DECODER_NOT_FOUND);
        break;
    }

bail:
    return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
}

COMPV_NAMESPACE_END()
