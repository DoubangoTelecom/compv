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
#include "compv/image/conv/compv_imageconv_common.h"
#include "compv/parallel/compv_asynctask.h"

COMPV_NAMESPACE_BEGIN()

COMPV_ERROR_CODE ImageConvKernelxx_AsynExec(const struct compv_asynctoken_param_xs* pc_params)
{
    const int funcId = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[0].pcParamPtr, int);
    switch (funcId) {
    case COMPV_IMAGECONV_FUNCID_RGBAToI420_YUV: 
	case COMPV_IMAGECONV_FUNCID_RGBToI420_YUV:{
        rgbaToI420Kernel_CompY CompY = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[1].pcParamPtr, rgbaToI420Kernel_CompY);
        rgbaToI420Kernel_CompUV CompUV = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[2].pcParamPtr, rgbaToI420Kernel_CompUV);
        const uint8_t* rgbaPtr = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[3].pcParamPtr, const uint8_t*);
        uint8_t* outYPtr = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[4].pcParamPtr, uint8_t*);
        uint8_t* outUPtr = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[5].pcParamPtr, uint8_t*);
        uint8_t* outVPtr = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[6].pcParamPtr, uint8_t*);
        compv_scalar_t height = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[7].pcParamPtr, int32_t);
        compv_scalar_t width = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[8].pcParamPtr, int32_t);
        compv_scalar_t stride = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[9].pcParamPtr, int32_t);
        COMPV_ALIGNED(DEFAULT) const int8_t* kXXXToYUV_YCoeffs8 = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[10].pcParamPtr, const int8_t*);
        COMPV_ALIGNED(DEFAULT)const int8_t* kXXXToYUV_UCoeffs8 = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[11].pcParamPtr, const int8_t*);
        COMPV_ALIGNED(DEFAULT)const int8_t* kXXXToYUV_VCoeffs8 = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[12].pcParamPtr, const int8_t*);
        CompY(rgbaPtr, outYPtr, height, width, stride, kXXXToYUV_YCoeffs8);
        if (outUPtr && outVPtr) {
            CompUV(rgbaPtr, outUPtr, outVPtr, height, width, stride, kXXXToYUV_UCoeffs8, kXXXToYUV_VCoeffs8);
        }
        break;
    }
    case COMPV_IMAGECONV_FUNCID_RGBToRGBA: {
        rgbToRgbaKernel toRGBA = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[1].pcParamPtr, rgbToRgbaKernel);
        const uint8_t* rgb = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[2].pcParamPtr, const uint8_t*);
        uint8_t* rgba = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[3].pcParamPtr, uint8_t*);
        compv_scalar_t height = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[4].pcParamPtr, int32_t);
        compv_scalar_t width = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[5].pcParamPtr, int32_t);
        compv_scalar_t stride = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[6].pcParamPtr, int32_t);
        toRGBA(rgb, rgba, height, width, stride);
        break;
    }
    case COMPV_IMAGECONV_FUNCID_I420ToRGBA: {
        i420ToRGBAKernel toRGBA = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[1].pcParamPtr, i420ToRGBAKernel);
        const uint8_t* yPtr = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[2].pcParamPtr, const uint8_t*);
        const uint8_t* uPtr = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[3].pcParamPtr, const uint8_t*);
        const uint8_t* vPtr = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[4].pcParamPtr, const uint8_t*);
        uint8_t* outRgbaPtr = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[5].pcParamPtr, uint8_t*);
        compv_scalar_t height = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[6].pcParamPtr, int32_t);
        compv_scalar_t width = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[7].pcParamPtr, int32_t);
        compv_scalar_t stride = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[8].pcParamPtr, int32_t);
        toRGBA(yPtr, uPtr, vPtr, outRgbaPtr, height, width, stride);
        break;
    }
    default:
        COMPV_DEBUG_ERROR("%d is an invalid funcId", funcId);
        return COMPV_ERROR_CODE_E_INVALID_CALL;
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()


