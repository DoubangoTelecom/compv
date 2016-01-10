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
#if !defined(_COMPV_IMAGE_IMAGECONV_RGBA_I420_H_)
#define _COMPV_IMAGE_IMAGECONV_RGBA_I420_H_

#include "compv/compv_config.h"
#include "compv/compv_common.h"
#include "compv/image/compv_image.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class CompVImageConvRgbaI420
{
public:
	static void rgbaToI420(const uint8_t* rgbaPtr, int32_t height, int32_t width, int32_t stride, uint8_t* outYPtr, uint8_t* outUPtr, uint8_t* outVPtr);
	static void i420ToRgba(const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, uint8_t* outRgbaPtr, int32_t height, int32_t width, int32_t stride);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_IMAGE_IMAGECONV_RGBA_I420_H_ */