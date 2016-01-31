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

/*
This function contains image scaling using bilinear filter.
Some literature:
	* https://en.wikipedia.org/wiki/Linear_interpolation
	* https://en.wikipedia.org/wiki/Bilinear_filtering
	* https://en.wikipedia.org/wiki/Bilinear_interpolation
*/
#include "compv/image/scale/compv_imagescale_bilinear.h"
#include "compv/image/scale/compv_imagescale_common.h"
#include "compv/compv_math.h"

COMPV_NAMESPACE_BEGIN()

#if !defined(COMPV_IMAGESCALE_BILINEAR_SAMPLES_PER_THREAD)
#define COMPV_IMAGESCALE_BILINEAR_SAMPLES_PER_THREAD (200 * 200) // minimum number of samples to consider per thread when multi-threading
#endif

// TODO(dmi): check if fixed-point implementation doesn't introduce noise

static void scaleBilinearKernel11_C(const uint8_t* inPtr, uint8_t* outPtr, compv_scalar_t inHeight, compv_scalar_t inWidth, compv_scalar_t inStride, compv_scalar_t outHeight, compv_scalar_t outWidth, compv_scalar_t outStride, compv_scalar_t sf_x, compv_scalar_t sf_y)
{
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
    compv_scalar_t x, y, x0, y0, x1, y1, nearestX, nearestY, weight0, weight1, weight2, weight3;
    uint8_t neighb0, neighb1, neighb2, neighb3;
    const uint8_t* inPtr_;
    uint8_t* outPtr_;

    y = 0;
    for (compv_scalar_t j = 0, y = 0; j < outHeight; ++j) {
        nearestY = (y >> 8); // nearest y-point
        nearestY = COMPV_MATH_CLIP2(nearestY, inHeight - 2);
        inPtr_ = (inPtr + (nearestY * inStride));
        outPtr_ = (outPtr + (j * outStride));
        x = 0;
        for (compv_scalar_t i = 0, x = 0; i < outWidth; ++i) {
            nearestX = (x >> 8); // nearest x-point
            nearestX = COMPV_MATH_CLIP2(nearestX, inWidth - 2);
            neighb0 = inPtr_[nearestX];
            neighb1 = inPtr_[nearestX + 1];
            neighb2 = inPtr_[nearestX + inStride];
            neighb3 = inPtr_[nearestX + inStride + 1];

            x0 = (x - (nearestX << 8));
            x1 = 256 - x0;
            y0 = (y - (nearestY << 8));
            y1 = 256 - y0;

            weight0 = x1 * y1;
            weight1 = x0 * y1;
            weight2 = x1 * y0;
            weight3 = x0 * y0;

            outPtr_[i] = (uint8_t)((neighb0 * weight0 + neighb1 * weight1 + neighb2 * weight2 + neighb3 * weight3) >> 16);

            x += sf_x;
        }
        y += sf_y;
    }
}

COMPV_ERROR_CODE CompVImageScaleBilinear::process(const CompVObjWrapper<CompVImage* >& inImage, CompVObjWrapper<CompVImage* >& outImage)
{
    // This is an internal function and it's up to the caller to check the input parameters
    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
    COMPV_PIXEL_FORMAT pixelFormat = outImage->getPixelFormat(); // caller must check input and output have same format

    const uint8_t* inPtrs[4] = { NULL, NULL, NULL, NULL }, *outPtrs[4] = { NULL, NULL, NULL, NULL };
    compv_scalar_t inWidths[4], outWidths[4];
    compv_scalar_t inHeights[4], outHeights[4];
    compv_scalar_t inStrides[4], outStrides[4];
    int compSize = 1;
    scaleBilinear scale = scaleBilinearKernel11_C;

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
        COMPV_CHECK_CODE_RETURN(err_ = COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
        break;
    }

    // TODO(dmi): multi-threading

    for (int k = 0; k < compSize; ++k) {
        const float float_sx = (float)inWidths[k] / outWidths[k];
        const float float_sy = (float)inHeights[k] / outHeights[k];
        const compv_scalar_t int_sx = (compv_scalar_t)(float_sx * 256.f); // do not use "<< 8" to include the error
        const compv_scalar_t int_sy = (compv_scalar_t)(float_sy * 256.f);  // do not use "<< 8" to include the error
        // We're using fixed-point math and requiring factor between ]0-255]
        // Doesn't make sense (down/up)scaling an image >255 times its initial size. For example: (16 x 16) <-> (4080, 4080).
        // We expect image sizes to be within [16 - 4080] which means any (down/up)scaling will be ok. Off course you can (up/down)sample a 5k image if you want.
        // Most of the time scaling is used to create pyramids with scaling factor is ]0, 1[ and each time level has a scaling factor equal to (sf(n-1)<<1).
        if (float_sx <= 0.f || float_sx >= 256.f || float_sy <= 0.f || float_sy >= 256.f) {
            COMPV_DEBUG_WARN("Invalid scaling factor: (%f,%f)", float_sx, float_sy);
            // We'll have a small distortion but do not break the conversion
        }
        scale(inPtrs[k], (uint8_t*)outPtrs[k], inHeights[k], inWidths[k], inStrides[k], outHeights[k], outWidths[k], outStrides[k], int_sx, int_sy);
    }

    return err_;
}

COMPV_NAMESPACE_END()