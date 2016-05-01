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
#include "compv/image/conv/compv_imageconv_grayscale.h"
#include "compv/image/conv/compv_imageconv_rgba_i420.h"
#include "compv/image/conv/compv_imageconv_common.h"

COMPV_NAMESPACE_BEGIN()

COMPV_ERROR_CODE CompVImageConvGrayscale::rgbToGrayscale(const CompVPtr<CompVImage* >& rgb, CompVPtr<CompVImage* >& grayscale)
{
    return CompVImageConvRgbaI420::rgbToI420(rgb, grayscale);
}

COMPV_ERROR_CODE CompVImageConvGrayscale::bgrToGrayscale(const CompVPtr<CompVImage* >& bgr, CompVPtr<CompVImage* >& grayscale)
{
    return CompVImageConvRgbaI420::bgrToI420(bgr, grayscale);
}

COMPV_ERROR_CODE CompVImageConvGrayscale::rgbaToGrayscale(const CompVPtr<CompVImage* >& rgba, CompVPtr<CompVImage* >& grayscale)
{
    return CompVImageConvRgbaI420::rgbaToI420(rgba, grayscale);
}

COMPV_ERROR_CODE CompVImageConvGrayscale::argbToGrayscale(const CompVPtr<CompVImage* >& argb, CompVPtr<CompVImage* >& grayscale)
{
    return CompVImageConvRgbaI420::argbToI420(argb, grayscale);
}

COMPV_ERROR_CODE CompVImageConvGrayscale::bgraToGrayscale(const CompVPtr<CompVImage* >& bgra, CompVPtr<CompVImage* >& grayscale)
{
    return CompVImageConvRgbaI420::bgraToI420(bgra, grayscale);
}

COMPV_ERROR_CODE CompVImageConvGrayscale::abgrToGrayscale(const CompVPtr<CompVImage* >& abgr, CompVPtr<CompVImage* >& grayscale)
{
    return CompVImageConvRgbaI420::abgrToI420(abgr, grayscale);
}

COMPV_ERROR_CODE CompVImageConvGrayscale::i420ToGrayscale(const CompVPtr<CompVImage* >& i420, CompVPtr<CompVImage* >& grayscale)
{
    return CompVImage::wrap(COMPV_PIXEL_FORMAT_GRAYSCALE, i420->getDataPtr(), i420->getWidth(), i420->getHeight(), i420->getStride(), &grayscale);
}

COMPV_NAMESPACE_END()
