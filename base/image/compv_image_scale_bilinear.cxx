/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/compv_image_scale_bilinear.h"

#include "compv/base/image/intrin/x86/compv_image_scale_bilinear_intrin_sse2.h"

#define COMPV_THIS_CLASSNAME	"CompVImageScaleBilinear"

COMPV_NAMESPACE_BEGIN()

static void scaleBilinearKernel11_C(const uint8_t* inPtr, compv_uscalar_t inWidth, compv_uscalar_t inHeight, compv_uscalar_t inStride, uint8_t* outPtr, compv_uscalar_t outWidth, compv_uscalar_t outHeight, compv_uscalar_t outStride, compv_uscalar_t sf_x, compv_uscalar_t sf_y)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD implementation found");
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No MT implementation found");

	if (1) {
		COMPV_DEBUG_INFO_CODE_FOR_TESTING("FIXME");
		CompVImageScaleBilinear_Intrin_SSE2(
			inPtr, inWidth, inHeight, inStride,
			outPtr, outWidth, outHeight, outStride,
			sf_x, sf_y);
		return;
	}

	compv_uscalar_t i, j, x, y, nearestX, nearestY;
	unsigned neighb0, neighb1, neighb2, neighb3, x0, y0, x1, y1;
	const uint8_t* inPtr_;

	for (j = 0, y = 0; j < outHeight; ++j) {
		nearestY = (y >> 8); // nearest y-point
		inPtr_ = (inPtr + (nearestY * inStride));
		for (i = 0, x = 0; i < outWidth; ++i, x += sf_x) {
			nearestX = (x >> 8); // nearest x-point (compute for each row but this is faster than storing the values then reading from mem)

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

COMPV_ERROR_CODE CompVImageScaleBilinear::process(const CompVMatPtr& imageIn, CompVMatPtr& imageOut)
{
	// Internal function, no need to check for input parameters
	float float_sx, float_sy;
	compv_uscalar_t int_sx, int_sy, strideIn, strideOut, widthIn, widthOut, heightIn, heightOut;
	for (int planeId = 0; planeId < static_cast<int>(imageOut->planeCount()); ++planeId) {
		strideIn = imageIn->stride(planeId);
		widthIn = imageIn->cols(planeId);
		heightIn = imageIn->rows(planeId);
		strideOut = imageOut->stride(planeId);
		widthOut = imageOut->cols(planeId);
		heightOut = imageOut->rows(planeId);
		float_sx = static_cast<float>(widthIn) / widthOut;
		float_sy = static_cast<float>(heightIn) / heightOut;
		int_sx = static_cast<compv_scalar_t>(float_sx * 255.f); // do not use "<< 8" to include the error
		int_sy = static_cast<compv_scalar_t>(float_sy * 255.f);  // do not use "<< 8" to include the error
		// We're using fixed-point math and requiring factor between ]0-255]
		// Doesn't make sense (down/up)scaling an image >255 times its initial size. For example: (16 x 16) <-> (4080, 4080).
		// We expect image sizes to be within [16 - 4080] which means any (down/up)scaling will be ok. Off course you can (up/down)sample a 5k image if you want.
		// Most of the time scaling is used to create pyramids with scaling factor is ]0, 1[ and each time level has a scaling factor equal to (sf(n-1)<<1).
		if (float_sx <= 0.f || float_sx >= 255.f || float_sy <= 0.f || float_sy >= 255.f) {
			COMPV_DEBUG_WARN_EX(COMPV_THIS_CLASSNAME, "Invalid scaling factor: (%f, %f)", float_sx, float_sy);
			// We'll have a small distortion but do not break the conversion
		}
#if 0
		if (scale_SIMD && COMPV_IS_ALIGNED(outPtrs[k], alignv) && COMPV_IS_ALIGNED(outStrides[k], alignv)) {
			scale_SIMD(inPtrs[k], (uint8_t*)outPtrs[k], inHeights[k], inWidths[k], inStrides[k], outHeights[k], outWidths[k], outStrides[k], int_sx, int_sy);
		}
		else
#endif
		{
			scaleBilinearKernel11_C(
				imageIn->ptr<const uint8_t>(0, 0, planeId), widthIn, heightIn, strideIn,
				imageOut->ptr<uint8_t>(0, 0, planeId), widthOut, heightOut, strideOut,
				int_sx, int_sy);
		}
	}

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
