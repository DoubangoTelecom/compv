/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/compv_image_conv_to_rgb24.h"
#include "compv/base/image/compv_image.h"
#include "compv/base/image/compv_image_conv_common.h"
#include "compv/base/math/compv_math_utils.h"
#include "compv/base/parallel/compv_parallel.h"

COMPV_NAMESPACE_BEGIN()

static void yuv420p_to_rgb24_C(const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
static void yuv422p_to_rgb24_C(const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
static void yuv444p_to_rgb24_C(const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
static void nv12_to_rgb24_C(const uint8_t* yPtr, const uint8_t* uvPtr, uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
static void nv21_to_rgb24_C(const uint8_t* yPtr, const uint8_t* uvPtr, uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
static void yuyv422_to_rgb24_C(const uint8_t* yuyvPtr, uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
static void uyvy422_to_rgb24_C(const uint8_t* yuyvPtr, uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);

COMPV_ERROR_CODE CompVImageConvToRGB24::process(const CompVMatPtr& imageIn, CompVMatPtrPtr imageRGB24)
{
	// Internal function, do not check input parameters (already done)

	CompVMatPtr imageOut = (*imageRGB24 == imageIn) ? nullptr : *imageRGB24;
	COMPV_CHECK_CODE_RETURN(CompVImage::newObj8u(&imageOut, COMPV_SUBTYPE_PIXELS_RGB24, imageIn->cols(), imageIn->rows(), imageIn->stride()));

	switch (imageIn->subType()) {
		case COMPV_SUBTYPE_PIXELS_RGB24:
			COMPV_CHECK_CODE_RETURN(CompVImage::clone(imageIn, &imageOut));
			break;

		case COMPV_SUBTYPE_PIXELS_YUV420P:
		case COMPV_SUBTYPE_PIXELS_YUV422P:
		case COMPV_SUBTYPE_PIXELS_YUV444P:
			COMPV_CHECK_CODE_RETURN(CompVImageConvToRGB24::yuvPlanar(imageIn, imageOut));
			break;

		case COMPV_SUBTYPE_PIXELS_NV12:
		case COMPV_SUBTYPE_PIXELS_NV21:
			COMPV_CHECK_CODE_RETURN(CompVImageConvToRGB24::yuvSemiPlanar(imageIn, imageOut));
			break;

		case COMPV_SUBTYPE_PIXELS_YUYV422:
		case COMPV_SUBTYPE_PIXELS_UYVY422:
			COMPV_CHECK_CODE_RETURN(CompVImageConvToRGB24::yuvPacked(imageIn, imageOut));
			break;

		default:
			COMPV_DEBUG_ERROR("%s -> RGB24 not supported", CompVGetSubtypeString(imageIn->subType()));
			return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
	}

	*imageRGB24 = imageOut;

	return COMPV_ERROR_CODE_S_OK;
}

// YUV420P, YVU420P, YUV422P, YUV444P
COMPV_ERROR_CODE CompVImageConvToRGB24::yuvPlanar(const CompVMatPtr& imageIn, CompVMatPtr& imageRGB24)
{
	// Internal function, do not check input parameters (already done)
	void(*planar_to_rgb24)(const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
		= nullptr;
	const COMPV_SUBTYPE inPixelFormat = imageIn->subType();
	switch (inPixelFormat) {
	case COMPV_SUBTYPE_PIXELS_YUV420P:
		planar_to_rgb24 = yuv420p_to_rgb24_C;
		break;
	case COMPV_SUBTYPE_PIXELS_YUV422P:
		planar_to_rgb24 = yuv422p_to_rgb24_C;
		break;
	case COMPV_SUBTYPE_PIXELS_YUV444P:
		planar_to_rgb24 = yuv444p_to_rgb24_C;
		break;
	default:
		COMPV_DEBUG_ERROR("%s -> RGB24 not supported", CompVGetSubtypeString(imageIn->subType()));
		return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
	}

	const size_t widthInSamples = imageRGB24->cols();
	const size_t heightInSamples = imageRGB24->rows();
	const size_t strideInSamples = imageRGB24->stride();

	const uint8_t* yPtr = imageIn->ptr<const uint8_t>(0, 0, COMPV_PLANE_Y);
	const uint8_t* uPtr = imageIn->ptr<const uint8_t>(0, 0, COMPV_PLANE_U);
	const uint8_t* vPtr = imageIn->ptr<const uint8_t>(0, 0, COMPV_PLANE_V);
	uint8_t* rgbPtr = imageRGB24->ptr<uint8_t>();

	CompVThreadDispatcherPtr threadDisp = CompVParallel::threadDispatcher();
	size_t maxThreads = threadDisp ? static_cast<size_t>(threadDisp->threadsCount()) : 0;

	// Compute number of threads
	const size_t threadsCount = (threadDisp && !threadDisp->isMotherOfTheCurrentThread())
		? CompVThreadDispatcher::guessNumThreadsDividingAcrossY(strideInSamples, heightInSamples, maxThreads, COMPV_IMAGE_CONV_MIN_SAMPLES_PER_THREAD)
		: 1;

	if (threadsCount > 1) {
		const size_t heights = (heightInSamples / threadsCount) & -2; //!\\ must be even number
		const size_t lastHeight = heightInSamples - ((threadsCount - 1) * heights);
		size_t yPtrPaddingInBytes, uPtrPaddingInBytes, vPtrPaddingInBytes, rgbPtrPaddingInBytes;
		size_t uPtrHeights, vPtrHeights, tmpWidth;
		CompVAsyncTaskIds taskIds;
		taskIds.reserve(threadsCount);
		auto funcPtr = [&](const uint8_t* yPtr_, const uint8_t* uPtr_, const uint8_t* vPtr_, uint8_t* rgbPtr_, compv_uscalar_t heightInSamples_) -> void {
			planar_to_rgb24(
				yPtr_, uPtr_, vPtr_, rgbPtr_, 
				widthInSamples, heightInSamples_, strideInSamples
			);
		};

		COMPV_CHECK_CODE_RETURN(CompVImageUtils::planeSizeForPixelFormat(inPixelFormat, COMPV_PLANE_U, 1, heights, &tmpWidth, &uPtrHeights));
		COMPV_CHECK_CODE_RETURN(CompVImageUtils::planeSizeForPixelFormat(inPixelFormat, COMPV_PLANE_V, 1, heights, &tmpWidth, &vPtrHeights));

		yPtrPaddingInBytes = imageIn->strideInBytes(COMPV_PLANE_Y) * heights;
		uPtrPaddingInBytes = imageIn->strideInBytes(COMPV_PLANE_U) * uPtrHeights;
		vPtrPaddingInBytes = imageIn->strideInBytes(COMPV_PLANE_U) * vPtrHeights;
		rgbPtrPaddingInBytes = imageRGB24->strideInBytes() * heights;
		
		for (size_t threadIdx = 0; threadIdx < threadsCount - 1; ++threadIdx) {
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtr, yPtr, uPtr, vPtr, rgbPtr, heights), taskIds), "Dispatching task failed");
			yPtr += yPtrPaddingInBytes;
			uPtr += uPtrPaddingInBytes;
			vPtr += vPtrPaddingInBytes;
			rgbPtr += rgbPtrPaddingInBytes;
		}
		if (lastHeight > 0) {
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtr, yPtr, uPtr, vPtr, rgbPtr, lastHeight), taskIds), "Dispatching task failed");
		}
		COMPV_CHECK_CODE_RETURN(threadDisp->wait(taskIds), "Failed to wait for tasks execution");
	}
	else {
		planar_to_rgb24(
			yPtr, uPtr, vPtr, rgbPtr,
			widthInSamples, heightInSamples, strideInSamples
		);
	}

	return COMPV_ERROR_CODE_S_OK;
}

// NV12, NV21
COMPV_ERROR_CODE CompVImageConvToRGB24::yuvSemiPlanar(const CompVMatPtr& imageIn, CompVMatPtr& imageRGB24)
{
	// Internal function, do not check input parameters (already done)
	void(*semiplanar_to_rgb24)(const uint8_t* yPtr, const uint8_t* uvPtr, uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
		= nullptr;
	const COMPV_SUBTYPE inPixelFormat = imageIn->subType();
	switch (inPixelFormat) {
	case COMPV_SUBTYPE_PIXELS_NV12:
		semiplanar_to_rgb24 = nv12_to_rgb24_C;
		break;
	case COMPV_SUBTYPE_PIXELS_NV21:
		semiplanar_to_rgb24 = nv21_to_rgb24_C;
		break;
	default:
		COMPV_DEBUG_ERROR("%s -> RGB24 not supported", CompVGetSubtypeString(imageIn->subType()));
		return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
	}

	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No MT implementation found");

	const size_t widthInSamples = imageRGB24->cols();
	const size_t heightInSamples = imageRGB24->rows();
	const size_t strideInSamples = imageRGB24->stride();

	const uint8_t* yPtr = imageIn->ptr<const uint8_t>(0, 0, COMPV_PLANE_Y);
	const uint8_t* uvPtr = imageIn->ptr<const uint8_t>(0, 0, COMPV_PLANE_UV);
	uint8_t* rgbPtr = imageRGB24->ptr<uint8_t>();

	CompVThreadDispatcherPtr threadDisp = CompVParallel::threadDispatcher();
	size_t maxThreads = threadDisp ? static_cast<size_t>(threadDisp->threadsCount()) : 0;

	// Compute number of threads
	const size_t threadsCount = (threadDisp && !threadDisp->isMotherOfTheCurrentThread())
		? CompVThreadDispatcher::guessNumThreadsDividingAcrossY(strideInSamples, heightInSamples, maxThreads, COMPV_IMAGE_CONV_MIN_SAMPLES_PER_THREAD)
		: 1;

	if (threadsCount > 1) {
		const size_t heights = (heightInSamples / threadsCount) & -2; //!\\ must be even number
		const size_t lastHeight = heightInSamples - ((threadsCount - 1) * heights);
		size_t yPtrPaddingInBytes, uvPtrPaddingInBytes, rgbPtrPaddingInBytes;
		size_t uvPtrHeights, tmpWidth;
		CompVAsyncTaskIds taskIds;
		taskIds.reserve(threadsCount);
		auto funcPtr = [&](const uint8_t* yPtr_, const uint8_t* uvPtr_, uint8_t* rgbPtr_, compv_uscalar_t heightInSamples_) -> void {
			semiplanar_to_rgb24(
				yPtr_, uvPtr_, rgbPtr_,
				widthInSamples, heightInSamples_, strideInSamples
			);
		};

		COMPV_CHECK_CODE_RETURN(CompVImageUtils::planeSizeForPixelFormat(inPixelFormat, COMPV_PLANE_UV, 1, heights, &tmpWidth, &uvPtrHeights));

		yPtrPaddingInBytes = imageIn->strideInBytes(COMPV_PLANE_Y) * heights;
		uvPtrPaddingInBytes = imageIn->strideInBytes(COMPV_PLANE_UV) * (uvPtrHeights << 1); //!\\ mul height by 2 because we U and V planes are packed with the same size
		rgbPtrPaddingInBytes = imageRGB24->strideInBytes() * heights;

		for (size_t threadIdx = 0; threadIdx < threadsCount - 1; ++threadIdx) {
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtr, yPtr, uvPtr, rgbPtr, heights), taskIds), "Dispatching task failed");
			yPtr += yPtrPaddingInBytes;
			uvPtr += uvPtrPaddingInBytes;
			rgbPtr += rgbPtrPaddingInBytes;
		}
		if (lastHeight > 0) {
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtr, yPtr, uvPtr, rgbPtr, lastHeight), taskIds), "Dispatching task failed");
		}
		COMPV_CHECK_CODE_RETURN(threadDisp->wait(taskIds), "Failed to wait for tasks execution");
	}
	else {
		semiplanar_to_rgb24(
			yPtr, uvPtr, rgbPtr,
			widthInSamples, heightInSamples, strideInSamples
		);
	}

	return COMPV_ERROR_CODE_S_OK;
}

