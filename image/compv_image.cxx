/* Copyright (C) 2016 Doubango Telecom <https://www.doubango.org>
*
* This file is part of Open Source ComputerVision (a.k.a CompV) project.
* Source code hosted at https://github.com/DoubangoTelecom/compv
* Website hosted at http://compv.org
*
* CompV is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* CompV is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with CompV.
*/
#include "compv/image/compv_image.h"
#include "compv/image/conv/compv_imageconv_rgba_i420.h"
#include "compv/image/conv/compv_imageconv_rgba_rgb.h"
#include "compv/image/conv/compv_imageconv_grayscale.h"
#include "compv/image/scale/compv_imagescale_bilinear.h"
#include "compv/compv_mem.h"
#include "compv/compv_engine.h"
#include "compv/compv_fileutils.h"
#include "compv/compv_debug.h"

#include <algorithm>


COMPV_NAMESPACE_BEGIN()

//
//	CompVImage
//

CompVImage::CompVImage(COMPV_IMAGE_FORMAT eImageFormat, COMPV_PIXEL_FORMAT ePixelFormat)
    :CompVObj()
    , m_nWidth(0)
    , m_nHeight(0)
    , m_nStride(0)
	, m_nBorderWidth(0)
	, m_nBorderHeight(0)
	, m_nBorderStride(0)
    , m_ePixelFormat(ePixelFormat)
    , m_eImageFormat(eImageFormat)
	, m_eBorderType(COMPV_BORDER_TYPE_NONE)
{

}

CompVImage::~CompVImage()
{

}