/* RGB to YUV conversion : http ://www.fourcc.org/fccyvrgb.php
Y = (0.257 * R) + (0.504 * G) + (0.098 * B) + 16
Cr = V = (0.439 * R) - (0.368 * G) - (0.071 * B) + 128
Cb = U = -(0.148 * R) - (0.291 * G) + (0.439 * B) + 128
and we know that :
Y = (Y * 256) / 256
V = (V * 256) / 256
U = (U * 256) / 256
= >
Y = ((65.792 * R) + (129.024 * G) + (25.088 * B) + 4096) >> 8
V = ((112.384 * R) + (-94.208 * G) + (-18.176 * B) + 32768) >> 8
U = ((-37.888 * R) + (-74.496 * G) + (112.384 * B) + 32768) >> 8
Numerical approx. = >
Y = (((66 * R) + (129 * G) + (25 * B))) >> 8 + 16
= (2 * ((33 * R) + (65 * G) + (13 * B))) >> 8 + 16
= (((33 * R) + (65 * G) + (13 * B))) >> 7 + 16
U = (((-38 * R) + (-74 * G) + (112 * B))) >> 8 + 128
V = (((112 * R) + (-94 * G) + (-18 * B))) >> 8 + 128
*/
#define RY 33
#define GY 65
#define BY 13
#define AY 0
#define RU -38
#define GU -74
#define BU 112
#define AU 0
#define RV 112
#define GV -94
#define BV -18
#define AV 0
////// RGBA -> YUV //////
COMPV_GEXTERN COMPV_ALIGN_DEFAULT() int8_t kRGBAToYUV_YCoeffs8[] = {
    RY, GY, BY, AY, RY, GY, BY, AY, RY, GY, BY, AY, RY, GY, BY, AY, // 128bits SSE register
    RY, GY, BY, AY, RY, GY, BY, AY, RY, GY, BY, AY, RY, GY, BY, AY, // 256bits AVX register
};
COMPV_GEXTERN COMPV_ALIGN_DEFAULT() int8_t kRGBAToYUV_UCoeffs8[] = {
    RU, GU, BU, AU, RU, GU, BU, AU, RU, GU, BU, AU, RU, GU, BU, AU, // 128bits SSE register
    RU, GU, BU, AU, RU, GU, BU, AU, RU, GU, BU, AU, RU, GU, BU, AU, // 256bits AVX register
};
COMPV_GEXTERN COMPV_ALIGN_DEFAULT() int8_t kRGBAToYUV_VCoeffs8[] = {
    RV, GV, BV, AV, RV, GV, BV, AV, RV, GV, BV, AV, RV, GV, BV, AV, // 128bits SSE register
    RV, GV, BV, AV, RV, GV, BV, AV, RV, GV, BV, AV, RV, GV, BV, AV, // 256bits AVX register
};
COMPV_GEXTERN COMPV_ALIGN_DEFAULT() int8_t kRGBAToYUV_UVCoeffs8[] = { // U and V interleaved: Each appear #1 time: UVUVUVUV....
    RU, GU, BU, AU, RV, GV, BV, AV, RU, GU, BU, AU, RV, GV, BV, AV,
    RU, GU, BU, AU, RV, GV, BV, AV, RU, GU, BU, AU, RV, GV, BV, AV,
};
COMPV_GEXTERN COMPV_ALIGN_DEFAULT() int8_t kRGBAToYUV_U2V2Coeffs8[] = { // U and V interleaved: Each appear #2 times: UUVVUUVVUUVV....
    RU, GU, BU, AU, RU, GU, BU, AU, RV, GV, BV, AV, RV, GV, BV, AV,
    RU, GU, BU, AU, RU, GU, BU, AU, RV, GV, BV, AV, RV, GV, BV, AV,
};