// YUYV422, UYVY422
COMPV_ERROR_CODE CompVImageConvToRGB24::yuvPacked(const CompVMatPtr& imageIn, CompVMatPtr& imageRGB24)
{
	// Internal function, do not check input parameters (already done)
	void(*packed_to_rgb24)(const uint8_t* yuyvPtr, uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
		= nullptr;
	const COMPV_SUBTYPE inPixelFormat = imageIn->subType();
	switch (inPixelFormat) {
	case COMPV_SUBTYPE_PIXELS_YUYV422:
		packed_to_rgb24 = yuyv422_to_rgb24_C;
		break;
	case COMPV_SUBTYPE_PIXELS_UYVY422:
		packed_to_rgb24 = uyvy422_to_rgb24_C;
		break;
	default:
		COMPV_DEBUG_ERROR("%s -> RGB24 not supported", CompVGetSubtypeString(imageIn->subType()));
		return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
	}

	const size_t widthInSamples = imageRGB24->cols();
	const size_t heightInSamples = imageRGB24->rows();
	const size_t strideInSamples = imageRGB24->stride();

	const uint8_t* yuvPtr = imageIn->ptr<const uint8_t>();
	uint8_t* rgbPtr = imageRGB24->ptr<uint8_t>();

	CompVThreadDispatcherPtr threadDisp = CompVParallel::threadDispatcher();
	size_t maxThreads = threadDisp ? static_cast<size_t>(threadDisp->threadsCount()) : 0;

	// Compute number of threads
	const size_t threadsCount = (threadDisp && !threadDisp->isMotherOfTheCurrentThread())
		? CompVThreadDispatcher::guessNumThreadsDividingAcrossY(strideInSamples, heightInSamples, maxThreads, COMPV_IMAGE_CONV_MIN_SAMPLES_PER_THREAD)
		: 1;

	if (threadsCount > 1) {
		const size_t heights = (heightInSamples / threadsCount) & -2; //!\\ must be even number
		const size_t lastHeight = heightInSamples - ((threadsCount - 1) * heights);
		size_t yuvPtrPaddingInBytes, rgbPtrPaddingInBytes;
		size_t yuvPtrHeights, tmpWidth;
		CompVAsyncTaskIds taskIds;
		taskIds.reserve(threadsCount);
		auto funcPtr = [&](const uint8_t* yuvPtr_, uint8_t* rgbPtr_, compv_uscalar_t heightInSamples_) -> void {
			packed_to_rgb24(
				yuvPtr_, rgbPtr_,
				widthInSamples, heightInSamples_, strideInSamples
			);
		};

		COMPV_CHECK_CODE_RETURN(CompVImageUtils::planeSizeForPixelFormat(inPixelFormat, 0, 1, heights, &tmpWidth, &yuvPtrHeights));

		yuvPtrPaddingInBytes = imageIn->strideInBytes() * yuvPtrHeights;
		rgbPtrPaddingInBytes = imageRGB24->strideInBytes() * heights;

		for (size_t threadIdx = 0; threadIdx < threadsCount - 1; ++threadIdx) {
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtr, yuvPtr, rgbPtr, heights), taskIds), "Dispatching task failed");
			yuvPtr += yuvPtrPaddingInBytes;
			rgbPtr += rgbPtrPaddingInBytes;
		}
		if (lastHeight > 0) {
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtr, yuvPtr, rgbPtr, lastHeight), taskIds), "Dispatching task failed");
		}
		COMPV_CHECK_CODE_RETURN(threadDisp->wait(taskIds), "Failed to wait for tasks execution");
	}
	else {
		packed_to_rgb24(
			yuvPtr, rgbPtr,
			widthInSamples, heightInSamples, strideInSamples
		);
	}

	return COMPV_ERROR_CODE_S_OK;
}