COMPV_ERROR_CODE CompVImage::convert(COMPV_PIXEL_FORMAT eDstPixelFormat, CompVObjWrapper<CompVImage*>* outImage)
{
    COMPV_CHECK_EXP_RETURN(outImage == NULL, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    COMPV_CHECK_EXP_RETURN(m_eImageFormat != COMPV_IMAGE_FORMAT_RAW, COMPV_ERROR_CODE_E_INVALID_IMAGE_FORMAT); // We only support RAW -> RAW. If you have a JPEG image, decode it first then wrap it
    CompVObjWrapper<CompVImage*> This = this; // when outImage is equal to this and the caller doesn't hold a reference the object could be destroyed before the end of the call. This line increment the refCount.
    if (eDstPixelFormat == m_eImageFormat) {
        *outImage = This;
        return COMPV_ERROR_CODE_S_OK;
    }
    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
    int32_t neededBuffSize;
    COMPV_CHECK_CODE_RETURN(err_ = CompVImage::getSizeForPixelFormat(eDstPixelFormat, m_nStride, m_nHeight, &neededBuffSize));
    bool bAllocOutImage = (!(*outImage) || (*outImage)->getDataSize() != neededBuffSize || (*outImage)->getImageFormat() != COMPV_IMAGE_FORMAT_RAW || (*outImage)->getPixelFormat() != eDstPixelFormat);
    if (bAllocOutImage) {
        COMPV_CHECK_CODE_RETURN(err_ = CompVImage::newObj(eDstPixelFormat, m_nWidth, m_nHeight, m_nStride, outImage));
    }
    else {
        // even if allocation isn't required we could have different width, height or stride
        // for example, 128x255 image requires same number of bytes than 255x128
        CompVObjWrapper<CompVBuffer*> buffer = (*outImage)->m_oData; // increment refCount
        COMPV_CHECK_CODE_RETURN(err_ = (*outImage)->setBuffer(buffer, m_nWidth, m_nHeight, m_nStride)); // changing the current buffer's layout
    }
    switch (m_ePixelFormat) {
        /***** RGBA -> XXX *****/
    case COMPV_PIXEL_FORMAT_R8G8B8A8: {
        if (eDstPixelFormat == COMPV_PIXEL_FORMAT_I420) {
            // RGBA -> I420
            COMPV_CHECK_CODE_RETURN(err_ = CompVImageConvRgbaI420::rgbaToI420(this, *outImage));
        }
        else if (eDstPixelFormat == COMPV_PIXEL_FORMAT_GRAYSCALE) {
            // RGBA -> GRAYSCALE
            COMPV_CHECK_CODE_RETURN(err_ = CompVImageConvGrayscale::rgbaToGrayscale(this, *outImage));
        }
        else {
            COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT);
        }
        break;
    }
    /***** ARGB -> XXX *****/
    case COMPV_PIXEL_FORMAT_A8R8G8B8: {
        if (eDstPixelFormat == COMPV_PIXEL_FORMAT_I420) {
            // ARGB - > I420
            COMPV_CHECK_CODE_RETURN(err_ = CompVImageConvRgbaI420::argbToI420(this, *outImage));
        }
        else if (eDstPixelFormat == COMPV_PIXEL_FORMAT_GRAYSCALE) {
            // ARGB -> GRAYSCALE
            COMPV_CHECK_CODE_RETURN(err_ = CompVImageConvGrayscale::argbToGrayscale(this, *outImage));
        }
        else {
            COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT);
        }
        break;
    }
    /***** BGRA -> XXX *****/
    case COMPV_PIXEL_FORMAT_B8G8R8A8: {
        if (eDstPixelFormat == COMPV_PIXEL_FORMAT_I420) {
            // BGRA -> I420
            COMPV_CHECK_CODE_RETURN(err_ = CompVImageConvRgbaI420::bgraToI420(this, *outImage));
        }
        else if (eDstPixelFormat == COMPV_PIXEL_FORMAT_GRAYSCALE) {
            // BGRA -> GRAYSCALE
            COMPV_CHECK_CODE_RETURN(err_ = CompVImageConvGrayscale::bgraToGrayscale(this, *outImage));
        }
        else {
            COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT);
        }
        break;
    }
    /***** ABGR -> XXX *****/
    case COMPV_PIXEL_FORMAT_A8B8G8R8: {
        if (eDstPixelFormat == COMPV_PIXEL_FORMAT_I420) {
            // ABGR -> I420
            COMPV_CHECK_CODE_RETURN(err_ = CompVImageConvRgbaI420::abgrToI420(this, *outImage));
        }
        else if (eDstPixelFormat == COMPV_PIXEL_FORMAT_GRAYSCALE) {
            // ABGR -> GRAYSCALE
            COMPV_CHECK_CODE_RETURN(err_ = CompVImageConvGrayscale::abgrToGrayscale(this, *outImage));
        }
        else {
            COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT);
        }
        break;
    }
    /***** RGB -> XXX *****/
    case COMPV_PIXEL_FORMAT_R8G8B8: {
        if (eDstPixelFormat == COMPV_PIXEL_FORMAT_I420) {
            // RGB -> I420
            COMPV_CHECK_CODE_RETURN(err_ = CompVImageConvRgbaI420::rgbToI420(this, *outImage));
        }
        else if (eDstPixelFormat == COMPV_PIXEL_FORMAT_R8G8B8A8) {
            // RGB -> RGBA
            COMPV_CHECK_CODE_RETURN(err_ = CompVImageConvRgbaRgb::rgbToRgba(this, *outImage));
        }
        else if (eDstPixelFormat == COMPV_PIXEL_FORMAT_GRAYSCALE) {
            // RGB -> GRAYSCALE
            COMPV_CHECK_CODE_RETURN(err_ = CompVImageConvGrayscale::rgbToGrayscale(this, *outImage));
        }
        else {
            COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT);
        }
        break;
    }
    /***** BGR -> XXX *****/
    case COMPV_PIXEL_FORMAT_B8G8R8: {
        if (eDstPixelFormat == COMPV_PIXEL_FORMAT_I420) {
            // BGR -> I420
            COMPV_CHECK_CODE_RETURN(err_ = CompVImageConvRgbaI420::bgrToI420(this, *outImage));
        }
        else if (eDstPixelFormat == COMPV_PIXEL_FORMAT_B8G8R8A8) {
            // BGR -> BGRA
            COMPV_CHECK_CODE_RETURN(err_ = CompVImageConvRgbaRgb::bgrToBgra(this, *outImage));
        }
        else if (eDstPixelFormat == COMPV_PIXEL_FORMAT_GRAYSCALE) {
            // BGR -> GRAYSCALE
            COMPV_CHECK_CODE_RETURN(err_ = CompVImageConvGrayscale::bgrToGrayscale(this, *outImage));
        }
        else {
            COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT);
        }
        break;
    }
    /***** I420 -> XXX *****/
    case COMPV_PIXEL_FORMAT_I420: {
        if (eDstPixelFormat == COMPV_PIXEL_FORMAT_R8G8B8A8) {
            // I420 -> RGBA
            COMPV_CHECK_CODE_RETURN(err_ = CompVImageConvRgbaI420::i420ToRgba(this, *outImage));
        }
        else if (eDstPixelFormat == COMPV_PIXEL_FORMAT_GRAYSCALE) {
            // I420 -> GRAYSCALE
            COMPV_CHECK_CODE_RETURN(err_ = CompVImageConvGrayscale::i420ToGrayscale(this, *outImage));
        }
        else {
            COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT);
        }
        break;
    }
    default: {
        COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT);
        break;
    }
    }
    return err_;
}

