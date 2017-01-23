/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/compv_image_conv_to_yuv444p.h"
#include "compv/base/image/compv_image.h"
#include "compv/base/image/compv_image_conv_rgbfamily.h"
#include "compv/base/image/compv_image_conv_common.h"
#include "compv/base/parallel/compv_parallel.h"
#include "compv/base/math/compv_math.h"

#if COMPV_HAVE_INTEL_IPP
#	include <ipp.h>
#endif

#define COMPV_THIS_CLASSNAME	"CompVImageConvToYUV444P"

COMPV_NAMESPACE_BEGIN()

COMPV_ERROR_CODE CompVImageConvToYUV444P::process(const CompVMatPtr& imageIn, CompVMatPtrPtr imageYUV444P)
{
	CompVMatPtr imageOut = (imageIn == *imageYUV444P) ? nullptr : *imageYUV444P; // Input must not be equal to output
	// Internal function, do not check input parameters (already done)
	switch (imageIn->subType()) {
	case COMPV_SUBTYPE_PIXELS_RGBA32:
	case COMPV_SUBTYPE_PIXELS_ARGB32:
	case COMPV_SUBTYPE_PIXELS_BGRA32:
	case COMPV_SUBTYPE_PIXELS_RGB24:
	case COMPV_SUBTYPE_PIXELS_BGR24:
	case COMPV_SUBTYPE_PIXELS_RGB565LE:
	case COMPV_SUBTYPE_PIXELS_RGB565BE:
	case COMPV_SUBTYPE_PIXELS_BGR565LE:
	case COMPV_SUBTYPE_PIXELS_BGR565BE:
		COMPV_CHECK_CODE_RETURN(CompVImageConvToYUV444P::rgbfamily(imageIn, &imageOut), "Conversion (RGBFamily -> YUV444P) failed");
		break;
	default:
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Chroma conversion not supported: %s -> COMPV_SUBTYPE_PIXELS_YUV444P", CompVGetSubtypeString(imageIn->subType()));
		return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
	}
	*imageYUV444P = imageOut;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImageConvToYUV444P::rgbfamily(const CompVMatPtr& imageRGBfamily, CompVMatPtrPtr imageYUV444P)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Before converting to Y or UV we unpack RGBfamily samples to RGBAfamily and this is done"
		"twice, once for luma and once for chroma. Performance issues (not cache-friendly). Sad !!");
	// Private function, do not check input parameters (already done)
	void(*rgbfamily_to_y)(const uint8_t* rgbPtr, uint8_t* outYPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride) = NULL;
	void (*rgbfamily_to_uv_planar_11)(const uint8_t* rgbPtr, uint8_t* outUPtr, uint8_t* outVPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride) = NULL;
	size_t bytesPerPixel;
	switch (imageRGBfamily->subType()) {
		case COMPV_SUBTYPE_PIXELS_RGBA32:
			rgbfamily_to_y = CompVImageConvRGBfamily::rgba32_to_y;
			rgbfamily_to_uv_planar_11 = CompVImageConvRGBfamily::rgba32_to_uv_planar_11;
			break;
		case COMPV_SUBTYPE_PIXELS_ARGB32:
			rgbfamily_to_y = CompVImageConvRGBfamily::argb32_to_y;
			rgbfamily_to_uv_planar_11 = CompVImageConvRGBfamily::argb32_to_uv_planar_11;
			break;
		case COMPV_SUBTYPE_PIXELS_BGRA32:
			rgbfamily_to_y = CompVImageConvRGBfamily::bgra32_to_y;
			rgbfamily_to_uv_planar_11 = CompVImageConvRGBfamily::bgra32_to_uv_planar_11;
			break;
		case COMPV_SUBTYPE_PIXELS_RGB24: 
			rgbfamily_to_y = CompVImageConvRGBfamily::rgb24_to_y; 
			rgbfamily_to_uv_planar_11 = CompVImageConvRGBfamily::rgb24_to_uv_planar_11;
			break;
		case COMPV_SUBTYPE_PIXELS_BGR24:
			rgbfamily_to_y = CompVImageConvRGBfamily::bgr24_to_y;
			rgbfamily_to_uv_planar_11 = CompVImageConvRGBfamily::bgr24_to_uv_planar_11;
			break;
		case COMPV_SUBTYPE_PIXELS_RGB565LE:
			rgbfamily_to_y = CompVImageConvRGBfamily::rgb565le_to_y;
			rgbfamily_to_uv_planar_11 = CompVImageConvRGBfamily::rgb565le_to_uv_planar_11;
			break;
		case COMPV_SUBTYPE_PIXELS_RGB565BE:
			rgbfamily_to_y = CompVImageConvRGBfamily::rgb565be_to_y;
			rgbfamily_to_uv_planar_11 = CompVImageConvRGBfamily::rgb565be_to_uv_planar_11;
			break;
		case COMPV_SUBTYPE_PIXELS_BGR565LE:
			rgbfamily_to_y = CompVImageConvRGBfamily::bgr565le_to_y;
			rgbfamily_to_uv_planar_11 = CompVImageConvRGBfamily::bgr565le_to_uv_planar_11;
			break;
		case COMPV_SUBTYPE_PIXELS_BGR565BE:
			rgbfamily_to_y = CompVImageConvRGBfamily::bgr565be_to_y;
			rgbfamily_to_uv_planar_11 = CompVImageConvRGBfamily::bgr565be_to_uv_planar_11;
			break;
		default:
			COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Failed to find RGBfamily conversion: %s -> COMPV_SUBTYPE_PIXELS_YUV444P", CompVGetSubtypeString(imageRGBfamily->subType()));
			return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
	}
	COMPV_CHECK_CODE_RETURN(CompVImageUtils::bitsCountForPixelFormat(imageRGBfamily->subType(), &bytesPerPixel));
	bytesPerPixel >>= 3;

	const size_t width = imageRGBfamily->cols();
	const size_t height = imageRGBfamily->rows();
	const size_t stride = imageRGBfamily->stride(); // in and out images must have same stride
	size_t strideY, strideUV;
	const uint8_t* rgbPtr = imageRGBfamily->ptr<const uint8_t>();
	uint8_t *yPtr, *uPtr, *vPtr;
	size_t threadsCount;
	CompVAsyncTaskIds taskIds;
	CompVThreadDispatcherPtr threadDisp = CompVParallel::threadDispatcher();
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
		? CompVThreadDispatcher::guessNumThreadsDividingAcrossY(stride, height, maxThreads, COMPV_IMAGE_CONV_MIN_SAMPLES_PER_THREAD)
		: 1;

	if (threadsCount > 1) {
		size_t rgbIdx = 0, YIdx = 0, UVIdx = 0;
		taskIds.reserve(threadsCount);
		auto funcPtr = [&](const uint8_t* rgbPtr, uint8_t* outYPtr, uint8_t* outUPtr, uint8_t* outVPtr, compv_uscalar_t height) -> void {
			rgbfamily_to_y(rgbPtr, outYPtr, width, height, strideY);
			rgbfamily_to_uv_planar_11(rgbPtr, outUPtr, outVPtr, width, height, strideUV);
		};
		size_t heights = (height / threadsCount);
		size_t lastHeight = height - ((threadsCount - 1) * heights);
		for (size_t threadIdx = 0; threadIdx < threadsCount; ++threadIdx) {
			COMPV_CHECK_CODE_ASSERT(err = threadDisp->invoke(std::bind(funcPtr, (rgbPtr + rgbIdx), (yPtr + YIdx), (uPtr + UVIdx), (vPtr + UVIdx), static_cast<compv_uscalar_t>((threadIdx == (threadsCount - 1) ? lastHeight : heights))), taskIds), "Dispatching task failed");
			rgbIdx += (heights * stride) * bytesPerPixel;
			YIdx += (heights * strideY);
			UVIdx += (heights * strideUV);
		}
		COMPV_CHECK_CODE_BAIL(err = threadDisp->wait(taskIds), "Failed to wait for tasks execution");
	}
	else {
		rgbfamily_to_y(rgbPtr, yPtr, width, height, stride);
		rgbfamily_to_uv_planar_11(rgbPtr, uPtr, vPtr, width, height, stride);
	}

bail:
	return err;
}

COMPV_NAMESPACE_END()
