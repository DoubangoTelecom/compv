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
#include "compv/image/compv_imageconv.h"
#include "compv/image/compv_imageconv_rgba_i420.h"
#include "compv/compv_mem.h"
#include "compv/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

CompVImageConv::CompVImageConv()
{

}

CompVImageConv::~CompVImageConv()
{

}

static COMPV_ERROR_CODE __allocImage(COMPV_PIXEL_FORMAT ePixelFormat, int32_t height, int32_t width, int32_t stride, CompVObjWrapper<CompVImage* >* image)
{
    COMPV_CHECK_EXP_RETURN(!width || !height || !stride || width > stride || !image, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
    int32_t neededBuffSize;

    COMPV_CHECK_CODE_BAIL(err_ = CompVImage::getSizeForPixelFormat(ePixelFormat, stride, height, &neededBuffSize));
    if (!(*image) || (*image)->getDataSize() != neededBuffSize || (*image)->getImageFormat() != COMPV_IMAGE_FORMAT_RAW || (*image)->getPixelFormat() != ePixelFormat) {
        CompVObjWrapper<CompVBuffer* > buffer;
        void* buffData = NULL;
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
    }

bail:
    return err_;
}

static COMPV_ERROR_CODE __rgbaToI420(const uint8_t* rgbaPtr, int32_t height, int32_t width, int32_t stride, CompVObjWrapper<CompVImage* >* i420, int compSize, int ridx, int gidx, int bidx, int aidx = INT_MAX)
{
    COMPV_CHECK_EXP_RETURN(rgbaPtr == NULL || !width || !height || !stride || width > stride || !i420, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    COMPV_CHECK_CODE_RETURN(__allocImage(COMPV_PIXEL_FORMAT_I420, height, width, stride, i420));

    uint8_t* outYPtr = (uint8_t*)(*i420)->getDataPtr();
    uint8_t* outUPtr = outYPtr + (height * stride);
    uint8_t* outVPtr = outUPtr + ((height * stride) >> 2);
    switch (compSize) {
    case 4:
        if (aidx == 0 && ridx == 1) {
            COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_PARAMETER);    // argb
        }
        else if (aidx == 0 && bidx == 1) {
            COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_PARAMETER);    // abgr
        }
        else if (aidx == 3 && ridx == 0) {
            COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_PARAMETER);    // rgba
        }
        else if (aidx == 3 && bidx == 1) {
            COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_PARAMETER);    // bgra
        }
        else {
            COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_PARAMETER);
        }
        break;
    case 3:
        if (ridx == 0) ; //rgb
        else if (ridx == 2) {
            COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_PARAMETER);    //bgr
        }
        else {
            COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_PARAMETER);
        }
        break;
    default:
        COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_PARAMETER);
        break;
    }
    CompVImageConvRgbaI420::rgbaToI420(rgbaPtr, height, width, stride, outYPtr, outUPtr, outVPtr);

    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImageConv::rgbaToI420(const uint8_t* rgbaPtr, int32_t height, int32_t width, int32_t stride, CompVObjWrapper<CompVImage* >* i420)
{
    COMPV_CHECK_EXP_RETURN(rgbaPtr == NULL || !width || !height || !stride || width > stride || !i420, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    COMPV_CHECK_CODE_RETURN(__allocImage(COMPV_PIXEL_FORMAT_I420, height, width, stride, i420));

    uint8_t* outYPtr = (uint8_t*)(*i420)->getDataPtr();
    uint8_t* outUPtr = outYPtr + (height * stride);
    uint8_t* outVPtr = outUPtr + ((height * stride) >> 2);
    CompVImageConvRgbaI420::rgbaToI420(rgbaPtr, height, width, stride, outYPtr, outUPtr, outVPtr);

    return COMPV_ERROR_CODE_S_OK;
}

static COMPV_ERROR_CODE argbToI420(const uint8_t* argbPtr, int32_t height, int32_t width, int32_t stride, CompVObjWrapper<CompVImage* >* i420)
{

}

COMPV_ERROR_CODE CompVImageConv::i420ToRGBA(const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, int32_t height, int32_t width, int32_t stride, CompVObjWrapper<CompVImage* >* rgba)
{
    COMPV_CHECK_EXP_RETURN(yPtr == NULL || uPtr == NULL || !vPtr || !width || !height || !stride || width > stride || !rgba, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    COMPV_CHECK_CODE_RETURN(__allocImage(COMPV_PIXEL_FORMAT_R8G8B8A8, height, width, stride, rgba));

    CompVImageConvRgbaI420::i420ToRgba(yPtr, uPtr, vPtr, (uint8_t*)(*rgba)->getDataPtr(), height, width, stride);

    return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
