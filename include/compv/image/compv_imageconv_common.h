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
#if !defined(_COMPV_IMAGE_IMAGECONV_COMMON_H_)
#define _COMPV_IMAGE_IMAGECONV_COMMON_H_

#include "compv/compv_config.h"
#include "compv/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

#if !defined(COMPV_IMAGCONV_MIN_SAMPLES_PER_THREAD)
#define COMPV_IMAGCONV_MIN_SAMPLES_PER_THREAD (150 * 150) // minimum number of samples to consider per thread when multi-threading
#endif

#if !defined(COMPV_IMAGCONV_MIN_SAMPLES_PER_THREAD)
#define COMPV_IMAGCONV_MIN_SAMPLES_PER_THREAD (150 * 150) // minimum number of samples to consider per thread when multi-threading
#endif

COMPV_NAMESPACE_BEGIN()

enum {
	COMPV_IMAGECONV_FUNCID_RGBAToI420_Y,
	COMPV_IMAGECONV_FUNCID_RGBAToI420_UV,
	COMPV_IMAGECONV_FUNCID_I420ToRGBA

	// no limitation
};
COMPV_ERROR_CODE ImageConvKernelxx_AsynExec(const struct compv_asynctoken_param_xs* pc_params);

typedef void(*rgbaToI420Kernel_CompY)(const uint8_t* rgbaPtr, uint8_t* outYPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride);
typedef void(*rgbaToI420Kernel_CompUV)(const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride);
typedef void(*i420ToRGBAKernel)(const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, uint8_t* outRgbaPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride);

COMPV_NAMESPACE_END()

COMPV_EXTERNC_BEGIN()

// Read "documentation/yuv_rgb_cov.txt" for more info on how we computed these coeffs
COMPV_GEXTERN COMV_ALIGN_DEFAULT() int8_t kRGBAToYUV_YCoeffs8[];
COMPV_GEXTERN COMV_ALIGN_DEFAULT() int8_t kRGBAToYUV_UCoeffs8[];
COMPV_GEXTERN COMV_ALIGN_DEFAULT() int8_t kRGBAToYUV_VCoeffs8[];
COMPV_GEXTERN COMV_ALIGN_DEFAULT() int8_t kRGBAToYUV_UVCoeffs8[];
COMPV_GEXTERN COMV_ALIGN_DEFAULT() int8_t kRGBAToYUV_U2V2Coeffs8[];
COMPV_GEXTERN COMV_ALIGN_DEFAULT() int8_t kRGBAToYUV_U4V4Coeffs8[];
COMPV_GEXTERN COMV_ALIGN_DEFAULT() int8_t kYUVToRGBA_RCoeffs8[];
COMPV_GEXTERN COMV_ALIGN_DEFAULT() int8_t kYUVToRGBA_GCoeffs8[];
COMPV_GEXTERN COMV_ALIGN_DEFAULT() int8_t kYUVToRGBA_BCoeffs8[];

COMPV_EXTERNC_END()

#endif /* _COMPV_IMAGE_IMAGECONV_COMMON_H_ */