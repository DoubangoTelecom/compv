/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/compv_image.h"
#include "compv/base/image/compv_image_conv_to_yuv444p.h"
#include "compv/base/image/compv_image_conv_rgbfamily.h"
#include "compv/base/image/compv_image_conv_common.h"
#include "compv/base/parallel/compv_parallel.h"
#include "compv/base/math/compv_math.h"

#define COMPV_THIS_CLASSNAME	"CompVImageConvToYUV444P"

COMPV_NAMESPACE_BEGIN()

COMPV_ERROR_CODE CompVImageConvToYUV444P::process(const CompVMatPtr& imageIn, CompVMatPtrPtr imageYUV444P)
{
	// Internal function, do not check input parameters (already done)
	switch (imageIn->subType()) {
	case COMPV_SUBTYPE_PIXELS_RGB24:
	case COMPV_SUBTYPE_PIXELS_BGR24:
		COMPV_CHECK_CODE_RETURN(CompVImageConvToYUV444P::rgb24family(imageIn, imageYUV444P), "Conversion (RGB24 -> YUV444P) failed");
		return COMPV_ERROR_CODE_S_OK;
	default:
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Chroma conversion not supported: %s -> COMPV_SUBTYPE_PIXELS_YUV444P", CompVGetSubtypeString(imageIn->subType()));
		return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
	}
}

COMPV_ERROR_CODE CompVImageConvToYUV444P::rgb24family(const CompVMatPtr& imageRGB24family, CompVMatPtrPtr imageYUV444P)
{
	// Private function, do not check input parameters (already done)
	void(*rgb24family_to_y)(const uint8_t* rgbPtr, uint8_t* outYPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
		= (imageRGB24family->subType() == COMPV_SUBTYPE_PIXELS_RGB24) ? &CompVImageConvRGBfamily::rgb24_to_y : &CompVImageConvRGBfamily::bgr24_to_y;
	void (*rgb24_to_uv_planar_11)(const uint8_t* rgbPtr, uint8_t* outUPtr, uint8_t* outVPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
		= (imageRGB24family->subType() == COMPV_SUBTYPE_PIXELS_RGB24) ? &CompVImageConvRGBfamily::rgb24_to_uv_planar_11 : &CompVImageConvRGBfamily::bgr24_to_uv_planar_11;
	const size_t width = imageRGB24family->cols();
	const size_t height = imageRGB24family->rows();
	const size_t stride = imageRGB24family->stride(); // in and out images must have same stride
	size_t strideY, strideUV;
	const uint8_t* rgbPtr = imageRGB24family->ptr<const uint8_t>();
	uint8_t *yPtr, *uPtr, *vPtr;
	size_t threadsCount;
	CompVAsyncTaskIds taskIds;
	CompVThreadDispatcherPtr threadDisp = CompVParallel::getThreadDispatcher();
	size_t maxThreads = threadDisp ? static_cast<size_t>(threadDisp->threadsCount()) : 0;
	COMPV_ERROR_CODE err;

	// Create the output YUV444P image (nop operation if image already allocated)
	COMPV_CHECK_CODE_BAIL(err = CompVImage::newObj8u(imageYUV444P, COMPV_SUBTYPE_PIXELS_YUV444P, width, height, stride));
	yPtr = (*imageYUV444P)->ptr<uint8_t>(0, 0, COMPV_PLANE_Y);
	uPtr = (*imageYUV444P)->ptr<uint8_t>(0, 0, COMPV_PLANE_U);
	vPtr = (*imageYUV444P)->ptr<uint8_t>(0, 0, COMPV_PLANE_V);
	strideY = (*imageYUV444P)->stride(COMPV_PLANE_Y); // same as value 'stride'
	strideUV = (*imageYUV444P)->stride(COMPV_PLANE_UV); // same as value 'stride' and 'strideY'

	// Compute number of threads
	threadsCount = (threadDisp && !threadDisp->isMotherOfTheCurrentThread())
		? threadDisp->guessNumThreadsDividingAcrossY(stride, height, maxThreads, COMPV_IMAGE_CONV_MIN_SAMPLES_PER_THREAD)
		: 1;

	if (threadsCount > 1) {
		size_t rgbIdx = 0, YIdx = 0, UVIdx = 0;
		taskIds.reserve(threadsCount);
		auto funcPtr = [&](const uint8_t* rgbPtr, uint8_t* outYPtr, uint8_t* outUPtr, uint8_t* outVPtr, compv_uscalar_t height) -> COMPV_ERROR_CODE {
			rgb24family_to_y(rgbPtr, outYPtr, width, height, strideY);
			rgb24_to_uv_planar_11(rgbPtr, outUPtr, outVPtr, width, height, strideUV);
			return COMPV_ERROR_CODE_S_OK;
		};
		size_t heights = (height / threadsCount);
		size_t lastHeight = height - ((threadsCount - 1) * heights);
		for (size_t threadIdx = 0; threadIdx < threadsCount; ++threadIdx) {
			COMPV_CHECK_CODE_ASSERT(err = threadDisp->invoke(std::bind(funcPtr, (rgbPtr + rgbIdx), (yPtr + YIdx), (uPtr + UVIdx), (vPtr + UVIdx), static_cast<compv_uscalar_t>((threadIdx == (threadsCount - 1) ? lastHeight : heights))), taskIds), "Dispatching task failed");
			rgbIdx += (heights * stride) * 3;
			YIdx += (heights * strideY);
			UVIdx += (heights * strideUV);
		}
		COMPV_CHECK_CODE_BAIL(err = threadDisp->wait(taskIds), "Failed to wait for tasks execution");
	}
	else {
		rgb24family_to_y(rgbPtr, yPtr, width, height, stride);
		rgb24_to_uv_planar_11(rgbPtr, uPtr, vPtr, width, height, stride);
	}

bail:
	return err;
}

COMPV_NAMESPACE_END()
