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

COMPV_ERROR_CODE CompVImageConv::rgbaToI420(const uint8_t* rgbaPtr, int32_t height, int32_t width, int32_t stride, CompVObjWrapper<CompVImage* >* i420)
{
	COMPV_CHECK_EXP_RETURN(rgbaPtr == NULL || !width || !height || !stride || width > stride || !i420, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
	// FIXME: use convenient functions
	int32_t neededBuffSize = ((stride * height) * 3) >> 1;
	uint8_t *outYPtr, *outUPtr, *outVPtr;
	if (!(*i420) || (*i420)->getDataSize() != neededBuffSize || (*i420)->getImageFormat() != COMPV_IMAGE_FORMAT_RAW || (*i420)->getPixelFormat() != COMPV_PIXEL_FORMAT_I420) {
		CompVObjWrapper<CompVBuffer* > buffer;
		void* buffData = NULL;
		COMPV_CHECK_CODE_BAIL(err_ = CompVImage::newObj(COMPV_IMAGE_FORMAT_RAW, COMPV_PIXEL_FORMAT_I420, i420));
		buffData = CompVMem::malloc(neededBuffSize);
		if (!buffData) {
			COMPV_CHECK_CODE_BAIL(err_ = COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
		}
		err_ = CompVBuffer::newObjAndTakeData(&buffData, neededBuffSize, &buffer);
		if (COMPV_ERROR_CODE_IS_NOK(err_)) {
			CompVMem::free(&buffData);
			COMPV_CHECK_CODE_BAIL(err_);
		}
		COMPV_CHECK_CODE_BAIL(err_ = (*i420)->setBuffer(buffer, width, height, stride));
	}

	outYPtr = (uint8_t*)(*i420)->getDataPtr();
	outUPtr = outYPtr + (height * stride);
	outVPtr = outUPtr + ((height * stride) >> 2);
	CompVImageConvArgbI420::fromRGBA(rgbaPtr, height, width, stride, outYPtr, outUPtr, outVPtr);

bail:
	return err_;
}

COMPV_ERROR_CODE CompVImageConv::i420ToRGBA(const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, int32_t height, int32_t width, int32_t stride, CompVObjWrapper<CompVImage* >* rgba)
{
	COMPV_CHECK_EXP_RETURN(yPtr == NULL || uPtr == NULL || !vPtr || !width || !height || !stride || width > stride || !rgba, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
	// FIXME: use convenient functions
	int32_t neededBuffSize = (stride * height) << 2;
	if (!(*rgba) || (*rgba)->getDataSize() != neededBuffSize || (*rgba)->getImageFormat() != COMPV_IMAGE_FORMAT_RAW || (*rgba)->getPixelFormat() != COMPV_PIXEL_FORMAT_R8G8B8A8) {
		CompVObjWrapper<CompVBuffer* > buffer;
		void* buffData = NULL;
		COMPV_CHECK_CODE_BAIL(err_ = CompVImage::newObj(COMPV_IMAGE_FORMAT_RAW, COMPV_PIXEL_FORMAT_R8G8B8A8, rgba));
		buffData = CompVMem::malloc(neededBuffSize);
		if (!buffData) {
			COMPV_CHECK_CODE_BAIL(err_ = COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
		}
		err_ = CompVBuffer::newObjAndTakeData(&buffData, neededBuffSize, &buffer);
		if (COMPV_ERROR_CODE_IS_NOK(err_)) {
			CompVMem::free(&buffData);
			COMPV_CHECK_CODE_BAIL(err_);
		}
		COMPV_CHECK_CODE_BAIL(err_ = (*rgba)->setBuffer(buffer, width, height, stride));
	}

	CompVImageConvArgbI420::fromI420(yPtr, uPtr, vPtr, (uint8_t*)(*rgba)->getDataPtr(), height, width, stride);

bail:
	return err_;
}

COMPV_NAMESPACE_END()
