/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/compv_image_conv_to_rgba32.h"
#include "compv/base/image/compv_image.h"
#include "compv/base/image/compv_image_conv_common.h"
#include "compv/base/math/compv_math_utils.h"
#include "compv/base/parallel/compv_parallel.h"

#include "compv/base/image/intrin/x86/compv_image_conv_to_rgba32_intrin_sse2.h"
#include "compv/base/image/intrin/x86/compv_image_conv_to_rgba32_intrin_avx2.h"

#define COMPV_THIS_CLASSNAME	"CompVImageConvToRGBA32"

COMPV_NAMESPACE_BEGIN()

static void yuv420p_to_rgba32_C(const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, uint8_t* rgbaPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
static void yuv422p_to_rgba32_C(const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, uint8_t* rgbaPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
static void yuv444p_to_rgba32_C(const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, uint8_t* rgbaPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
static void nv12_to_rgba32_C(const uint8_t* yPtr, const uint8_t* uvPtr, uint8_t* rgbaPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
static void nv21_to_rgba32_C(const uint8_t* yPtr, const uint8_t* uvPtr, uint8_t* rgbaPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
static void yuyv422_to_rgba32_C(const uint8_t* yuyvPtr, uint8_t* rgbaPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
static void uyvy422_to_rgba32_C(const uint8_t* yuyvPtr, uint8_t* rgbaPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);

COMPV_ERROR_CODE CompVImageConvToRGBA32::process(const CompVMatPtr& imageIn, CompVMatPtrPtr imageRGBA32)
{
	// Internal function, do not check input parameters (already done)

	CompVMatPtr imageOut = (*imageRGBA32 == imageIn) ? nullptr : *imageRGBA32;
	COMPV_CHECK_CODE_RETURN(CompVImage::newObj8u(&imageOut, COMPV_SUBTYPE_PIXELS_RGBA32, imageIn->cols(), imageIn->rows(), imageIn->stride()));

	switch (imageIn->subType()) {
		case COMPV_SUBTYPE_PIXELS_RGBA32:
			COMPV_CHECK_CODE_RETURN(CompVImage::clone(imageIn, &imageOut));
			break;

		case COMPV_SUBTYPE_PIXELS_YUV420P:
		case COMPV_SUBTYPE_PIXELS_YUV422P:
		case COMPV_SUBTYPE_PIXELS_YUV444P:
			COMPV_CHECK_CODE_RETURN(CompVImageConvToRGBA32::yuvPlanar(imageIn, imageOut));
			break;

		case COMPV_SUBTYPE_PIXELS_NV12:
		case COMPV_SUBTYPE_PIXELS_NV21:
			COMPV_CHECK_CODE_RETURN(CompVImageConvToRGBA32::yuvSemiPlanar(imageIn, imageOut));
			break;

		case COMPV_SUBTYPE_PIXELS_YUYV422:
		case COMPV_SUBTYPE_PIXELS_UYVY422:
			COMPV_CHECK_CODE_RETURN(CompVImageConvToRGBA32::yuvPacked(imageIn, imageOut));
			break;

		default:
			COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "%s -> RGB24 not supported", CompVGetSubtypeString(imageIn->subType()));
			return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
	}

#if 0
	COMPV_DEBUG_INFO_CODE_FOR_TESTING();
	const uint8_t *r = imageOut->ptr<const uint8_t>();
	for (size_t i = 0; i < 100; i+=4) {
		printf("%u, ", r[i]);
	}
#endif

	*imageRGBA32 = imageOut;

	return COMPV_ERROR_CODE_S_OK;
}

// YUV420P, YVU420P, YUV422P, YUV444P
COMPV_ERROR_CODE CompVImageConvToRGBA32::yuvPlanar(const CompVMatPtr& imageIn, CompVMatPtr& imageRGBA32)
{
	// Internal function, do not check input parameters (already done)
	void(*planar_to_rgba32)(const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, uint8_t* rgbaPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
		= nullptr;
	const COMPV_SUBTYPE inPixelFormat = imageIn->subType();
	switch (inPixelFormat) {
	case COMPV_SUBTYPE_PIXELS_YUV420P:
		planar_to_rgba32 = yuv420p_to_rgba32_C;
#if COMPV_ARCH_X86
		if (CompVCpu::isEnabled(kCpuFlagSSE2) && imageRGBA32->isAlignedSSE(0) && imageIn->isAlignedSSE(0) && imageIn->isAlignedSSE(1) && imageIn->isAlignedSSE(2)) {
			COMPV_EXEC_IFDEF_INTRIN_X86(planar_to_rgba32 = CompVImageConvYuv420_to_Rgba32_Intrin_SSE2);
		}
		if (CompVCpu::isEnabled(kCpuFlagAVX2) && imageRGBA32->isAlignedAVX(0) && imageIn->isAlignedAVX(0) && imageIn->isAlignedAVX(1) && imageIn->isAlignedAVX(2)) {
			COMPV_EXEC_IFDEF_INTRIN_X86(planar_to_rgba32 = CompVImageConvYuv420_to_Rgba32_Intrin_AVX2);
		}
#elif COMPV_ARCH_ARM
#endif
		break;
	case COMPV_SUBTYPE_PIXELS_YUV422P:
		planar_to_rgba32 = yuv422p_to_rgba32_C;
		break;
	case COMPV_SUBTYPE_PIXELS_YUV444P:
		planar_to_rgba32 = yuv444p_to_rgba32_C;
		break;
	default:
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "%s -> RGB24 not supported", CompVGetSubtypeString(imageIn->subType()));
		return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
	}

	const size_t widthInSamples = imageRGBA32->cols();
	const size_t heightInSamples = imageRGBA32->rows();
	const size_t strideInSamples = imageRGBA32->stride();

	const uint8_t* yPtr = imageIn->ptr<const uint8_t>(0, 0, COMPV_PLANE_Y);
	const uint8_t* uPtr = imageIn->ptr<const uint8_t>(0, 0, COMPV_PLANE_U);
	const uint8_t* vPtr = imageIn->ptr<const uint8_t>(0, 0, COMPV_PLANE_V);
	uint8_t* rgbaPtr = imageRGBA32->ptr<uint8_t>();

	CompVThreadDispatcherPtr threadDisp = CompVParallel::threadDispatcher();
	size_t maxThreads = threadDisp ? static_cast<size_t>(threadDisp->threadsCount()) : 0;

	// Compute number of threads
	const size_t threadsCount = (threadDisp && !threadDisp->isMotherOfTheCurrentThread())
		? CompVThreadDispatcher::guessNumThreadsDividingAcrossY(strideInSamples, heightInSamples, maxThreads, COMPV_IMAGE_CONV_MIN_SAMPLES_PER_THREAD)
		: 1;

	if (threadsCount > 1) {
		const size_t heights = (heightInSamples / threadsCount) & -2; //!\\ must be even number
		const size_t lastHeight = heightInSamples - ((threadsCount - 1) * heights);
		size_t yPtrPaddingInBytes, uPtrPaddingInBytes, vPtrPaddingInBytes, rgbaPtrPaddingInBytes;
		size_t uPtrHeights, vPtrHeights, tmpWidth;
		CompVAsyncTaskIds taskIds;
		taskIds.reserve(threadsCount);
		auto funcPtr = [&](const uint8_t* yPtr_, const uint8_t* uPtr_, const uint8_t* vPtr_, uint8_t* rgbaPtr_, compv_uscalar_t heightInSamples_) -> void {
			planar_to_rgba32(
				yPtr_, uPtr_, vPtr_, rgbaPtr_, 
				widthInSamples, heightInSamples_, strideInSamples
			);
		};

		COMPV_CHECK_CODE_RETURN(CompVImageUtils::planeSizeForPixelFormat(inPixelFormat, COMPV_PLANE_U, 1, heights, &tmpWidth, &uPtrHeights));
		COMPV_CHECK_CODE_RETURN(CompVImageUtils::planeSizeForPixelFormat(inPixelFormat, COMPV_PLANE_V, 1, heights, &tmpWidth, &vPtrHeights));

		yPtrPaddingInBytes = imageIn->strideInBytes(COMPV_PLANE_Y) * heights;
		uPtrPaddingInBytes = imageIn->strideInBytes(COMPV_PLANE_U) * uPtrHeights;
		vPtrPaddingInBytes = imageIn->strideInBytes(COMPV_PLANE_U) * vPtrHeights;
		rgbaPtrPaddingInBytes = imageRGBA32->strideInBytes() * heights;
		
		for (size_t threadIdx = 0; threadIdx < threadsCount - 1; ++threadIdx) {
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtr, yPtr, uPtr, vPtr, rgbaPtr, heights), taskIds), "Dispatching task failed");
			yPtr += yPtrPaddingInBytes;
			uPtr += uPtrPaddingInBytes;
			vPtr += vPtrPaddingInBytes;
			rgbaPtr += rgbaPtrPaddingInBytes;
		}
		if (lastHeight > 0) {
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtr, yPtr, uPtr, vPtr, rgbaPtr, lastHeight), taskIds), "Dispatching task failed");
		}
		COMPV_CHECK_CODE_RETURN(threadDisp->wait(taskIds), "Failed to wait for tasks execution");
	}
	else {
		planar_to_rgba32(
			yPtr, uPtr, vPtr, rgbaPtr,
			widthInSamples, heightInSamples, strideInSamples
		);
	}

	return COMPV_ERROR_CODE_S_OK;
}

