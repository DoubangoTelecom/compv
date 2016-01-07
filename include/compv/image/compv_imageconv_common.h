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

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_EXTERNC_BEGIN()

// Read "documentation/yuv_rgb_cov.txt" for more info on how we computed these coeffs
COMPV_GEXTERN COMV_ALIGN_DEFAULT() int8_t kRGBAToYUV_YCoeffs8[];
COMPV_GEXTERN COMV_ALIGN_DEFAULT() int8_t kRGBAToYUV_UCoeffs8[];
COMPV_GEXTERN COMV_ALIGN_DEFAULT() int8_t kRGBAToYUV_VCoeffs8[];
COMPV_GEXTERN COMV_ALIGN_DEFAULT() int8_t kRGBAToYUV_UVCoeffs8[];
COMPV_GEXTERN COMV_ALIGN_DEFAULT() int8_t kRGBAToYUV_U2V2Coeffs8[];
COMPV_GEXTERN COMV_ALIGN_DEFAULT() int8_t kRGBAToYUV_U4V4Coeffs8[];

#if !defined(COMPV_IMAGCONV_MIN_SAMPLES_PER_THREAD)
#define COMPV_IMAGCONV_MIN_SAMPLES_PER_THREAD (150 * 150) // minimum number of samples to consider per thread when multi-threading
#endif

COMPV_EXTERNC_END()

#endif /* _COMPV_IMAGE_IMAGECONV_COMMON_H_ */