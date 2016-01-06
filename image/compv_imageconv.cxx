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
#include "compv/image/compv_imageconv_to_i420.h"
#include "compv/compv_mem.h"
#include "compv/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

extern COMPV_ERROR_CODE CompVInit();

CompVObjWrapper<CompVThreadDispatcher* > CompVImageConv::s_ThreadDisp = NULL;

CompVImageConv::CompVImageConv()
{

}

CompVImageConv::~CompVImageConv()
{

}

COMPV_ERROR_CODE CompVImageConv::multiThreadingEnable(CompVObjWrapper<CompVThreadDispatcher* > dispatcher)
{
	COMPV_CHECK_CODE_RETURN(CompVInit());
	COMPV_CHECK_EXP_RETURN(!dispatcher, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	s_ThreadDisp = dispatcher;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImageConv::multiThreadingDisable()
{
	s_ThreadDisp = NULL;
	return COMPV_ERROR_CODE_S_OK;
}

bool CompVImageConv::isMultiThreadingEnabled()
{
	return !!s_ThreadDisp;
}

CompVObjWrapper<CompVThreadDispatcher* >& CompVImageConv::getThreadDispatcher()
{
	return s_ThreadDisp;
}

COMPV_ERROR_CODE CompVImageConv::getBestStride(size_t stride, size_t *bestStride)
{
	COMPV_CHECK_EXP_RETURN(bestStride == NULL, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	*bestStride = (stride + (stride & (COMPV_SIMD_ALIGNV_DEFAULT - 1)));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImageConv::rgbaToI420(const uint8_t* rgbaPtr, size_t width, size_t height, size_t stride, CompVObjWrapper<CompVImage* >* i420)
{
	COMPV_CHECK_EXP_RETURN(rgbaPtr == NULL || !width || !height || !stride || !i420, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
	// FIXME: use convenient functions
	size_t neededBuffSize = ((stride * height) * 3) >> 1;
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
	CompVImageConvToI420::fromRGBA(rgbaPtr, width, height, stride, outYPtr, outUPtr, outVPtr);

bail:
	return err_;
}

COMPV_NAMESPACE_END()
