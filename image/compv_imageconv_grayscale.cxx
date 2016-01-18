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
#include "compv/image/compv_imageconv_grayscale.h"
#include "compv/image/compv_imageconv_rgba_i420.h"
#include "compv/image/compv_imageconv_common.h"

COMPV_NAMESPACE_BEGIN()

COMPV_ERROR_CODE CompVImageConvGrayscale::rgbToGrayscale(const CompVObjWrapper<CompVImage* >& rgb, CompVObjWrapper<CompVImage* >& grayscale)
{
    return CompVImageConvRgbaI420::rgbToI420(rgb, grayscale);
}

COMPV_ERROR_CODE CompVImageConvGrayscale::bgrToGrayscale(const CompVObjWrapper<CompVImage* >& bgr, CompVObjWrapper<CompVImage* >& grayscale)
{
    return CompVImageConvRgbaI420::bgrToI420(bgr, grayscale);
}

COMPV_ERROR_CODE CompVImageConvGrayscale::rgbaToGrayscale(const CompVObjWrapper<CompVImage* >& rgba, CompVObjWrapper<CompVImage* >& grayscale)
{
    return CompVImageConvRgbaI420::rgbaToI420(rgba, grayscale);
}

COMPV_ERROR_CODE CompVImageConvGrayscale::argbToGrayscale(const CompVObjWrapper<CompVImage* >& argb, CompVObjWrapper<CompVImage* >& grayscale)
{
    return CompVImageConvRgbaI420::argbToI420(argb, grayscale);
}

COMPV_ERROR_CODE CompVImageConvGrayscale::bgraToGrayscale(const CompVObjWrapper<CompVImage* >& bgra, CompVObjWrapper<CompVImage* >& grayscale)
{
    return CompVImageConvRgbaI420::bgraToI420(bgra, grayscale);
}

COMPV_ERROR_CODE CompVImageConvGrayscale::abgrToGrayscale(const CompVObjWrapper<CompVImage* >& abgr, CompVObjWrapper<CompVImage* >& grayscale)
{
    return CompVImageConvRgbaI420::abgrToI420(abgr, grayscale);
}

COMPV_ERROR_CODE CompVImageConvGrayscale::i420ToGrayscale(const CompVObjWrapper<CompVImage* >& i420, CompVObjWrapper<CompVImage* >& grayscale)
{
    return CompVImage::wrap(COMPV_PIXEL_FORMAT_GRAYSCALE, i420->getDataPtr(), i420->getWidth(), i420->getHeight(), i420->getStride(), &grayscale);
}

COMPV_NAMESPACE_END()