// NV12, NV21
COMPV_ERROR_CODE CompVImageConvToRGBA32::yuvSemiPlanar(const CompVMatPtr& imageIn, CompVMatPtr& imageRGBA32)
{
	// Internal function, do not check input parameters (already done)
	void(*semiplanar_to_rgba32)(const uint8_t* yPtr, const uint8_t* uvPtr, uint8_t* rgbaPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
		= nullptr;
	const COMPV_SUBTYPE inPixelFormat = imageIn->subType();
	switch (inPixelFormat) {
	case COMPV_SUBTYPE_PIXELS_NV12:
		semiplanar_to_rgba32 = nv12_to_rgba32_C;
		break;
	case COMPV_SUBTYPE_PIXELS_NV21:
		semiplanar_to_rgba32 = nv21_to_rgba32_C;
		break;
	default:
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "%s -> RGB24 not supported", CompVGetSubtypeString(imageIn->subType()));
		return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
	}

	const size_t widthInSamples = imageRGBA32->cols();
	const size_t heightInSamples = imageRGBA32->rows();
	const size_t strideInSamples = imageRGBA32->stride();

	const uint8_t* yPtr = imageIn->ptr<const uint8_t>(0, 0, COMPV_PLANE_Y);
	const uint8_t* uvPtr = imageIn->ptr<const uint8_t>(0, 0, COMPV_PLANE_UV);
	uint8_t* rgbaPtr = imageRGBA32->ptr<uint8_t>();

	CompVThreadDispatcherPtr threadDisp = CompVParallel::threadDispatcher();
	size_t maxThreads = threadDisp ? static_cast<size_t>(threadDisp->threadsCount()) : 0;

	// Compute number of threads
	const size_t threadsCount = (threadDisp && !threadDisp->isMotherOfTheCurrentThread())
		? CompVThreadDispatcher::guessNumThreadsDividingAcrossY(strideInSamples, heightInSamples, maxThreads, COMPV_IMAGE_CONV_MIN_SAMPLES_PER_THREAD)
		: 1;

	if (threadsCount > 1) {
		const size_t heights = (heightInSamples / threadsCount) & -2; //!\\ must be even number
		const size_t lastHeight = heightInSamples - ((threadsCount - 1) * heights);
		size_t yPtrPaddingInBytes, uvPtrPaddingInBytes, rgbaPtrPaddingInBytes;
		size_t uvPtrHeights, tmpWidth;
		CompVAsyncTaskIds taskIds;
		taskIds.reserve(threadsCount);
		auto funcPtr = [&](const uint8_t* yPtr_, const uint8_t* uvPtr_, uint8_t* rgbaPtr_, compv_uscalar_t heightInSamples_) -> void {
			semiplanar_to_rgba32(
				yPtr_, uvPtr_, rgbaPtr_,
				widthInSamples, heightInSamples_, strideInSamples
			);
		};

		COMPV_CHECK_CODE_RETURN(CompVImageUtils::planeSizeForPixelFormat(inPixelFormat, COMPV_PLANE_UV, 1, heights, &tmpWidth, &uvPtrHeights));

		yPtrPaddingInBytes = imageIn->strideInBytes(COMPV_PLANE_Y) * heights;
		uvPtrPaddingInBytes = imageIn->strideInBytes(COMPV_PLANE_UV) * uvPtrHeights;
		rgbaPtrPaddingInBytes = imageRGBA32->strideInBytes() * heights;

		for (size_t threadIdx = 0; threadIdx < threadsCount - 1; ++threadIdx) {
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtr, yPtr, uvPtr, rgbaPtr, heights), taskIds), "Dispatching task failed");
			yPtr += yPtrPaddingInBytes;
			uvPtr += uvPtrPaddingInBytes;
			rgbaPtr += rgbaPtrPaddingInBytes;
		}
		if (lastHeight > 0) {
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtr, yPtr, uvPtr, rgbaPtr, lastHeight), taskIds), "Dispatching task failed");
		}
		COMPV_CHECK_CODE_RETURN(threadDisp->wait(taskIds), "Failed to wait for tasks execution");
	}
	else {
		semiplanar_to_rgba32(
			yPtr, uvPtr, rgbaPtr,
			widthInSamples, heightInSamples, strideInSamples
		);
	}

	return COMPV_ERROR_CODE_S_OK;
}

