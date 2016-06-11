/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/

/*
This function contains image scaling using bilinear filter.
Some literature:
	* https://en.wikipedia.org/wiki/Linear_interpolation
	* https://en.wikipedia.org/wiki/Bilinear_filtering
	* https://en.wikipedia.org/wiki/Bilinear_interpolation
*/
#include "compv/image/scale/compv_imagescale_bilinear.h"
#include "compv/image/scale/compv_imagescale_common.h"
#include "compv/compv_cpu.h"
#include "compv/math/compv_math.h"
#include "compv/compv_mem.h"

#include "compv/intrinsics/x86/image/scale/compv_imagescale_bilinear_intrin_sse41.h"

#if COMPV_ARCH_X86 && COMPV_ASM
COMPV_EXTERNC void ScaleBilinear_Asm_X86_SSE41(const uint8_t* inPtr, COMPV_ALIGNED(SSE) uint8_t* outPtr, compv::compv_uscalar_t inHeight, compv::compv_uscalar_t inWidth, compv::compv_uscalar_t inStride, compv::compv_uscalar_t outHeight, compv::compv_uscalar_t outWidth, COMPV_ALIGNED(SSE) compv::compv_uscalar_t outStride, compv::compv_uscalar_t sf_x, compv::compv_uscalar_t sf_y);
#endif /* COMPV_ARCH_X86 && COMPV_ASM */

COMPV_NAMESPACE_BEGIN()

#if !defined(COMPV_IMAGESCALE_BILINEAR_SAMPLES_PER_THREAD)
#define COMPV_IMAGESCALE_BILINEAR_SAMPLES_PER_THREAD (200 * 200) // minimum number of samples to consider per thread when multi-threading
#endif

static void scaleBilinearKernel11_C(const uint8_t* inPtr, uint8_t* outPtr, compv_uscalar_t inHeight, compv_uscalar_t inWidth, compv_uscalar_t inStride, compv_uscalar_t outHeight, compv_uscalar_t outWidth, compv_uscalar_t outStride, compv_uscalar_t sf_x, compv_uscalar_t sf_y)
{
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // TODO(dmi): SIMD, MT

    compv_uscalar_t i, j, x, y, nearestX, nearestY;
    unsigned neighb0, neighb1, neighb2, neighb3, x0, y0, x1, y1, S, S0, S1;
    const uint8_t* inPtr_;

    S, S0, S1;

    for (j = 0, y = 0; j < outHeight; ++j) {
        nearestY = (y >> 8); // nearest y-point
        inPtr_ = (inPtr + (nearestY * inStride));
        for (i = 0, x = 0; i < outWidth; ++i, x += sf_x) {
            nearestX = (x >> 8); // nearest x-point

            neighb0 = *(inPtr_ + nearestX);
            neighb1 = *(inPtr_ + nearestX + 1);
            neighb2 = *(inPtr_ + nearestX + inStride);
            neighb3 = *(inPtr_ + nearestX + inStride + 1);

            x0 = x & 0xff;
            y0 = y & 0xff;
            x1 = 0xff - x0;
            y1 = 0xff - y0;
#if 1
            // weight0 = x1 * y1;
            // weight1 = x0 * y1;
            // weight2 = x1 * y0;
            // weight3 = x0 * y0;
            // S = neighb0 * weight0 + neighb1 * weight1 + neighb2 * weight2 + neighb3 * weight3
            // S = neighb0 * (x1 * y1) + neighb1 * (x0 * y1) + neighb2 * (x1 * y0) + neighb3 * (x0 * y0)
            // S = y1 * ((neighb0 * x1) + (neighb1 * x0)) + y0 * ((neighb2 * x1 ) + (neighb3 * x0))
            outPtr[i] = (uint8_t)((y1 * ((neighb0 * x1) + (neighb1 * x0)) + y0 * ((neighb2 * x1) + (neighb3 * x0))) >> 16);
#else
            // x0 = x & 0xff;
            // y0 = y & 0xff;
            // x1 = 0xff - x0;
            // y1 = 0xff - y0;
            // weight0 = x1 * y1;
            // weight1 = x0 * y1;
            // weight2 = x1 * y0;
            // weight3 = x0 * y0;
            // S = neighb0 * weight0 + neighb1 * weight1 + neighb2 * weight2 + neighb3 * weight3
            // S = neighb0 * (x1 * y1) + neighb1 * (x0 * y1) + neighb2 * (x1 * y0) + neighb3 * (x0 * y0)
            // S = y1 * ((neighb0 * x1) + (neighb1 * x0)) + y0 * ((neighb2 * x1) + (neighb3 * x0))
            // S = y1 * ((neighb0 * (0xff - x0)) + (neighb1 * x0)) + y0 * ((neighb2 * (0xff - x0)) + (neighb3 * x0))
            // S = y1 * (neighb0 * 0xff - neighb0 * x0 + neighb1 * x0) + y0 * (neighb2 * 0xff - neighb2 * x0 + neighb3 * x0)
            // S = y1 * (neighb0 * 0xff - (neighb0 + neighb1) * x0) + y0 * (neighb2 * 0xff - (neighb2 + neighb3) * x0)
            // S = y1 * ((neighb0 << 8) - neighb0 - x0 * (neighb0 - neighb1)) + y0 * ((neighb2 << 8) - neighb2 - x0 * (neighb2 - neighb3)
            // S = (0xff - y0) * S0 + y0 * S1
            // S = S0 * 0xff - y0 * S0 + y0 * S1
            // S = (S0 << 8) - S0 - y0 * (S0 - S1)
            S0 = ((neighb0 << 8) - neighb0 - x0 * (neighb0 - neighb1));
            S1 = ((neighb2 << 8) - neighb2 - x0 * (neighb2 - neighb3));
            S = (S0 << 8) - S0 - y0 * (S0 - S1);
            outPtr[i] = (uint8_t)(S >> 16);
#endif
        }

        y += sf_y;
        outPtr += outStride;
    }
}

