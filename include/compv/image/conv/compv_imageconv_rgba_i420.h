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
#if !defined(_COMPV_IMAGE_CONV_IMAGECONV_RGBA_I420_H_)
#define _COMPV_IMAGE_CONV_IMAGECONV_RGBA_I420_H_

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
    static COMPV_ERROR_CODE rgbToI420(const CompVPtr<CompVImage* >& rgb, CompVPtr<CompVImage* >& i420);
    static COMPV_ERROR_CODE bgrToI420(const CompVPtr<CompVImage* >& bgr, CompVPtr<CompVImage* >& i420);

    static COMPV_ERROR_CODE rgbaToI420(const CompVPtr<CompVImage* >& rgba, CompVPtr<CompVImage* >& i420);
    static COMPV_ERROR_CODE argbToI420(const CompVPtr<CompVImage* >& argb, CompVPtr<CompVImage* >& i420);
    static COMPV_ERROR_CODE bgraToI420(const CompVPtr<CompVImage* >& bgra, CompVPtr<CompVImage* >& i420);
    static COMPV_ERROR_CODE abgrToI420(const CompVPtr<CompVImage* >& abgr, CompVPtr<CompVImage* >& i420);

    static COMPV_ERROR_CODE i420ToRgba(const CompVPtr<CompVImage* >& i420, CompVPtr<CompVImage* >& rgba);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_IMAGE_CONV_IMAGECONV_RGBA_I420_H_ */