// R = (37Y' + 0U' + 51V') >> 5
// G = (37Y' - 13U' - 26V') >> 5
// B = (37Y' + 65U' + 0V') >> 5
// where Y'=(Y - 16), U' = (U - 128), V'=(V - 128)
// For ASM code _mm_subs_epu8(U, 128) produce overflow -> use I16
// R!i16 = (37Y + 0U + 51V - 7120) >> 5
// G!i16 = (37Y - 13U - 26V + 4400) >> 5
// B!i16 = (37Y + 65U + 0V - 8912) >> 5
#define COMPV_YUV_TO_RGB(yp, up, vp, r, g, b) \
	r = CompVMathUtils::clampPixel8((37 * yp + 51 * vp) >> 5); \
	g = CompVMathUtils::clampPixel8((37 * yp - 13 * up - 26 * vp) >> 5); \
	b = CompVMathUtils::clampPixel8((37 * yp + 65 * up) >> 5)

#define yuv420p_to_rgb24_strideUV() const compv_uscalar_t strideUV = ((stride + 1) >> 1);
#define yuv422p_to_rgb24_strideUV() const compv_uscalar_t strideUV = ((stride + 1) >> 1);
#define yuv444p_to_rgb24_strideUV() const compv_uscalar_t strideUV = stride;