// Function not used
static COMPV_ERROR_CODE scaleBilinearGrayscale(const uint8_t* inPtr, uint8_t* outPtr, compv_uscalar_t inHeight, compv_uscalar_t inWidth, compv_uscalar_t inStride, compv_uscalar_t outHeight, compv_uscalar_t outWidth, compv_uscalar_t outStride, compv_uscalar_t sf_x, compv_uscalar_t sf_y)
{
    COMPV_DEBUG_INFO_CODE_FOR_TESTING();
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // TODO(dmi): SIMD, MT
    size_t nearestYCount = CompVMem::alignForward(outHeight * sizeof(int32_t), COMPV_SIMD_ALIGNV_DEFAULT);
    size_t nearestXCount = CompVMem::alignForward(outWidth * sizeof(int32_t), COMPV_SIMD_ALIGNV_DEFAULT);
    size_t yCount = CompVMem::alignForward(outHeight * sizeof(int32_t), COMPV_SIMD_ALIGNV_DEFAULT);
    size_t xCount = CompVMem::alignForward(outWidth * sizeof(int32_t), COMPV_SIMD_ALIGNV_DEFAULT);
    int8_t *mem = (int8_t *)CompVMem::malloc(nearestYCount + nearestXCount + (yCount << 1) + (xCount << 1)); // For (2k x 1k) image we'll alloc 24k memory -> the speedup worth it
    COMPV_CHECK_EXP_RETURN(!mem, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

    int32_t *nearestY = (int32_t*)mem;
    int32_t *nearestX = (int32_t*)(mem + nearestYCount);
    int32_t *x0 = (int32_t*)(((uint8_t*)nearestX) + nearestXCount);
    int32_t *x1 = (int32_t*)(((uint8_t*)x0) + xCount);
    int32_t *y0 = (int32_t*)(((uint8_t*)x1) + xCount);
    int32_t *y1 = (int32_t*)(((uint8_t*)y0) + yCount);

#if 1
    // No need for outPtr to be aligned
    COMPV_DEBUG_INFO_CODE_FOR_TESTING();
    ScaleBilinearGrayscale_Intrin_SSE41(inPtr, outPtr, inStride, outHeight, outWidth, outStride, sf_x, sf_y, nearestX, nearestY, x0, y0, x1, y1);
#else

    compv_uscalar_t i, j;
    int32_t x, y, sf_y_ = (int32_t)sf_y, sf_x_ = (int32_t)sf_x, inStride_ = (int32_t)inStride;
    uint8_t neighb0, neighb1, neighb2, neighb3;
    const uint8_t *inPtr_;
    const int32_t *nearestX_, *x0_, *x1_;

    for (j = 0, y = 0; j < outHeight; ++j) {
        nearestY[j] = (y >> 8) * inStride_;
        y0[j] = y & 0xff;
        y1[j] = 0xff - y0[j];
        y += sf_y_;
    }
    for (i = 0, x = 0; i < outWidth; ++i) {
        nearestX[i] = (x >> 8);
        x0[i] = x & 0xff;
        x1[i] = 0xff - x0[i];
        x += sf_x_;
    }
    for (j = 0; j < outHeight; ++j, ++y0, ++y1, ++nearestY) {
        inPtr_ = (inPtr + *nearestY);
        for (i = 0, nearestX_ = nearestX, x0_ = x0, x1_ = x1; i < outWidth; ++i, ++nearestX_, ++x0_, ++x1_) {
            neighb0 = *(inPtr_ + *nearestX_);
            neighb1 = *(inPtr_ + *nearestX_ + 1);
            neighb2 = *(inPtr_ + *nearestX_ + inStride);
            neighb3 = *(inPtr_ + *nearestX_ + inStride + 1);

            // weight0 = x1 * y1;
            // weight1 = x0 * y1;
            // weight2 = x1 * y0;
            // weight3 = x0 * y0;
            // S = neighb0 * weight0 + neighb1 * weight1 + neighb2 * weight2 + neighb3 * weight3
            // S = neighb0 * (x1 * y1) + neighb1 * (x0 * y1) + neighb2 * (x1 * y0) + neighb3 * (x0 * y0)
            // S = y1 * ((neighb0 * x1) + (neighb1 * x0)) + y0 * ((neighb2 * x1 ) + (neighb3 * x0))
            outPtr[i] = (uint8_t)((*y1 * ((neighb0 **x1_) + (neighb1 **x0_)) + *y0 * ((neighb2 **x1_) + (neighb3 **x0_))) >> 16);
        }
        outPtr += outStride;
    }
#endif

    CompVMem::free((void**)&mem);
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImageScaleBilinear::process(const CompVPtr<CompVImage* >& inImage, CompVPtr<CompVImage* >& outImage)
{
    // This is an internal function and it's up to the caller to check the input parameters
    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
    COMPV_PIXEL_FORMAT pixelFormat = outImage->getPixelFormat(); // caller must check input and output have same format

    const uint8_t* inPtrs[4] = { NULL, NULL, NULL, NULL }, *outPtrs[4] = { NULL, NULL, NULL, NULL };
    compv_uscalar_t inWidths[4], outWidths[4];
    compv_uscalar_t inHeights[4], outHeights[4];
    compv_uscalar_t inStrides[4], outStrides[4];
    int compSize = 1, alignv = 1;
    scaleBilinear scale_SIMD = NULL;

#if 0 // C++ code is faster
    COMPV_DEBUG_INFO_CODE_FOR_TESTING();
    if (CompVCpu::isEnabled(kCpuFlagSSE41)) {
        COMPV_EXEC_IFDEF_INTRIN_X86((scale_SIMD = ScaleBilinear_Intrin_SSE41, alignv = COMPV_SIMD_ALIGNV_SSE));
        COMPV_EXEC_IFDEF_ASM_X86((scale_SIMD = ScaleBilinear_Asm_X86_SSE41, alignv = COMPV_SIMD_ALIGNV_SSE));
    }
#endif

    switch (pixelFormat) {
    case COMPV_PIXEL_FORMAT_GRAYSCALE:
        inPtrs[0] = (const uint8_t*)inImage->getDataPtr();
        inWidths[0] = inImage->getWidth();
        inHeights[0] = inImage->getHeight();
        inStrides[0] = inImage->getStride();

        outPtrs[0] = (const uint8_t*)outImage->getDataPtr();
        outWidths[0] = outImage->getWidth();
        outHeights[0] = outImage->getHeight();
        outStrides[0] = outImage->getStride();

        compSize = 1;
        break;
    case COMPV_PIXEL_FORMAT_I420:
        COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // The scale() function is optimized for grayscale only
        inPtrs[0] = (const uint8_t*)inImage->getDataPtr();
        inWidths[0] = inImage->getWidth();
        inHeights[0] = inImage->getHeight();
        inStrides[0] = inImage->getStride();
        inPtrs[1] = inPtrs[0] + (inHeights[0] * inStrides[0]);
        inWidths[1] = inWidths[0] >> 1;
        inHeights[1] = inHeights[0] >> 1;
        inStrides[1] = inStrides[0] >> 1;
        inPtrs[2] = inPtrs[1] + (inHeights[1] * inStrides[1]);
        inWidths[2] = inWidths[1];
        inHeights[2] = inHeights[1];
        inStrides[2] = inStrides[1];

        outPtrs[0] = (const uint8_t*)outImage->getDataPtr();
        outWidths[0] = outImage->getWidth();
        outHeights[0] = outImage->getHeight();
        outStrides[0] = outImage->getStride();
        outPtrs[1] = outPtrs[0] + (outHeights[0] * outStrides[0]);
        outWidths[1] = outWidths[0] >> 1;
        outHeights[1] = outHeights[0] >> 1;
        outStrides[1] = outStrides[0] >> 1;
        outPtrs[2] = outPtrs[1] + (outHeights[1] * outStrides[1]);
        outWidths[2] = outWidths[1];
        outHeights[2] = outHeights[1];
        outStrides[2] = outStrides[1];

        compSize = 3;
        break;
    default:
        COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
        COMPV_CHECK_CODE_RETURN(err_ = COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
        break;
    }

    // TODO(dmi): multi-threading

    for (int k = 0; k < compSize; ++k) {
        const float float_sx = (float)inWidths[k] / outWidths[k];
        const float float_sy = (float)inHeights[k] / outHeights[k];
        const compv_scalar_t int_sx = (compv_scalar_t)(float_sx * 255.f); // do not use "<< 8" to include the error
        const compv_scalar_t int_sy = (compv_scalar_t)(float_sy * 255.f);  // do not use "<< 8" to include the error
        // We're using fixed-point math and requiring factor between ]0-255]
        // Doesn't make sense (down/up)scaling an image >255 times its initial size. For example: (16 x 16) <-> (4080, 4080).
        // We expect image sizes to be within [16 - 4080] which means any (down/up)scaling will be ok. Off course you can (up/down)sample a 5k image if you want.
        // Most of the time scaling is used to create pyramids with scaling factor is ]0, 1[ and each time level has a scaling factor equal to (sf(n-1)<<1).
        if (float_sx <= 0.f || float_sx >= 255.f || float_sy <= 0.f || float_sy >= 255.f) {
            COMPV_DEBUG_WARN("Invalid scaling factor: (%f,%f)", float_sx, float_sy);
            // We'll have a small distortion but do not break the conversion
        }
        if (scale_SIMD && COMPV_IS_ALIGNED(outPtrs[k], alignv) && COMPV_IS_ALIGNED(outStrides[k], alignv)) {
            scale_SIMD(inPtrs[k], (uint8_t*)outPtrs[k], inHeights[k], inWidths[k], inStrides[k], outHeights[k], outWidths[k], outStrides[k], int_sx, int_sy);
        }
        else {
            scaleBilinearKernel11_C(inPtrs[k], (uint8_t*)outPtrs[k], inHeights[k], inWidths[k], inStrides[k], outHeights[k], outWidths[k], outStrides[k], int_sx, int_sy);
        }
    }

    return err_;
}

COMPV_NAMESPACE_END()