// YUYV422, UYVY422
COMPV_ERROR_CODE CompVImageConvToRGBA32::yuvPacked(const CompVMatPtr& imageIn, CompVMatPtr& imageRGBA32)
{
	// Internal function, do not check input parameters (already done)
	void(*packed_to_rgba32)(const uint8_t* yuyvPtr, uint8_t* rgbaPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
		= nullptr;
	const COMPV_SUBTYPE inPixelFormat = imageIn->subType();
	switch (inPixelFormat) {
	case COMPV_SUBTYPE_PIXELS_YUYV422:
		packed_to_rgba32 = yuyv422_to_rgba32_C;
		break;
	case COMPV_SUBTYPE_PIXELS_UYVY422:
		packed_to_rgba32 = uyvy422_to_rgba32_C;
		break;
	default:
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "%s -> RGB24 not supported", CompVGetSubtypeString(imageIn->subType()));
		return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
	}

	const size_t widthInSamples = imageRGBA32->cols();
	const size_t heightInSamples = imageRGBA32->rows();
	const size_t strideInSamples = imageRGBA32->stride();

	const uint8_t* yuvPtr = imageIn->ptr<const uint8_t>();
	uint8_t* rgbaPtr = imageRGBA32->ptr<uint8_t>();

	CompVThreadDispatcherPtr threadDisp = CompVParallel::threadDispatcher();
	size_t maxThreads = threadDisp ? static_cast<size_t>(threadDisp->threadsCount()) : 0;

	// Compute number of threads
	const size_t threadsCount = (threadDisp && !threadDisp->isMotherOfTheCurrentThread())
		? CompVThreadDispatcher::guessNumThreadsDividingAcrossY(strideInSamples, heightInSamples, maxThreads, COMPV_IMAGE_CONV_MIN_SAMPLES_PER_THREAD)
		: 1;

	if (threadsCount > 1) {
		const size_t heights = (heightInSamples / threadsCount) & -2; //!\\ must be even number
		const size_t lastHeight = heightInSamples - ((threadsCount - 1) * heights);
		size_t yuvPtrPaddingInBytes, rgbaPtrPaddingInBytes;
		size_t yuvPtrHeights, tmpWidth;
		CompVAsyncTaskIds taskIds;
		taskIds.reserve(threadsCount);
		auto funcPtr = [&](const uint8_t* yuvPtr_, uint8_t* rgbaPtr_, compv_uscalar_t heightInSamples_) -> void {
			packed_to_rgba32(
				yuvPtr_, rgbaPtr_,
				widthInSamples, heightInSamples_, strideInSamples
			);
		};

		COMPV_CHECK_CODE_RETURN(CompVImageUtils::planeSizeForPixelFormat(inPixelFormat, 0, 1, heights, &tmpWidth, &yuvPtrHeights));

		yuvPtrPaddingInBytes = imageIn->strideInBytes() * yuvPtrHeights;
		rgbaPtrPaddingInBytes = imageRGBA32->strideInBytes() * heights;

		for (size_t threadIdx = 0; threadIdx < threadsCount - 1; ++threadIdx) {
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtr, yuvPtr, rgbaPtr, heights), taskIds), "Dispatching task failed");
			yuvPtr += yuvPtrPaddingInBytes;
			rgbaPtr += rgbaPtrPaddingInBytes;
		}
		if (lastHeight > 0) {
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtr, yuvPtr, rgbaPtr, lastHeight), taskIds), "Dispatching task failed");
		}
		COMPV_CHECK_CODE_RETURN(threadDisp->wait(taskIds), "Failed to wait for tasks execution");
	}
	else {
		packed_to_rgba32(
			yuvPtr, rgbaPtr,
			widthInSamples, heightInSamples, strideInSamples
		);
	}

	return COMPV_ERROR_CODE_S_OK;
}