#define yuv420p_to_rgb24_indexUV() (i >> 1)
#define yuv422p_to_rgb24_indexUV() (i >> 1)
#define yuv444p_to_rgb24_indexUV() (i)

#define yuv420p_to_rgb24_checkAddStrideUV() if (j & 1)
#define yuv422p_to_rgb24_checkAddStrideUV() 
#define yuv444p_to_rgb24_checkAddStrideUV() 

#define planar_to_rgb24(name, yPtr, uPtr, vPtr, rgbPtr, width, height, stride) { \
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found"); \
		const compv_uscalar_t strideRGB = stride * 3; \
		const compv_uscalar_t strideY = stride; \
		name##_to_rgb24_strideUV(); \
		int16_t Yp, Up, Vp; \
		compv_uscalar_t i, j, k; \
		for (j = 0; j < height; ++j) { \
			for (i = 0, k = 0; i < width; ++i, k += 3) { \
				Yp = (yPtr[i] - 16); \
				Up = (uPtr[name##_to_rgb24_indexUV()] - 127); \
				Vp = (vPtr[name##_to_rgb24_indexUV()] - 127); \
				COMPV_YUV_TO_RGB(Yp, Up, Vp, rgbPtr[k + 0], rgbPtr[k + 1], rgbPtr[k + 2]); \
			} \
			rgbPtr += strideRGB; \
			yPtr += strideY; \
			name##_to_rgb24_checkAddStrideUV() { \
				uPtr += strideUV; \
				vPtr += strideUV; \
			} \
		} \
	}

