/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_IMAGE_CONV_COMMON_H_)
#define _COMPV_BASE_IMAGE_CONV_COMMON_H_

#if defined(_COMPV_BASE_API_H_)
#error("This is a private file and must not be part of the API")
#endif

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"

#if !defined(COMPV_IMAGE_CONV_MIN_SAMPLES_PER_THREAD)
#define COMPV_IMAGE_CONV_MIN_SAMPLES_PER_THREAD		(200 * 200) // minimum number of samples to consider per thread when multi-threading
#endif

COMPV_NAMESPACE_BEGIN()

// Read "documentation/yuv_rgb_conv.txt" for more info on how we computed these coeffs
// RGBA -> YUV
extern COMPV_BASE_API COMPV_ALIGN_DEFAULT() int8_t kRGBAToYUV_YCoeffs8[];
extern COMPV_BASE_API COMPV_ALIGN_DEFAULT() int8_t kRGBAToYUV_UCoeffs8[];
extern COMPV_BASE_API COMPV_ALIGN_DEFAULT() int8_t kRGBAToYUV_VCoeffs8[];
extern COMPV_BASE_API COMPV_ALIGN_DEFAULT() int8_t kRGBAToYUV_UVCoeffs8[];
extern COMPV_BASE_API COMPV_ALIGN_DEFAULT() int8_t kRGBAToYUV_U2V2Coeffs8[];
extern COMPV_BASE_API COMPV_ALIGN_DEFAULT() int8_t kRGBAToYUV_U4V4Coeffs8[];
// ARGB -> YUV
extern COMPV_BASE_API COMPV_ALIGN_DEFAULT() int8_t kARGBToYUV_YCoeffs8[];
extern COMPV_BASE_API COMPV_ALIGN_DEFAULT() int8_t kARGBToYUV_UCoeffs8[];
extern COMPV_BASE_API COMPV_ALIGN_DEFAULT() int8_t kARGBToYUV_VCoeffs8[];
//  BGRA -> YUV
extern COMPV_BASE_API COMPV_ALIGN_DEFAULT() int8_t kBGRAToYUV_YCoeffs8[];
extern COMPV_BASE_API COMPV_ALIGN_DEFAULT() int8_t kBGRAToYUV_UCoeffs8[];
extern COMPV_BASE_API COMPV_ALIGN_DEFAULT() int8_t kBGRAToYUV_VCoeffs8[];
// ABGR -> YUV
extern COMPV_BASE_API COMPV_ALIGN_DEFAULT() int8_t kABGRToYUV_YCoeffs8[];
extern COMPV_BASE_API COMPV_ALIGN_DEFAULT() int8_t kABGRToYUV_UCoeffs8[];
extern COMPV_BASE_API COMPV_ALIGN_DEFAULT() int8_t kABGRToYUV_VCoeffs8[];
// RGB -> YUV
extern COMPV_BASE_API COMPV_ALIGN_DEFAULT() int8_t kRGBToYUV_YCoeffs8[];
extern COMPV_BASE_API COMPV_ALIGN_DEFAULT() int8_t kRGBToYUV_UCoeffs8[];
extern COMPV_BASE_API COMPV_ALIGN_DEFAULT() int8_t kRGBToYUV_VCoeffs8[];
// BGR -> YUV
extern COMPV_BASE_API COMPV_ALIGN_DEFAULT() int8_t kBGRToYUV_YCoeffs8[];
extern COMPV_BASE_API COMPV_ALIGN_DEFAULT() int8_t kBGRToYUV_UCoeffs8[];
extern COMPV_BASE_API COMPV_ALIGN_DEFAULT() int8_t kBGRToYUV_VCoeffs8[];

// YUV -> RGBA
extern COMPV_BASE_API COMPV_ALIGN_DEFAULT() int8_t kYUVToRGBA_RCoeffs8[];
extern COMPV_BASE_API COMPV_ALIGN_DEFAULT() int8_t kYUVToRGBA_GCoeffs8[];
extern COMPV_BASE_API COMPV_ALIGN_DEFAULT() int8_t kYUVToRGBA_BCoeffs8[];

extern COMPV_BASE_API COMPV_ALIGN_DEFAULT() int32_t kShuffleEpi8_RgbToRgba_i32[];

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_IMAGE_CONV_COMMON_H_ */