// R = (37Y' + 0U' + 51V') >> 5
// G = (37Y' - 13U' - 26V') >> 5
// B = (37Y' + 65U' + 0V') >> 5
// A = 255
// where Y'=(Y - 16), U' = (U - 128), V'=(V - 128)
#define COMPV_YUV_TO_RGBA(yp, up, vp, r, g, b, a) \
	r = CompVMathUtils::clampPixel8((37 * yp + 51 * vp) >> 5); \
	g = CompVMathUtils::clampPixel8((37 * yp - 13 * up - 26 * vp) >> 5); \
	b = CompVMathUtils::clampPixel8((37 * yp + 65 * up) >> 5); \
	a = 0xff

#define yuv420p_to_rgba32_strideUV() const compv_uscalar_t strideUV = ((stride + 1) >> 1);
#define yuv422p_to_rgba32_strideUV() const compv_uscalar_t strideUV = ((stride + 1) >> 1);
#define yuv444p_to_rgba32_strideUV() const compv_uscalar_t strideUV = stride;

#define yuv420p_to_rgba32_indexUV() (i >> 1)
#define yuv422p_to_rgba32_indexUV() (i >> 1)
#define yuv444p_to_rgba32_indexUV() (i)

#define yuv420p_to_rgba32_checkAddStrideUV() if (j & 1)
#define yuv422p_to_rgba32_checkAddStrideUV() 
#define yuv444p_to_rgba32_checkAddStrideUV() 

