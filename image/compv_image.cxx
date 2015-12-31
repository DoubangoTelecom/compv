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
#include "compv/compv_mem.h"
#include "compv/compv_fileutils.h"
#include "compv/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

//
//	CompVImage
//

CompVImage::CompVImage(COMPV_IMAGE_FORMAT eImageFormat, COMPV_PIXEL_FORMAT ePixelFormat)
	:CompVObj()
	, m_nWidth(0)
	, m_nHeight(0)
	, m_ePixelFormat(ePixelFormat)
	, m_eImageFormat(eImageFormat)
{

}

CompVImage::~CompVImage()
{

}

COMPV_ERROR_CODE CompVImage::setBuffer(CompVObjWrapper<CompVBuffer*> & buffer, size_t width, size_t height)
{
	if (!buffer || !width || !height) {
		COMPV_DEBUG_ERROR("Invalid parameter");
		return COMPV_ERROR_CODE_E_INVALID_PARAMETER;
	}
	m_oData = buffer;
	m_nWidth = width;
	m_nHeight = height;
	return COMPV_ERROR_CODE_S_OK;
}

CompVObjWrapper<CompVImage*> CompVImage::loadImage(const char* filePath)
{
	COMPV_DEBUG_ERROR("Not implemented");

	return NULL;
}

COMPV_ERROR_CODE CompVImage::getSizeForPixelFormat(COMPV_PIXEL_FORMAT ePixelFormat, size_t width, size_t height, size_t *size)
{
	COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
	size_t bitsCount;
	if (!size) {
		COMPV_DEBUG_ERROR("Invalid parameter");
		return COMPV_ERROR_CODE_E_INVALID_PARAMETER;
	}
	COMPV_CHECK_CODE_RETURN(err_ = CompVImage::getBitsCountForPixelFormat(ePixelFormat, &bitsCount));
	if (bitsCount & 0x7) {
		COMPV_DEBUG_ERROR("Invalid bitsCount=%d", bitsCount);
		COMPV_CHECK_CODE_RETURN(err_ = COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT);
	}
	*size = (width * height) * (bitsCount >> 3);
	return err_;
}

COMPV_ERROR_CODE CompVImage::getBitsCountForPixelFormat(COMPV_PIXEL_FORMAT ePixelFormat, size_t* bitsCount)
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
	default:
		COMPV_DEBUG_ERROR("Invalid pixel format");
		return COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT;
	}
}

COMPV_ERROR_CODE CompVImage::newObj(COMPV_IMAGE_FORMAT eImageFormat, COMPV_PIXEL_FORMAT ePixelFormat, CompVObjWrapper<CompVImage*>* image)
{
	if (!image) {
		COMPV_DEBUG_ERROR("Invalid parameter");
		return COMPV_ERROR_CODE_E_INVALID_PARAMETER;
	}
	*image = new CompVImage(eImageFormat, ePixelFormat);
	if (!*image) {
		COMPV_DEBUG_ERROR("Failed to alloc new 'CompVImage' object");
		return COMPV_ERROR_CODE_E_OUT_OF_MEMORY;
	}
	return COMPV_ERROR_CODE_S_OK;
}

//
//	CompVImageDecoder
//

CompVDecodeFileFuncPtr CompVImageDecoder::s_funcptrDecodeFileJpeg = NULL;
CompVDecodeInfoFuncPtr CompVImageDecoder::s_funcptrDecodeInfoJpeg = NULL;

#define kModuleNameImageDecoder "ImageDecoder"

COMPV_ERROR_CODE CompVImageDecoder::init()
{
	COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;

	COMPV_DEBUG_INFO_EX(kModuleNameImageDecoder, "Initializing image decoder...");

#if defined(HAVE_LIBJPEG)
	extern COMPV_ERROR_CODE libjpegDecodeFile(const char* filePath, CompVObjWrapper<CompVImage*>* image);
	extern COMPV_ERROR_CODE libjpegDecodeInfo(const char* filePath, CompVImageInfo& info);
	CompVImageDecoder::s_funcptrDecodeFileJpeg = libjpegDecodeFile;
	CompVImageDecoder::s_funcptrDecodeInfoJpeg = libjpegDecodeInfo;
#else
	COMPV_DEBUG_WARN_EX(kModuleNameImageDecoder, "No jpeg decoder found");
#endif /* HAVE_LIBJPEG */

	COMPV_CHECK_CODE_BAIL(err_ = COMPV_ERROR_CODE_S_OK);

bail:
	return err_;
}

COMPV_ERROR_CODE CompVImageDecoder::decodeFile(const char* filePath, CompVObjWrapper<CompVImage*>* image)
{
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
