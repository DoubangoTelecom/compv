/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/compv_image_scale_bilinear.h"
#include "compv/base/compv_cpu.h"
#include "compv/base/parallel/compv_parallel.h"

#include "compv/base/image/intrin/x86/compv_image_scale_bilinear_intrin_sse2.h"

#define COMPV_THIS_CLASSNAME	"CompVImageScaleBilinear"

#define COMPV_IMAGE_SCALE_SAMPLES_PER_THREAD (100 * 100)

COMPV_NAMESPACE_BEGIN()

static void scaleBilinear_C(const uint8_t* inPtr, compv_uscalar_t inWidth, compv_uscalar_t inHeight, compv_uscalar_t inStride, uint8_t* outPtr, compv_uscalar_t outWidth, compv_uscalar_t outHeight, compv_uscalar_t outYStart, compv_uscalar_t outStride, compv_uscalar_t sf_x, compv_uscalar_t sf_y)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
	compv_uscalar_t i, x, y, ymax, nearestX, nearestY;
	unsigned int neighb0, neighb1, neighb2, neighb3, x0, y0, x1, y1;
	const uint8_t* inPtr_;

	outPtr += (outYStart * outStride);
	y = (sf_y * outYStart);
	ymax = y + (outHeight * sf_y);

	while (y < ymax) {
		nearestY = (y >> 8); // nearest y-point
		inPtr_ = (inPtr + (nearestY * inStride));
		for (i = 0, x = 0; i < outWidth; ++i, x += sf_x) {
			nearestX = (x >> 8); // nearest x-point

			neighb0 = inPtr_[nearestX];
			neighb1 = inPtr_[nearestX + 1];
			neighb2 = inPtr_[nearestX + inStride];
			neighb3 = inPtr_[nearestX + inStride + 1];

			x0 = x & 0xff;
			y0 = y & 0xff;
			x1 = 0xff - x0;
			y1 = 0xff - y0;

			outPtr[i] = static_cast<uint8_t>((y1 * ((neighb0 * x1) + (neighb1 * x0)) + y0 * ((neighb2 * x1) + (neighb3 * x0))) >> 16); // no need for saturation after >> 16
		}
		outPtr += outStride;
		y += sf_y;
	}
}

static COMPV_ERROR_CODE scaleBilinear(const uint8_t* inPtr, compv_uscalar_t inWidth, compv_uscalar_t inHeight, compv_uscalar_t inStride, uint8_t* outPtr, compv_uscalar_t outWidth, compv_uscalar_t outHeight, compv_uscalar_t outStride, compv_uscalar_t sf_x, compv_uscalar_t sf_y)
{
	void(*scale)(const uint8_t* inPtr, compv_uscalar_t inWidth, compv_uscalar_t inHeight, compv_uscalar_t inStride, uint8_t* outPtr, compv_uscalar_t outWidth, compv_uscalar_t outHeight, compv_uscalar_t outYStart, compv_uscalar_t outStride, compv_uscalar_t sf_x, compv_uscalar_t sf_y)
		= scaleBilinear_C;

	size_t threadsCount;
	CompVAsyncTaskIds taskIds;
	CompVThreadDispatcherPtr threadDisp = CompVParallel::threadDispatcher();
	size_t maxThreads = threadDisp ? static_cast<size_t>(threadDisp->threadsCount()) : 0;

	// Compute number of threads
	threadsCount = (threadDisp && !threadDisp->isMotherOfTheCurrentThread())
		? CompVThreadDispatcher::guessNumThreadsDividingAcrossY(outStride, outHeight, maxThreads, COMPV_IMAGE_SCALE_SAMPLES_PER_THREAD)
		: 1;

#if COMPV_ARCH_X86
	if (CompVCpu::isEnabled(kCpuFlagSSE2) && !(outStride & 0x1)) {
		// COMPV_EXEC_IFDEF_INTRIN_X86((funcptr = CompVImageScaleBilinear_Intrin_SSE2));
	}
#elif COMPV_ARCH_ARM
	if (CompVCpu::isEnabled(kCpuFlagARM_NEON)) {
	}
#endif

	if (threadsCount > 1) {
		size_t YStart = 0;
		taskIds.reserve(threadsCount);
		auto funcPtr = [&](size_t ystart, size_t height) -> void {
			scale(inPtr, inWidth, inHeight, inStride, outPtr, outWidth, height, ystart, outStride, sf_x, sf_y);
		};
		size_t heights = (outHeight / threadsCount);
		size_t lastHeight = outHeight - ((threadsCount - 1) * heights);
		for (size_t threadIdx = 0; threadIdx < threadsCount; ++threadIdx) {
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtr, YStart, (threadIdx == (threadsCount - 1) ? lastHeight : heights)), taskIds), "Dispatching task failed");
			YStart += (threadIdx == (threadsCount - 1) ? lastHeight : heights);
		}
		COMPV_CHECK_CODE_RETURN(threadDisp->wait(taskIds), "Failed to wait for tasks execution");
	}
	else {
		scale(inPtr, inWidth, inHeight, inStride, outPtr, outWidth, outHeight, 0, outStride, sf_x, sf_y);
	}

	return COMPV_ERROR_CODE_S_OK;
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
		COMPV_CHECK_CODE_RETURN(scaleBilinear(
			imageIn->ptr<const uint8_t>(0, 0, planeId), widthIn, heightIn, strideIn, 
			imageOut->ptr<uint8_t>(0, 0, planeId), widthOut, heightOut, strideOut, 
			int_sx, int_sy
		));
	}

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
