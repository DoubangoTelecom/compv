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
#if !defined(_COMPV_IMAGE_IMAGECONV_RGBA_I420_INTRIN_SSE_H_)
#define _COMPV_IMAGE_IMAGECONV_RGBA_I420_INTRIN_SSE_H_

#include "compv/compv_config.h"
#include "compv/compv_common.h"
#include "compv/image/compv_image.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

#if defined(COMPV_ARCH_X86) && defined(COMPV_INTRINSIC)

COMPV_NAMESPACE_BEGIN()

void rgbaToI420Kernel11_CompY_Intrin_Aligned_SSSE3(COMV_ALIGNED(SSE) const uint8_t* rgbaPtr, uint8_t* outYPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride, COMV_ALIGNED(SSE) const int8_t* kXXXXToYUV_YCoeffs8);
void rgbaToI420Kernel41_CompY_Intrin_Aligned_SSSE3(COMV_ALIGNED(SSE) const uint8_t* rgbaPtr, uint8_t* outYPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride, COMV_ALIGNED(SSE) const int8_t* kXXXXToYUV_YCoeffs8);
void rgbaToI420Kernel11_CompUV_Intrin_Aligned_SSSE3(COMV_ALIGNED(SSE) const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride, COMV_ALIGNED(SSE) const int8_t* kXXXXToYUV_UCoeffs8, COMV_ALIGNED(SSE) const int8_t* kXXXXToYUV_VCoeffs8);
void rgbaToI420Kernel41_CompUV_Intrin_Aligned_SSSE3(COMV_ALIGNED(SSE) const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride, COMV_ALIGNED(SSE) const int8_t* kXXXXToYUV_UCoeffs8, COMV_ALIGNED(SSE) const int8_t* kXXXXToYUV_VCoeffs8);

void rgbToI420Kernel31_CompY_Intrin_Aligned_SSSE3(COMV_ALIGNED(SSE) const uint8_t* rgbPtr, uint8_t* outYPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride, COMV_ALIGNED(SSE) const int8_t* kXXXXToYUV_YCoeffs8);
void rgbToI420Kernel31_CompUV_Intrin_Aligned_SSSE3(COMV_ALIGNED(SSE) const uint8_t* rgbPtr, uint8_t* outUPtr, uint8_t* outVPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride, COMV_ALIGNED(SSE) const int8_t* kXXXXToYUV_UCoeffs8, COMV_ALIGNED(SSE) const int8_t* kXXXXToYUV_VCoeffs8);

void i420ToRGBAKernel11_Intrin_Aligned_SSSE3(COMV_ALIGNED(SSE) const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, COMV_ALIGNED(SSE) uint8_t* outRgbaPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride);
void i420ToRGBAKernel41_Intrin_Aligned_SSSE3(COMV_ALIGNED(SSE) const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, COMV_ALIGNED(SSE) uint8_t* outRgbaPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 */

#endif /* _COMPV_IMAGE_IMAGECONV_RGBA_I420_INTRIN_SSE_H_ */