#define planar_to_rgba32(name, yPtr, uPtr, vPtr, rgbaPtr, width, height, stride) { \
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found"); \
		const compv_uscalar_t strideRGBA = stride << 2; \
		const compv_uscalar_t strideY = stride; \
		name##_to_rgba32_strideUV(); \
		int16_t Yp, Up, Vp; \
		compv_uscalar_t i, j, k; \
		for (j = 0; j < height; ++j) { \
			for (i = 0, k = 0; i < width; ++i, k += 4) { \
				Yp = (yPtr[i] - 16); \
				Up = (uPtr[name##_to_rgba32_indexUV()] - 127); \
				Vp = (vPtr[name##_to_rgba32_indexUV()] - 127); \
				COMPV_YUV_TO_RGBA(Yp, Up, Vp, rgbaPtr[k + 0], rgbaPtr[k + 1], rgbaPtr[k + 2], rgbaPtr[k + 3]); \
			} \
			rgbaPtr += strideRGBA; \
			yPtr += strideY; \
			name##_to_rgba32_checkAddStrideUV() { \
				uPtr += strideUV; \
				vPtr += strideUV; \
			} \
		} \
	}

static void yuv420p_to_rgba32_C(const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, uint8_t* rgbaPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	planar_to_rgba32(yuv420p, yPtr, uPtr, vPtr, rgbaPtr, width, height, stride);
}