COMPV_ERROR_CODE CompVImage::scale(COMPV_SCALE_TYPE type, int32_t outWidth, int32_t outHeight, CompVObjWrapper<CompVImage*>* outImage)
{
    COMPV_CHECK_EXP_RETURN(outImage == NULL || !outWidth || !outHeight, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
    int32_t neededBuffSize;
    int32_t outStride = outWidth;
    CompVObjWrapper<CompVImage*> This = this; // when outImage is equal to this and the caller doesn't hold a reference the object could be destroyed before the end of the call. This line increment the refCount.
    COMPV_CHECK_CODE_RETURN(err_ = CompVImage::getBestStride(outWidth, &outStride));
    COMPV_CHECK_CODE_RETURN(err_ = CompVImage::getSizeForPixelFormat(This->getPixelFormat(), outStride, outHeight, &neededBuffSize));
    bool bSelfTransfer = This == (*outImage);
    bool bScaleFactor1 = outWidth == This->getWidth() && outHeight == This->getHeight(); // *must* compute here before overriding This, otherwise always true
    bool bAllocOutImage = (!(*outImage) || (*outImage)->getDataSize() != neededBuffSize || (*outImage)->getImageFormat() != COMPV_IMAGE_FORMAT_RAW || (*outImage)->getPixelFormat() != This->getPixelFormat());
    if (bAllocOutImage) {
        COMPV_CHECK_CODE_RETURN(err_ = CompVImage::newObj(This->getPixelFormat(), outWidth, outHeight, outStride, outImage));
    }
    else {
        // even if allocation isn't required we could have different width, height or stride
        // for example, 128x255 image requires same number of bytes than 255x128
        CompVObjWrapper<CompVBuffer*> buffer = (*outImage)->m_oData; // increment refCount
        COMPV_CHECK_CODE_RETURN(err_ = (*outImage)->setBuffer(buffer, outWidth, outHeight, outStride)); // changing the current buffer's layout
    }
    if (bScaleFactor1 & !CompVEngine::isTestingMode()) { // In testing mode we may want to encode the same image several times to check CPU, Memory, Latency...
        if (bSelfTransfer && !bAllocOutImage) {
            // *outImage = This is enought
            return COMPV_ERROR_CODE_S_OK;
        }
        return CompVImage::copy(getPixelFormat(), getDataPtr(), getWidth(), getHeight(), getStride(), (void*)(*outImage)->getDataPtr(), (*outImage)->getWidth(), (*outImage)->getHeight(), (*outImage)->getStride());
    }

    switch (type) {
    case COMPV_SCALE_TYPE_BILINEAR:
        COMPV_CHECK_CODE_RETURN(err_ = CompVImageScaleBilinear::process(This, *outImage));
        break;
    default:
        COMPV_DEBUG_ERROR("%d not supported as scaling type", type);
        COMPV_CHECK_CODE_RETURN(err_ = COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
        break;
    }

    return err_;
}

COMPV_ERROR_CODE CompVImage::setBuffer(CompVObjWrapper<CompVBuffer*> & buffer, int32_t width, int32_t height, int32_t stride /*= 0*/)
{
    COMPV_CHECK_EXP_RETURN(!buffer || !width || !height, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    m_oData = buffer;
    m_nWidth = width;
    m_nHeight = height;
    m_nStride = (stride < width ? width : stride);
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImage::getBestStride(int32_t stride, int32_t *bestStride)
{
    COMPV_CHECK_EXP_RETURN(bestStride == NULL, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    *bestStride = (int32_t)CompVMem::alignForward(stride, CompVMem::getBestAlignment());
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImage::getSizeForPixelFormat(COMPV_PIXEL_FORMAT ePixelFormat, int32_t width, int32_t height, int32_t *size)
{
    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
    int32_t bitsCount;
    if (!size) {
        COMPV_DEBUG_ERROR("Invalid parameter");
        return COMPV_ERROR_CODE_E_INVALID_PARAMETER;
    }
    COMPV_CHECK_CODE_RETURN(err_ = CompVImage::getBitsCountForPixelFormat(ePixelFormat, &bitsCount));
    if (bitsCount & 7) {
        if (bitsCount == 12) { // 12/8 = 1.5 = 3/2
            *size = (((width * height) * 3) >> 1);
        }
        else {
            float f = ((float)bitsCount) / 8.f;
            *size = (int32_t)round((width * height) * f);
        }
    }
    else {
        *size = (width * height) * (bitsCount >> 3);
    }
    return err_;
}

COMPV_ERROR_CODE CompVImage::copy(COMPV_PIXEL_FORMAT ePixelFormat, const void* inPtr, int32_t inWidth, int32_t inHeight, int32_t inStride, void* outPtr, int32_t outWidth, int32_t outHeight, int32_t outStride)
{
    COMPV_CHECK_EXP_RETURN(inPtr == NULL || inWidth <= 0 || inHeight <= 0 || inStride <= 0 || inWidth > inStride || inStride == NULL || outWidth <= 0 || outHeight <= 0 || outWidth > outStride || outStride <= 0,
                           COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
	int32_t widthToCopySamples = std::min(inWidth, outWidth);
	int32_t strideToCopySamples = std::min(inStride, outStride);
	int32_t heightToCopySamples = std::min(inHeight, outHeight);

    switch (ePixelFormat) {
    case COMPV_PIXEL_FORMAT_R8G8B8:
    case COMPV_PIXEL_FORMAT_B8G8R8:
    case COMPV_PIXEL_FORMAT_R8G8B8A8:
    case COMPV_PIXEL_FORMAT_B8G8R8A8:
    case COMPV_PIXEL_FORMAT_A8B8G8R8:
    case COMPV_PIXEL_FORMAT_A8R8G8B8:
    case COMPV_PIXEL_FORMAT_GRAYSCALE: {
        int32_t bitsCount = 0;
        int32_t inStrideBytes, outStrideBytes;
        const uint8_t* inPtr_ = (const uint8_t*)inPtr;
        uint8_t* outPtr_ = (uint8_t*)outPtr;
        COMPV_CHECK_CODE_RETURN(err_ = CompVImage::getBitsCountForPixelFormat(ePixelFormat, &bitsCount));
        int32_t bytesCount = bitsCount >> 3;
        int32_t widthToCopyBytes = widthToCopySamples * bytesCount;
        inStrideBytes = inStride * bytesCount;
        outStrideBytes = outStride * bytesCount;
        // TODO(dmi): divide across Y and multi-thread
        for (int32_t j = 0; j < heightToCopySamples; ++j) {
            CompVMem::copy(outPtr_, inPtr_, widthToCopyBytes);
            outPtr_ += outStrideBytes;
            inPtr_ += inStrideBytes;
        }
        break;
    }
    case COMPV_PIXEL_FORMAT_I420: {
        uint8_t* outYPtr = (uint8_t*)outPtr;
        uint8_t* outUPtr = outYPtr + (outHeight * outStride);
        uint8_t* outVPtr = outUPtr + ((outHeight * outStride) >> 2);
        const uint8_t* inYPtr = (uint8_t*)inPtr;
        const uint8_t* inUPtr = inYPtr + (inHeight * inStride);
        const uint8_t* inVPtr = inUPtr + ((inHeight * inStride) >> 2);
        int32_t bytesToCopyY = widthToCopySamples * 1;
        int32_t bytesToCopyYDiv2 = bytesToCopyY >> 1;
        int32_t outStrideYDiv2 = outStride >> 1;
        int32_t inStrideYDiv2 = inStride >> 1;
        // TODO(dmi): divide across Y and multi-thread
        for (int32_t j = 0; j < heightToCopySamples; ++j) {
            CompVMem::copy(outYPtr, inYPtr, bytesToCopyY);
            if (j & 1) {
                CompVMem::copy(outUPtr, inUPtr, bytesToCopyYDiv2);
                CompVMem::copy(outVPtr, inVPtr, bytesToCopyYDiv2);
                outUPtr += outStrideYDiv2;
                outVPtr += outStrideYDiv2;
                inUPtr += inStrideYDiv2;
                inVPtr += inStrideYDiv2;
            }
            outYPtr += outStride;
            inYPtr += inStride;
        }
        break;
    }
    default:
        COMPV_DEBUG_ERROR("Invalid pixel format");
        return COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT;
    }
    return err_;
}

COMPV_ERROR_CODE CompVImage::wrap(COMPV_PIXEL_FORMAT ePixelFormat, const void* dataPtr, int32_t width, int32_t height, int32_t stride, CompVObjWrapper<CompVImage*>* image)
{
    COMPV_CHECK_EXP_RETURN(dataPtr == NULL || width <= 0 || height <= 0 || stride <= 0 || image == NULL, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
    int32_t bestStride = stride, bestAlign = CompVMem::getBestAlignment();
    bool bAllocNewImage, bAllocNewBuffer;
    CompVObjWrapper<CompVBuffer* > buffer;

    // Compute best stride
    COMPV_CHECK_CODE_BAIL(err_ = CompVImage::getBestStride(stride, &bestStride));

#if 1
    bAllocNewBuffer =
        stride != bestStride ||
        !COMPV_IS_ALIGNED(dataPtr, bestAlign);

    bAllocNewImage = !(*image) ||
                     (*image)->getImageFormat() != COMPV_IMAGE_FORMAT_RAW ||
                     (*image)->getPixelFormat() != ePixelFormat;
#else // to test copy and alloc
    COMPV_DEBUG_INFO_CODE_FOR_TESTING();
    bAllocNewBuffer = bAllocNewImage = true;
#endif

    if (bAllocNewBuffer) {
        void* buffData = NULL;
        int neededBuffSize = 0;
        COMPV_CHECK_CODE_BAIL(err_ = CompVImage::getSizeForPixelFormat(ePixelFormat, bestStride, height, &neededBuffSize));
        buffData = CompVMem::malloc(neededBuffSize); // we always alloc using bestAlign no need to use mallocAligned(ptr, bestAlign)
        if (!buffData) {
            COMPV_CHECK_CODE_BAIL(err_ = COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
        }
        err_ = CompVBuffer::newObjAndTakeData(&buffData, neededBuffSize, &buffer);
        if (COMPV_ERROR_CODE_IS_NOK(err_)) {
            CompVMem::free(&buffData);
            COMPV_CHECK_CODE_BAIL(err_);
        }
        COMPV_CHECK_CODE_BAIL(err_ = CompVImage::copy(ePixelFormat,
                                     dataPtr, width, height, stride,
                                     (void*)buffer->getPtr(), width, height, bestStride)); // copy data
    }
    else {
        int32_t dataSize;
        COMPV_CHECK_CODE_BAIL(err_ = CompVImage::getSizeForPixelFormat(ePixelFormat, stride, height, &dataSize));
        COMPV_CHECK_CODE_BAIL(err_ = CompVBuffer::newObjAndRefData(dataPtr, dataSize, &buffer)); // take a reference to the buffer, do not copy it
    }

    if (bAllocNewImage) {
        COMPV_CHECK_CODE_BAIL(err_ = CompVImage::newObj(COMPV_IMAGE_FORMAT_RAW, ePixelFormat, image));
    }

    COMPV_CHECK_CODE_BAIL(err_ = (*image)->setBuffer(buffer, width, height, bestStride));

bail:
    return err_;
}

COMPV_ERROR_CODE CompVImage::getBitsCountForPixelFormat(COMPV_PIXEL_FORMAT ePixelFormat, int32_t* bitsCount)
{
    if (!bitsCount) {
        COMPV_DEBUG_ERROR("Invalid parameter");
        return COMPV_ERROR_CODE_E_INVALID_PARAMETER;
    }
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

COMPV_ERROR_CODE CompVImage::newObj(COMPV_IMAGE_FORMAT eImageFormat, COMPV_PIXEL_FORMAT ePixelFormat, CompVObjWrapper<CompVImage*>* image)
{
    COMPV_CHECK_CODE_RETURN(CompVEngine::init());
    COMPV_CHECK_EXP_RETURN(image == NULL, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    *image = new CompVImage(eImageFormat, ePixelFormat);
    if (!*image) {
        COMPV_DEBUG_ERROR("Failed to alloc new 'CompVImage' object");
        return COMPV_ERROR_CODE_E_OUT_OF_MEMORY;
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImage::newObj(COMPV_PIXEL_FORMAT ePixelFormat, int32_t width, int32_t height, int32_t stride, CompVObjWrapper<CompVImage*>* image)
{
    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
    COMPV_CHECK_CODE_RETURN(err_ = CompVImage::newObj(COMPV_IMAGE_FORMAT_RAW, ePixelFormat, image));
    CompVObjWrapper<CompVBuffer* > buffer;
    void* buffData = NULL;
    int32_t neededBuffSize;
    COMPV_CHECK_CODE_BAIL(err_ = CompVImage::getSizeForPixelFormat(ePixelFormat, stride, height, &neededBuffSize));
    COMPV_CHECK_CODE_BAIL(err_ = CompVImage::newObj(COMPV_IMAGE_FORMAT_RAW, ePixelFormat, image));
    buffData = CompVMem::malloc(neededBuffSize);
    if (!buffData) {
        COMPV_CHECK_CODE_BAIL(err_ = COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    }
    err_ = CompVBuffer::newObjAndTakeData(&buffData, neededBuffSize, &buffer);
    if (COMPV_ERROR_CODE_IS_NOK(err_)) {
        CompVMem::free(&buffData);
        COMPV_CHECK_CODE_BAIL(err_);
    }
    COMPV_CHECK_CODE_BAIL(err_ = (*image)->setBuffer(buffer, width, height, stride));
bail:
    return  err_;
}

//
//	CompVImageDecoder
//

#if defined(HAVE_LIBJPEG)
extern COMPV_ERROR_CODE libjpegDecodeFile(const char* filePath, CompVObjWrapper<CompVImage*>* image);
extern COMPV_ERROR_CODE libjpegDecodeInfo(const char* filePath, CompVImageInfo& info);
#endif

CompVDecodeFileFuncPtr CompVImageDecoder::s_funcptrDecodeFileJpeg = NULL;
CompVDecodeInfoFuncPtr CompVImageDecoder::s_funcptrDecodeInfoJpeg = NULL;

#define kModuleNameImageDecoder "ImageDecoder"

COMPV_ERROR_CODE CompVImageDecoder::init()
{
    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
    static bool s_Initialized = false;

    if (!s_Initialized) {
        COMPV_DEBUG_INFO_EX(kModuleNameImageDecoder, "Initializing image decoder...");

#if defined(HAVE_LIBJPEG)
        CompVImageDecoder::s_funcptrDecodeFileJpeg = libjpegDecodeFile;
        CompVImageDecoder::s_funcptrDecodeInfoJpeg = libjpegDecodeInfo;
#else
        COMPV_DEBUG_WARN_EX(kModuleNameImageDecoder, "No jpeg decoder found");
#endif /* HAVE_LIBJPEG */

        COMPV_CHECK_CODE_BAIL(err_ = COMPV_ERROR_CODE_S_OK);

        s_Initialized = true;
    }

bail:
    return err_;
}

COMPV_ERROR_CODE CompVImageDecoder::decodeFile(const char* filePath, CompVObjWrapper<CompVImage*>* image)
{
    COMPV_CHECK_CODE_RETURN(CompVEngine::init());
    if (CompVFileUtils::empty(filePath) || !CompVFileUtils::exists(filePath)) {
        COMPV_DEBUG_ERROR_EX(kModuleNameImageDecoder, "File is empty or doesn't exist: %s", filePath);
        COMPV_ERROR_CODE_E_INVALID_PARAMETER;
    }
    _COMPV_IMAGE_FORMAT format_ = CompVFileUtils::getImageFormat(filePath);
    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
    switch (format_) {
    case COMPV_IMAGE_FORMAT_JPEG:
        if (CompVImageDecoder::s_funcptrDecodeFileJpeg) {
            return CompVImageDecoder::s_funcptrDecodeFileJpeg(filePath, image);
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
    COMPV_CHECK_CODE_RETURN(CompVEngine::init());
    if (CompVFileUtils::empty(filePath) || !CompVFileUtils::exists(filePath)) {
        COMPV_DEBUG_ERROR_EX(kModuleNameImageDecoder, "File is empty or doesn't exist: %s", filePath);
        COMPV_ERROR_CODE_E_INVALID_PARAMETER;
    }
    _COMPV_IMAGE_FORMAT format_ = CompVFileUtils::getImageFormat(filePath);
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