COMPV_GEXTERN COMPV_ALIGN_DEFAULT() int8_t kRGBAToYUV_U4V4Coeffs8[] = { // AVX-only: U and V interleaved: Each appear #4 times: UUUUVVVVUUUUVVVV.....
    RU, GU, BU, AU, RU, GU, BU, AU, RU, GU, BU, AU, RU, GU, BU, AU,
    RV, GV, BV, AV, RV, GV, BV, AV, RV, GV, BV, AV, RV, GV, BV, AV,
};
////// ARGB -> YUV //////
COMPV_GEXTERN COMPV_ALIGN_DEFAULT() int8_t kARGBToYUV_YCoeffs8[] = {
    AY, RY, GY, BY, AY, RY, GY, BY, AY, RY, GY, BY, AY, RY, GY, BY, // 128bits SSE register
    AY, RY, GY, BY, AY, RY, GY, BY, AY, RY, GY, BY, AY, RY, GY, BY, // 256bits AVX register
};
COMPV_GEXTERN COMPV_ALIGN_DEFAULT() int8_t kARGBToYUV_UCoeffs8[] = {
    AU, RU, GU, BU, AU, RU, GU, BU, AU, RU, GU, BU, AU, RU, GU, BU, // 128bits SSE register
    AU, RU, GU, BU, AU, RU, GU, BU, AU, RU, GU, BU, AU, RU, GU, BU, // 256bits AVX register
};
COMPV_GEXTERN COMPV_ALIGN_DEFAULT() int8_t kARGBToYUV_VCoeffs8[] = {
    AV, RV, GV, BV, AV, RV, GV, BV, AV, RV, GV, BV, AV, RV, GV, BV, // 128bits SSE register
    AV, RV, GV, BV, AV, RV, GV, BV, AV, RV, GV, BV, AV, RV, GV, BV, // 256bits AVX register
};
////// BGRA -> YUV //////
COMPV_GEXTERN COMPV_ALIGN_DEFAULT() int8_t kBGRAToYUV_YCoeffs8[] = {
    BY, GY, RY, AY, BY, GY, RY, AY, BY, GY, RY, AY, BY, GY, RY, AY, // 128bits SSE register
    BY, GY, RY, AY, BY, GY, RY, AY, BY, GY, RY, AY, BY, GY, RY, AY, // 256bits AVX register
};
COMPV_GEXTERN COMPV_ALIGN_DEFAULT() int8_t kBGRAToYUV_UCoeffs8[] = {
    BU, GU, RU, AU, BU, GU, RU, AU, BU, GU, RU, AU, BU, GU, RU, AU, // 128bits SSE register
    BU, GU, RU, AU, BU, GU, RU, AU, BU, GU, RU, AU, BU, GU, RU, AU, // 256bits AVX register
};
COMPV_GEXTERN COMPV_ALIGN_DEFAULT() int8_t kBGRAToYUV_VCoeffs8[] = {
    BV, GV, RV, AV, BV, GV, RV, AV, BV, GV, RV, AV, BV, GV, RV, AV, // 128bits SSE register
    BV, GV, RV, AV, BV, GV, RV, AV, BV, GV, RV, AV, BV, GV, RV, AV, // 256bits AVX register
};
////// ABGR -> YUV //////
COMPV_GEXTERN COMPV_ALIGN_DEFAULT() int8_t kABGRToYUV_YCoeffs8[] = {
    AY, BY, GY, RY, AY, BY, GY, RY, AY, BY, GY, RY, AY, BY, GY, RY, // 128bits SSE register
    AY, BY, GY, RY, AY, BY, GY, RY, AY, BY, GY, RY, AY, BY, GY, RY, // 256bits AVX register
};
COMPV_GEXTERN COMPV_ALIGN_DEFAULT() int8_t kABGRToYUV_UCoeffs8[] = {
    AU, BU, GU, RU, AU, BU, GU, RU, AU, BU, GU, RU, AU, BU, GU, RU, // 128bits SSE register
    AU, BU, GU, RU, AU, BU, GU, RU, AU, BU, GU, RU, AU, BU, GU, RU, // 256bits AVX register
};
COMPV_GEXTERN COMPV_ALIGN_DEFAULT() int8_t kABGRToYUV_VCoeffs8[] = {
    AV, BV, GV, RV, AV, BV, GV, RV, AV, BV, GV, RV, AV, BV, GV, RV, // 128bits SSE register
    AV, BV, GV, RV, AV, BV, GV, RV, AV, BV, GV, RV, AV, BV, GV, RV, // 256bits AVX register
};
////// RGB -> YUV //////
COMPV_GEXTERN COMPV_ALIGN_DEFAULT() int8_t kRGBToYUV_YCoeffs8[] = {
    RY, GY, BY, RY, GY, BY, RY, GY, BY, RY, GY, BY, RY, GY, BY, RY, // SSE+0, AVX+0
    GY, BY, RY, GY, BY, RY, GY, BY, RY, GY, BY, RY, GY, BY, RY, GY, // SSE+16
    BY, RY, GY, BY, RY, GY, BY, RY, GY, BY, RY, GY, BY, RY, GY, BY, // SSE+32, AVX+32
    RY, GY, BY, RY, GY, BY, RY, GY, BY, RY, GY, BY, RY, GY, BY, RY,
    GY, BY, RY, GY, BY, RY, GY, BY, RY, GY, BY, RY, GY, BY, RY, GY, // AVX+64
    BY, RY, GY, BY, RY, GY, BY, RY, GY, BY, RY, GY, BY, RY, GY, BY,
};
COMPV_GEXTERN COMPV_ALIGN_DEFAULT() int8_t kRGBToYUV_UCoeffs8[] = {
    RU, GU, BU, RU, GU, BU, RU, GU, BU, RU, GU, BU, RU, GU, BU, RU, // SSE+0, AVX+0
    GU, BU, RU, GU, BU, RU, GU, BU, RU, GU, BU, RU, GU, BU, RU, GU, // SSE+16
    BU, RU, GU, BU, RU, GU, BU, RU, GU, BU, RU, GU, BU, RU, GU, BU, // SSE+32, AVX+32
    RU, GU, BU, RU, GU, BU, RU, GU, BU, RU, GU, BU, RU, GU, BU, RU,
    GU, BU, RU, GU, BU, RU, GU, BU, RU, GU, BU, RU, GU, BU, RU, GU, // AVX+64
    BU, RU, GU, BU, RU, GU, BU, RU, GU, BU, RU, GU, BU, RU, GU, BU,
};
COMPV_GEXTERN COMPV_ALIGN_DEFAULT() int8_t kRGBToYUV_VCoeffs8[] = {
    RV, GV, BV, RV, GV, BV, RV, GV, BV, RV, GV, BV, RV, GV, BV, RV, // SSE+0, AVX+0
    GV, BV, RV, GV, BV, RV, GV, BV, RV, GV, BV, RV, GV, BV, RV, GV, // SSE+16
    BV, RV, GV, BV, RV, GV, BV, RV, GV, BV, RV, GV, BV, RV, GV, BV, // SSE+32, AVX+32
    RV, GV, BV, RV, GV, BV, RV, GV, BV, RV, GV, BV, RV, GV, BV, RV,
    GV, BV, RV, GV, BV, RV, GV, BV, RV, GV, BV, RV, GV, BV, RV, GV, // AVX+64
    BV, RV, GV, BV, RV, GV, BV, RV, GV, BV, RV, GV, BV, RV, GV, BV,
};
////// BGR -> YUV //////
COMPV_GEXTERN COMPV_ALIGN_DEFAULT() int8_t kBGRToYUV_YCoeffs8[] = {
    BY, GY, RY, BY, GY, RY, BY, GY, RY, BY, GY, RY, BY, GY, RY, BY, // SSE+0, AVX+0
    GY, RY, BY, GY, RY, BY, GY, RY, BY, GY, RY, BY, GY, RY, BY, GY, // SSE+16
    RY, BY, GY, RY, BY, GY, RY, BY, GY, RY, BY, GY, RY, BY, GY, RY, // SSE+32, AVX+32
    BY, GY, RY, BY, GY, RY, BY, GY, RY, BY, GY, RY, BY, GY, RY, BY,
    GY, RY, BY, GY, RY, BY, GY, RY, BY, GY, RY, BY, GY, RY, BY, GY, // AVX+64
    RY, BY, GY, RY, BY, GY, RY, BY, GY, RY, BY, GY, RY, BY, GY, RY,
};
COMPV_GEXTERN COMPV_ALIGN_DEFAULT() int8_t kBGRToYUV_UCoeffs8[] = {
    BU, GU, RU, BU, GU, RU, BU, GU, RU, BU, GU, RU, BU, GU, RU, BU, // SSE+0, AVX+0
    GU, RU, BU, GU, RU, BU, GU, RU, BU, GU, RU, BU, GU, RU, BU, GU, // SSE+16
    RU, BU, GU, RU, BU, GU, RU, BU, GU, RU, BU, GU, RU, BU, GU, RU, // SSE+32, AVX+32
    BU, GU, RU, BU, GU, RU, BU, GU, RU, BU, GU, RU, BU, GU, RU, BU,
    GU, RU, BU, GU, RU, BU, GU, RU, BU, GU, RU, BU, GU, RU, BU, GU, // AVX+64
    RU, BU, GU, RU, BU, GU, RU, BU, GU, RU, BU, GU, RU, BU, GU, RU,
};
COMPV_GEXTERN COMPV_ALIGN_DEFAULT() int8_t kBGRToYUV_VCoeffs8[] = {
    BV, GV, RV, BV, GV, RV, BV, GV, RV, BV, GV, RV, BV, GV, RV, BV, // SSE+0, AVX+0
    GV, RV, BV, GV, RV, BV, GV, RV, BV, GV, RV, BV, GV, RV, BV, GV, // SSE+16
    RV, BV, GV, RV, BV, GV, RV, BV, GV, RV, BV, GV, RV, BV, GV, RV, // SSE+32, AVX+32
    BV, GV, RV, BV, GV, RV, BV, GV, RV, BV, GV, RV, BV, GV, RV, BV,
    GV, RV, BV, GV, RV, BV, GV, RV, BV, GV, RV, BV, GV, RV, BV, GV, // AVX+64
    RV, BV, GV, RV, BV, GV, RV, BV, GV, RV, BV, GV, RV, BV, GV, RV,
};