static void yuv422p_to_rgba32_C(const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, uint8_t* rgbaPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	planar_to_rgba32(yuv422p, yPtr, uPtr, vPtr, rgbaPtr, width, height, stride);
}

static void yuv444p_to_rgba32_C(const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, uint8_t* rgbaPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	planar_to_rgba32(yuv444p, yPtr, uPtr, vPtr, rgbaPtr, width, height, stride);
}

#define nv12_to_rgba32_indexU() 0
#define nv12_to_rgba32_indexV() 1

#define nv21_to_rgba32_indexU() 1
#define nv21_to_rgba32_indexV() 0

#define semiplanar_to_rgba32(name, yPtr, uvPtr, rgbaPtr, width, height, stride) { \
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found"); \
		const compv_uscalar_t strideRGBA = stride << 2; \
		int16_t Yp, Up, Vp; \
		for (compv_uscalar_t j = 0; j < height; ++j) { \
			for (compv_uscalar_t i = 0, k = 0; i < width; ++i, k += 4) { \
				Yp = (yPtr[i] - 16); \
				Up = (uvPtr[(i & -2) + name##_to_rgba32_indexU()] - 127); \
				Vp = (uvPtr[(i & -2) + name##_to_rgba32_indexV()] - 127); \
				COMPV_YUV_TO_RGBA(Yp, Up, Vp, rgbaPtr[k + 0], rgbaPtr[k + 1], rgbaPtr[k + 2], rgbaPtr[k + 3]); \
			} \
			rgbaPtr += strideRGBA; \
			yPtr += stride; \
			if (j & 1) { \
				uvPtr += stride; \
			} \
		} \
	}

static void nv12_to_rgba32_C(const uint8_t* yPtr, const uint8_t* uvPtr, uint8_t* rgbaPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	semiplanar_to_rgba32(nv12, yPtr, uvPtr, rgbaPtr, width, height, stride);
}

static void nv21_to_rgba32_C(const uint8_t* yPtr, const uint8_t* uvPtr, uint8_t* rgbaPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	semiplanar_to_rgba32(nv21, yPtr, uvPtr, rgbaPtr, width, height, stride);
}


#define yuyv422_to_rgba32_indexY() 0
#define yuyv422_to_rgba32_indexU() 1
#define yuyv422_to_rgba32_indexV() 3

#define uyvy422_to_rgba32_indexY() 1
#define uyvy422_to_rgba32_indexU() 0
#define uyvy422_to_rgba32_indexV() 2

#define packed_to_rgba32(name, yuyvPtr, rgbaPtr, width, height, stride) { \
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found"); \
		const compv_uscalar_t strideRGBA = (stride << 2); \
		const compv_uscalar_t strideYUV = (stride << 1); \
		const compv_uscalar_t maxWidth = (width << 1); \
		int16_t Yp, Up, Vp; \
		for (compv_uscalar_t j = 0; j < height; ++j) { \
			for (compv_uscalar_t i = 0, k = 0; i < maxWidth; i += 2, k += 4) { \
				Yp = (yuyvPtr[i + name##_to_rgba32_indexY()] - 16); \
				Up = (yuyvPtr[((i >> 2) << 2) + name##_to_rgba32_indexU()] - 127); \
				Vp = (yuyvPtr[((i >> 2) << 2) + name##_to_rgba32_indexV()] - 127); \
				COMPV_YUV_TO_RGBA(Yp, Up, Vp, rgbaPtr[k + 0], rgbaPtr[k + 1], rgbaPtr[k + 2], rgbaPtr[k + 3]); \
			} \
			rgbaPtr += strideRGBA; \
			yuyvPtr += strideYUV; \
		} \
	}

static void yuyv422_to_rgba32_C(const uint8_t* yuyvPtr, uint8_t* rgbaPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	packed_to_rgba32(yuyv422, yuyvPtr, rgbaPtr, width, height, stride);
}

static void uyvy422_to_rgba32_C(const uint8_t* yuyvPtr, uint8_t* rgbaPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	packed_to_rgba32(uyvy422, yuyvPtr, rgbaPtr, width, height, stride);
}

COMPV_NAMESPACE_END()
