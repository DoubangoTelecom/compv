/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/compv_image_conv_hsv.h"
#include "compv/base/image/compv_image_conv_to_rgb24.h"
#include "compv/base/image/compv_image.h"
#include "compv/base/image/compv_image_conv_common.h"
#include "compv/base/math/compv_math_utils.h"
#include "compv/base/parallel/compv_parallel.h"

COMPV_NAMESPACE_BEGIN()

static void rgb24_to_hsv_C(const uint8_t* rgb24Ptr, uint8_t* hsvPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);

//
// CompVImageConvToHSV
//

COMPV_ERROR_CODE CompVImageConvToHSV::process(const CompVMatPtr& imageIn, CompVMatPtrPtr imageHSV)
{
	// Internal function, do not check input parameters (already done)

	CompVMatPtr imageRGB24;
	COMPV_CHECK_CODE_RETURN(CompVImageConvToRGB24::process(imageIn, &imageRGB24));

	CompVMatPtr imageOut = (*imageHSV == imageIn) ? nullptr : *imageHSV;
	COMPV_CHECK_CODE_RETURN(CompVImage::newObj8u(&imageOut, COMPV_SUBTYPE_PIXELS_HSV, imageRGB24->cols(), imageRGB24->rows(), imageRGB24->stride()));

	COMPV_CHECK_CODE_RETURN(CompVImageConvToHSV::rgb24ToHsv(
		imageRGB24->ptr<const uint8_t>(),
		imageOut->ptr<uint8_t>(),
		imageOut->cols(),
		imageOut->rows(),
		imageOut->stride()
	));

	*imageHSV = imageOut;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImageConvToHSV::rgb24ToHsv(const uint8_t* rgb24Ptr, uint8_t* hsvPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	void(*rgb24_to_hsv)(const uint8_t* rgb24Ptr, uint8_t* hsvPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
		= rgb24_to_hsv_C;

	CompVThreadDispatcherPtr threadDisp = CompVParallel::threadDispatcher();
	size_t maxThreads = threadDisp ? static_cast<size_t>(threadDisp->threadsCount()) : 0;

	// Compute number of threads
	const size_t threadsCount = (threadDisp && !threadDisp->isMotherOfTheCurrentThread())
		? CompVThreadDispatcher::guessNumThreadsDividingAcrossY(width, height, maxThreads, COMPV_IMAGE_CONV_MIN_SAMPLES_PER_THREAD)
		: 1;

	if (threadsCount > 1) {
		const size_t heights = (height / threadsCount);
		const size_t lastHeight = heights + (height % threadsCount);
		const size_t paddingInBytes = (stride * sizeof(compv_uint8x3_t) * heights);
		CompVAsyncTaskIds taskIds;
		taskIds.reserve(threadsCount);
		auto funcPtr = [&](const uint8_t* rgb24Ptr_, uint8_t* hsvPtr_, compv_uscalar_t height_) -> void {
			rgb24_to_hsv(rgb24Ptr_, hsvPtr_, width, height_, stride);
		};

		for (size_t threadIdx = 0; threadIdx < threadsCount - 1; ++threadIdx) {
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtr, rgb24Ptr, hsvPtr, heights), taskIds), "Dispatching task failed");
			rgb24Ptr += paddingInBytes;
			hsvPtr += paddingInBytes;
		}
		if (lastHeight > 0) {
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtr, rgb24Ptr, hsvPtr, lastHeight), taskIds), "Dispatching task failed");
		}
		COMPV_CHECK_CODE_RETURN(threadDisp->wait(taskIds), "Failed to wait for tasks execution");
	}
	else {
		rgb24_to_hsv(rgb24Ptr, hsvPtr, width, height, stride);
	}

	return COMPV_ERROR_CODE_S_OK;
}

static void rgb24_to_hsv_C(const uint8_t* rgb24Ptr, uint8_t* hsvPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");

	size_t i, j;
	const compv_uint8x3_t* rgb24Ptr_ = reinterpret_cast<const compv_uint8x3_t*>(rgb24Ptr);
	compv_uint8x3_t* hsvPtr_ = reinterpret_cast<compv_uint8x3_t*>(hsvPtr);
	uint8_t minVal, maxVal, minus, r, g, b;
	for (j = 0; j <height; ++j) {
		for (i = 0; i < width; ++i) {
			const compv_uint8x3_t& rgb = rgb24Ptr_[i];
			compv_uint8x3_t& hsv = hsvPtr_[i];
			r = rgb[0], g = rgb[1], b = rgb[2];
			minVal = r < g ? (r < b ? r : b) : (g < b ? g : b);
			maxVal = r > g ? (r > b ? r : b) : (g > b ? g : b);
			
			if (!(hsv[2] = maxVal)) {
				hsv[0] = hsv[1] = 0;
			}
			else {
				minus = maxVal - minVal;
				if (!(hsv[1] = (255 * minus) / hsv[2])) {
					hsv[0] = 0;
				}
				else {
					if (maxVal == r) {
						hsv[0] = 0 + ((43 * (g - b)) / minus);
					}
					else if (maxVal == g) {
						hsv[0] = 85 + ((43 * (b - r)) / minus);
					}
					else {
						hsv[0] = 171 + ((43 * (r - g)) / minus);
					}
				}
			}
		}
		rgb24Ptr_ += stride;
		hsvPtr_ += stride;
	}
}

COMPV_NAMESPACE_END()
