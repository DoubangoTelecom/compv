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
#if !defined(_COMPV_IMAGE_CONV_IMAGECONV_RGBA_RGB_INTRIN_AVX2_H_)
#define _COMPV_IMAGE_CONV_IMAGECONV_RGBA_RGB_INTRIN_AVX2_H_

#include "compv/compv_config.h"
#include "compv/compv_common.h"
#include "compv/image/compv_image.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

#if defined(COMPV_ARCH_X86) && defined(COMPV_INTRINSIC)

COMPV_NAMESPACE_BEGIN()

void rgbToRgbaKernel31_Intrin_Aligned_AVX2(COMV_ALIGNED(AVX2) const uint8_t* rgb, COMV_ALIGNED(AVX2) uint8_t* rgba, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride);
void bgrToBgraKernel31_Intrin_Aligned_AVX2(COMV_ALIGNED(AVX2) const uint8_t* bgr, COMV_ALIGNED(AVX2) uint8_t* bgra, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride);

COMPV_NAMESPACE_END()

#endif /* defined(COMPV_ARCH_X86) && defined(COMPV_INTRINSIC) */

#endif /* _COMPV_IMAGE_CONV_IMAGECONV_RGBA_RGB_INTRIN_AVX2_H_ */
