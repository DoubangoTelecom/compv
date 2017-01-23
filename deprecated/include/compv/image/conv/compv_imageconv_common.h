/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_IMAGE_CONV_IMAGECONV_COMMON_H_)
#define _COMPV_IMAGE_CONV_IMAGECONV_COMMON_H_

#include "compv/compv_config.h"
#include "compv/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

#if !defined(COMPV_IMAGCONV_MIN_SAMPLES_PER_THREAD)
#define COMPV_IMAGCONV_MIN_SAMPLES_PER_THREAD (200 * 200) // minimum number of samples to consider per thread when multi-threading
#endif

COMPV_NAMESPACE_BEGIN()

enum {
    COMPV_IMAGECONV_FUNCID_RGBAToI420_YUV,

    COMPV_IMAGECONV_FUNCID_RGBToI420_YUV,
    COMPV_IMAGECONV_FUNCID_RGBToRGBA,

    COMPV_IMAGECONV_FUNCID_I420ToRGBA,

    // no limitation
};

COMPV_ERROR_CODE ImageConvKernelxx_AsynExec(const struct compv_asynctoken_param_xs* pc_params);

typedef void(*rgbaToI420Kernel_CompY)(const uint8_t* rgbaPtr, uint8_t* outYPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride, COMPV_ALIGNED(DEFAULT)const int8_t* kXXXToYUV_YCoeffs8); // rgba, argb, abgr, bgra, rgb, bgr....
typedef void(*rgbaToI420Kernel_CompUV)(const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride, COMPV_ALIGNED(DEFAULT)const int8_t* kXXXToYUV_UCoeffs8, COMPV_ALIGNED(DEFAULT)const int8_t* kXXXToYUV_VCoeffs8); // rgba, argb, abgr, bgra, rgb, bgr....
typedef rgbaToI420Kernel_CompY rgbToI420Kernel_CompY;
typedef rgbaToI420Kernel_CompUV rgbToI420Kernel_CompUV;
typedef void(*rgbToRgbaKernel)(const uint8_t* rgb, uint8_t* rgba, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride);
typedef void(*i420ToRGBAKernel)(const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, uint8_t* outRgbaPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride); // rgba, argb, abgr, bgra, rgb, bgr....

COMPV_NAMESPACE_END()

COMPV_EXTERNC_BEGIN()

// Read "documentation/yuv_rgb_cov.txt" for more info on how we computed these coeffs
// RGBA -> YUV
extern COMPV_API COMPV_ALIGN_DEFAULT() int8_t kRGBAToYUV_YCoeffs8[];
extern COMPV_API COMPV_ALIGN_DEFAULT() int8_t kRGBAToYUV_UCoeffs8[];
extern COMPV_API COMPV_ALIGN_DEFAULT() int8_t kRGBAToYUV_VCoeffs8[];
extern COMPV_API COMPV_ALIGN_DEFAULT() int8_t kRGBAToYUV_UVCoeffs8[];
extern COMPV_API COMPV_ALIGN_DEFAULT() int8_t kRGBAToYUV_U2V2Coeffs8[];
extern COMPV_API COMPV_ALIGN_DEFAULT() int8_t kRGBAToYUV_U4V4Coeffs8[];
// ARGB -> YUV
extern COMPV_API COMPV_ALIGN_DEFAULT() int8_t kARGBToYUV_YCoeffs8[];
extern COMPV_API COMPV_ALIGN_DEFAULT() int8_t kARGBToYUV_UCoeffs8[];
extern COMPV_API COMPV_ALIGN_DEFAULT() int8_t kARGBToYUV_VCoeffs8[];
//  BGRA -> YUV
extern COMPV_API COMPV_ALIGN_DEFAULT() int8_t kBGRAToYUV_YCoeffs8[];
extern COMPV_API COMPV_ALIGN_DEFAULT() int8_t kBGRAToYUV_UCoeffs8[];
extern COMPV_API COMPV_ALIGN_DEFAULT() int8_t kBGRAToYUV_VCoeffs8[];
// ABGR -> YUV
extern COMPV_API COMPV_ALIGN_DEFAULT() int8_t kABGRToYUV_YCoeffs8[];
extern COMPV_API COMPV_ALIGN_DEFAULT() int8_t kABGRToYUV_UCoeffs8[];
extern COMPV_API COMPV_ALIGN_DEFAULT() int8_t kABGRToYUV_VCoeffs8[];
// RGB -> YUV
extern COMPV_API COMPV_ALIGN_DEFAULT() int8_t kRGBToYUV_YCoeffs8[];
extern COMPV_API COMPV_ALIGN_DEFAULT() int8_t kRGBToYUV_UCoeffs8[];
extern COMPV_API COMPV_ALIGN_DEFAULT() int8_t kRGBToYUV_VCoeffs8[];
// BGR -> YUV
extern COMPV_API COMPV_ALIGN_DEFAULT() int8_t kBGRToYUV_YCoeffs8[];
extern COMPV_API COMPV_ALIGN_DEFAULT() int8_t kBGRToYUV_UCoeffs8[];
extern COMPV_API COMPV_ALIGN_DEFAULT() int8_t kBGRToYUV_VCoeffs8[];

// YUV -> RGBA
extern COMPV_API COMPV_ALIGN_DEFAULT() int8_t kYUVToRGBA_RCoeffs8[];
extern COMPV_API COMPV_ALIGN_DEFAULT() int8_t kYUVToRGBA_GCoeffs8[];
extern COMPV_API COMPV_ALIGN_DEFAULT() int8_t kYUVToRGBA_BCoeffs8[];

extern COMPV_API COMPV_ALIGN_DEFAULT() int32_t kShuffleEpi8_RgbToRgba_i32[];

COMPV_EXTERNC_END()

#endif /* _COMPV_IMAGE_CONV_IMAGECONV_COMMON_H_ */