static void yuv420p_to_rgb24_C(const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	planar_to_rgb24(yuv420p, yPtr, uPtr, vPtr, rgbPtr, width, height, stride);
}

static void yuv422p_to_rgb24_C(const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	planar_to_rgb24(yuv422p, yPtr, uPtr, vPtr, rgbPtr, width, height, stride);
}

static void yuv444p_to_rgb24_C(const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	planar_to_rgb24(yuv444p, yPtr, uPtr, vPtr, rgbPtr, width, height, stride);
}

#define nv12_to_rgb24_indexU() 0
#define nv12_to_rgb24_indexV() 1

#define nv21_to_rgb24_indexU() 1
#define nv21_to_rgb24_indexV() 0

#define semiplanar_to_rgb24(name, yPtr, uvPtr, rgbPtr, width, height, stride) { \
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found"); \
		const compv_uscalar_t strideRGB = stride * 3; \
		int16_t Yp, Up, Vp; \
		for (compv_uscalar_t j = 0; j < height; ++j) { \
			for (compv_uscalar_t i = 0, k = 0; i < width; ++i, k += 3) { \
				Yp = (yPtr[i] - 16); \
				Up = (uvPtr[(i & -2) + name##_to_rgb24_indexU()] - 127); \
				Vp = (uvPtr[(i & -2) + name##_to_rgb24_indexV()] - 127); \
				COMPV_YUV_TO_RGB(Yp, Up, Vp, rgbPtr[k + 0], rgbPtr[k + 1], rgbPtr[k + 2]); \
			} \
			rgbPtr += strideRGB; \
			yPtr += stride; \
			if (j & 1) { \
				uvPtr += stride; \
			} \
		} \
	}

static void nv12_to_rgb24_C(const uint8_t* yPtr, const uint8_t* uvPtr, uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	semiplanar_to_rgb24(nv12, yPtr, uvPtr, rgbPtr, width, height, stride);
}

static void nv21_to_rgb24_C(const uint8_t* yPtr, const uint8_t* uvPtr, uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	semiplanar_to_rgb24(nv21, yPtr, uvPtr, rgbPtr, width, height, stride);
}


#define yuyv422_to_rgb24_indexY() 0
#define yuyv422_to_rgb24_indexU() 1
#define yuyv422_to_rgb24_indexV() 3

#define uyvy422_to_rgb24_indexY() 1
#define uyvy422_to_rgb24_indexU() 0
#define uyvy422_to_rgb24_indexV() 2

#define packed_to_rgb24(name, yuyvPtr, rgbPtr, width, height, stride) { \
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found"); \
		const compv_uscalar_t padSample = (stride - width); \
		const compv_uscalar_t strideRGB = stride * 3; \
		const compv_uscalar_t maxWidth = (width << 1); \
		int16_t Yp, Up, Vp; \
		for (compv_uscalar_t j = 0; j < height; ++j) { \
			for (compv_uscalar_t i = 0, k = 0; i < maxWidth; i += 2, k += 3) { \
				Yp = (yuyvPtr[i + name##_to_rgb24_indexY()] - 16); \
				Up = (yuyvPtr[((i >> 2) << 2) + name##_to_rgb24_indexU()] - 127); \
				Vp = (yuyvPtr[((i >> 2) << 2) + name##_to_rgb24_indexV()] - 127); \
				COMPV_YUV_TO_RGB(Yp, Up, Vp, rgbPtr[k + 0], rgbPtr[k + 1], rgbPtr[k + 2]); \
			} \
			rgbPtr += strideRGB; \
			if (j & 1) { \
				yuyvPtr += (stride << 2); \
			} \
		} \
	}

static void yuyv422_to_rgb24_C(const uint8_t* yuyvPtr, uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	packed_to_rgb24(yuyv422, yuyvPtr, rgbPtr, width, height, stride);
}

static void uyvy422_to_rgb24_C(const uint8_t* yuyvPtr, uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	packed_to_rgb24(uyvy422, yuyvPtr, rgbPtr, width, height, stride);
}

COMPV_NAMESPACE_END()