#undef RY
#undef GY
#undef BY
#undef AY
#undef RU
#undef GU
#undef BU
#undef AU
#undef RV
#undef GV
#undef BV
#undef AV


/* YUV to RGB conversion : http ://www.fourcc.org/fccyvrgb.php
R = 1.164(Y - 16) + 1.596(V - 128)
G = 1.164(Y - 16) - 0.813(V - 128) - 0.391(U - 128)
B = 1.164(Y - 16) + 2.018(U - 128)
and we know that :
R = (R * 64) / 64
G = (G * 64) / 64
B = (B * 64) / 64
= >
R * 64 = 74(Y - 16) + 102(V - 128)
G * 64 = 74(Y - 16) - 52(V - 128) - 25(U - 128)
B * 64 = 74(Y - 16) + 129(U - 128)
= >
R * 64 = 74Y' + 0U' + 102V'
G * 64 = 74Y' - 25U' - 52V'
B * 64 = 74Y' + 129U' + 0V'
where Y'=(Y - 16), U' = (U - 128), V'=(V - 128)
= >
R * 32 = 37Y' + 0U' + 51V'
G * 32 = 37Y' - 13U' - 26V'
B * 32 = 37Y' + 65U' + 0V'
= >
R = (37Y' + 0U' + 51V') >> 5
G = (37Y' - 13U' - 26V') >> 5
B = (37Y' + 65U' + 0V') >> 5
*/
COMPV_GEXTERN COMPV_ALIGN_DEFAULT() int8_t kYUVToRGBA_RCoeffs8[] = { // Extended with a zero to have #4 coeffs: k0, k1, k2, 0
    37, 0, 51, 0, 37, 0, 51, 0, 37, 0, 51, 0, 37, 0, 51, 0, // 128bits SSE register
    37, 0, 51, 0, 37, 0, 51, 0, 37, 0, 51, 0, 37, 0, 51, 0, // 256bits AVX register
};
COMPV_GEXTERN COMPV_ALIGN_DEFAULT() int8_t kYUVToRGBA_GCoeffs8[] = { // Extended with a zero to have #4 coeffs: k0, k1, k2, 0
    37, -13, -26, 0, 37, -13, -26, 0, 37, -13, -26, 0, 37, -13, -26, 0, // 128bits SSE register
    37, -13, -26, 0, 37, -13, -26, 0, 37, -13, -26, 0, 37, -13, -26, 0, // 256bits AVX register
};
COMPV_GEXTERN COMPV_ALIGN_DEFAULT() int8_t kYUVToRGBA_BCoeffs8[] = { // Extended with a zero to have #4 coeffs: k0, k1, k2, 0
    37, 65, 0, 0, 37, 65, 0, 0, 37, 65, 0, 0, 37, 65, 0, 0, // 128bits SSE register
    37, 65, 0, 0, 37, 65, 0, 0, 37, 65, 0, 0, 37, 65, 0, 0, // 256bits AVX register
};


// Insert 8bytes every 24bytes
// The index-3 must be 0xff to produce zeros used later to generate the alpha channel
COMPV_GEXTERN COMPV_ALIGN_DEFAULT() int32_t kShuffleEpi8_RgbToRgba_i32[] = {
    COMPV_MM_SHUFFLE_EPI8(0xff, 2, 1, 0), COMPV_MM_SHUFFLE_EPI8(0xff, 5, 4, 3), COMPV_MM_SHUFFLE_EPI8(0xff, 8, 7, 6), COMPV_MM_SHUFFLE_EPI8(0xff, 11, 10, 9), // 128bits SSE register
    COMPV_MM_SHUFFLE_EPI8(0xff, 2, 1, 0), COMPV_MM_SHUFFLE_EPI8(0xff, 5, 4, 3), COMPV_MM_SHUFFLE_EPI8(0xff, 8, 7, 6), COMPV_MM_SHUFFLE_EPI8(0xff, 11, 10, 9), // 256bits SSE register
};