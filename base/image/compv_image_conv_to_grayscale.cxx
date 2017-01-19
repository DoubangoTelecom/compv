/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/compv_image_conv_to_grayscale.h"
#include "compv/base/image/compv_image.h"
#include "compv/base/image/compv_image_conv_rgbfamily.h"
#include "compv/base/image/compv_image_conv_common.h"
#include "compv/base/parallel/compv_parallel.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_cpu.h"

#include "compv/base/image/intrin/x86/compv_image_conv_grayscale_intrin_ssse3.h"
#include "compv/base/image/intrin/arm/compv_image_conv_grayscale_intrin_neon.h"

#define COMPV_THIS_CLASSNAME	"CompVImageConvToGrayscale"

COMPV_NAMESPACE_BEGIN()

#if COMPV_ASM
#	if COMPV_ARCH_X86
	COMPV_EXTERNC void CompVImageConvYuyv422_to_y_Asm_X86_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* yuv422Ptr, COMPV_ALIGNED(SSE) uint8_t* outYPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
	COMPV_EXTERNC void CompVImageConvUyvy422_to_y_Asm_X86_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* yuv422Ptr, COMPV_ALIGNED(SSE) uint8_t* outYPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
#	elif COMPV_ARCH_ARM
	COMPV_EXTERNC void CompVImageConvYuyv422_to_y_Asm_NEON32(COMPV_ALIGNED(NEON) const uint8_t* yuv422Ptr, COMPV_ALIGNED(NEON) uint8_t* outYPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
	COMPV_EXTERNC void CompVImageConvUyvy422_to_y_Asm_NEON32(COMPV_ALIGNED(NEON) const uint8_t* yuv422Ptr, COMPV_ALIGNED(NEON) uint8_t* outYPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
#	endif /* COMPV_ARCH_XXX */
#endif /* COMPV_ASM */

COMPV_ERROR_CODE CompVImageConvToGrayscale::process(const CompVMatPtr& imageIn, CompVMatPtrPtr imageGray)
{
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
	case COMPV_SUBTYPE_PIXELS_BGR565BE: {
		// RGBfamily -> graysacle
		CompVMatPtr imageOut = (imageIn == *imageGray) ? nullptr : *imageGray; // Input must not be equal to output
		COMPV_CHECK_CODE_RETURN(CompVImageConvToGrayscale::rgbfamily(imageIn, &imageOut), "Conversion (RGBFamily -> Grayscale) failed");
		*imageGray = imageOut;
		return COMPV_ERROR_CODE_S_OK;
	}
	case COMPV_SUBTYPE_PIXELS_Y:
	case COMPV_SUBTYPE_PIXELS_NV12:
	case COMPV_SUBTYPE_PIXELS_NV21:
	case COMPV_SUBTYPE_PIXELS_YUV420P:
	case COMPV_SUBTYPE_PIXELS_YVU420P:
	case COMPV_SUBTYPE_PIXELS_YUV422P:
	case COMPV_SUBTYPE_PIXELS_YUV444P: {
		// Planar Y -> Graysacle
		// This is very fast as we'll just reshape the data or make a copy. This is why we recommend using planar YUV for camera output.
		if (imageIn == *imageGray) {
			if (imageIn->subType() != COMPV_SUBTYPE_PIXELS_Y) { // When input already equal to gray then, do nothing
				// 'newObj8u' will not realloc the data when the requested size is less than the original one.
				// sizeof(Gray) is always less than input format which means 'newObj8u' will just reshape the input data.
				COMPV_CHECK_CODE_RETURN(CompVImage::newObj8u(imageGray, COMPV_SUBTYPE_PIXELS_Y, imageIn->cols(COMPV_PLANE_Y), imageIn->rows(COMPV_PLANE_Y), imageIn->stride(COMPV_PLANE_Y)));
			}
		}
		else {
			COMPV_CHECK_CODE_RETURN(CompVImage::wrap(COMPV_SUBTYPE_PIXELS_Y,
				imageIn->ptr<uint8_t>(0, 0, COMPV_PLANE_Y), imageIn->cols(COMPV_PLANE_Y), imageIn->rows(COMPV_PLANE_Y), imageIn->stride(COMPV_PLANE_Y),
				imageGray));
		}
		return COMPV_ERROR_CODE_S_OK;
	}
	case COMPV_SUBTYPE_PIXELS_YUYV422:
	case COMPV_SUBTYPE_PIXELS_UYVY422: {
		// Packed (non-Planar) YUV422 -> Grayscale
		COMPV_CHECK_CODE_RETURN(CompVImageConvToGrayscale::yuv422family(imageIn, imageGray));
		return COMPV_ERROR_CODE_S_OK;
	}
	default:
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Chroma conversion not supported: %s -> COMPV_SUBTYPE_PIXELS_Y", CompVGetSubtypeString(imageIn->subType()));
		return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
	}
}

static void yuyv422_to_y_C(const uint8_t* yuyv422Ptr, uint8_t* outYPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
static void uyvy422_to_y_c(const uint8_t* uyvy422Ptr, uint8_t* outYPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);

COMPV_ERROR_CODE CompVImageConvToGrayscale::yuv422family(const CompVMatPtr& imageYUV422family, CompVMatPtrPtr imageGray)
{
	void(*yuv422family_to_y)(const uint8_t* yuv422Ptr, uint8_t* outYPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride) = NULL;
	switch (imageYUV422family->subType()) {
	case COMPV_SUBTYPE_PIXELS_YUYV422:
		yuv422family_to_y = yuyv422_to_y_C;
#if COMPV_ARCH_X86
		if (CompVCpu::isEnabled(kCpuFlagSSE2) && imageYUV422family->isAlignedSSE()) {
			COMPV_EXEC_IFDEF_INTRIN_X86(yuv422family_to_y = CompVImageConvYuyv422_to_y_Intrin_SSSE3);
			COMPV_EXEC_IFDEF_ASM_X86(yuv422family_to_y = CompVImageConvYuyv422_to_y_Asm_X86_SSSE3);
		}
#elif COMPV_ARCH_ARM
		if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && imageYUV422family->isAlignedNEON()) {
			COMPV_EXEC_IFDEF_INTRIN_ARM(yuv422family_to_y = CompVImageConvYuyv422_to_y_Intrin_NEON);
			COMPV_EXEC_IFDEF_ASM_ARM(yuv422family_to_y = CompVImageConvYuyv422_to_y_Asm_NEON32);
		}
#endif
		break;
	case COMPV_SUBTYPE_PIXELS_UYVY422:
		yuv422family_to_y = uyvy422_to_y_c;
#if COMPV_ARCH_X86
		if (CompVCpu::isEnabled(kCpuFlagSSE2) && imageYUV422family->isAlignedSSE()) {
			COMPV_EXEC_IFDEF_INTRIN_X86(yuv422family_to_y = CompVImageConvUyvy422_to_y_Intrin_SSSE3);
			COMPV_EXEC_IFDEF_ASM_X86(yuv422family_to_y = CompVImageConvUyvy422_to_y_Asm_X86_SSSE3);
		}
#elif COMPV_ARCH_ARM
		if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && imageYUV422family->isAlignedNEON()) {
			COMPV_EXEC_IFDEF_INTRIN_ARM(yuv422family_to_y = CompVImageConvUyvy422_to_y_Intrin_NEON);
			COMPV_EXEC_IFDEF_ASM_ARM(yuv422family_to_y = CompVImageConvUyvy422_to_y_Asm_NEON32);
		}
#endif
		break;
	default:
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Failed to find YUV422family conversion: %s -> COMPV_SUBTYPE_PIXELS_Y", CompVGetSubtypeString(imageYUV422family->subType()));
		return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
	}
	size_t threadsCount;
	CompVAsyncTaskIds taskIds;
	CompVThreadDispatcherPtr threadDisp = CompVParallel::threadDispatcher();
	size_t maxThreads = threadDisp ? static_cast<size_t>(threadDisp->threadsCount()) : 0;
	const size_t width = imageYUV422family->cols();
	const size_t height = imageYUV422family->rows();
	const size_t stride = imageYUV422family->stride();
	COMPV_CHECK_CODE_RETURN(CompVImage::newObj8u(imageGray, COMPV_SUBTYPE_PIXELS_Y, width, height, stride));
	uint8_t* yPtr = (*imageGray)->ptr<uint8_t>(0, 0, COMPV_PLANE_Y);
	const uint8_t* yuv422Ptr = imageYUV422family->ptr<const uint8_t>();
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;

	// Compute number of threads
	threadsCount = (threadDisp && !threadDisp->isMotherOfTheCurrentThread())
		? CompVThreadDispatcher::guessNumThreadsDividingAcrossY(stride, height, maxThreads, (COMPV_IMAGE_CONV_MIN_SAMPLES_PER_THREAD << 1)) // '<<1' because conversion is memory read only, not CPU intensive like RGB -> Y
		: 1;

	if (threadsCount > 1) {
		size_t bytesPerPixel;
		COMPV_CHECK_CODE_RETURN(CompVImageUtils::bitsCountForPixelFormat(imageYUV422family->subType(), &bytesPerPixel));
		bytesPerPixel >>= 3;
		size_t yuv422Idx = 0, YIdx = 0;
		taskIds.reserve(threadsCount);
		auto funcPtr = [&](const uint8_t* yuv422Ptr, uint8_t* outYPtr, compv_uscalar_t height) -> void {
			yuv422family_to_y(yuv422Ptr, outYPtr, width, height, stride);
		};
		size_t heights = (height / threadsCount);
		size_t lastHeight = height - ((threadsCount - 1) * heights);
		for (size_t threadIdx = 0; threadIdx < threadsCount; ++threadIdx) {
			COMPV_CHECK_CODE_BAIL(err = threadDisp->invoke(std::bind(funcPtr, (yuv422Ptr + yuv422Idx), (yPtr + YIdx), static_cast<compv_uscalar_t>((threadIdx == (threadsCount - 1) ? lastHeight : heights))), taskIds), "Dispatching task failed");
			yuv422Idx += (heights * stride) * bytesPerPixel;
			YIdx += (heights * stride);
		}
		COMPV_CHECK_CODE_BAIL(err = threadDisp->wait(taskIds), "Failed to wait for tasks execution");
	}
	else {
		yuv422family_to_y(yuv422Ptr, yPtr, width, height, stride);
	}
bail:
	return err;
}

COMPV_ERROR_CODE CompVImageConvToGrayscale::rgbfamily(const CompVMatPtr& imageRGBfamily, CompVMatPtrPtr imageGray)
{
	// Private function, do not check input parameters (already done)
	void(*rgbfamily_to_y)(const uint8_t* rgbPtr, uint8_t* outYPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride) = NULL;
	switch (imageRGBfamily->subType()) {
	case COMPV_SUBTYPE_PIXELS_RGBA32:
		rgbfamily_to_y = CompVImageConvRGBfamily::rgba32_to_y;
		break;
	case COMPV_SUBTYPE_PIXELS_ARGB32:
		rgbfamily_to_y = CompVImageConvRGBfamily::argb32_to_y;
		break;
	case COMPV_SUBTYPE_PIXELS_BGRA32:
		rgbfamily_to_y = CompVImageConvRGBfamily::bgra32_to_y;
		break;
	case COMPV_SUBTYPE_PIXELS_RGB24:
		rgbfamily_to_y = CompVImageConvRGBfamily::rgb24_to_y;
		break;
	case COMPV_SUBTYPE_PIXELS_BGR24:
		rgbfamily_to_y = CompVImageConvRGBfamily::bgr24_to_y;
		break;
	case COMPV_SUBTYPE_PIXELS_RGB565LE:
		rgbfamily_to_y = CompVImageConvRGBfamily::rgb565le_to_y;
		break;
	case COMPV_SUBTYPE_PIXELS_RGB565BE:
		rgbfamily_to_y = CompVImageConvRGBfamily::rgb565be_to_y;
		break;
	case COMPV_SUBTYPE_PIXELS_BGR565LE:
		rgbfamily_to_y = CompVImageConvRGBfamily::bgr565le_to_y;
		break;
	case COMPV_SUBTYPE_PIXELS_BGR565BE:
		rgbfamily_to_y = CompVImageConvRGBfamily::bgr565be_to_y;
		break;
	default:
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Failed to find RGBfamily conversion: %s -> COMPV_SUBTYPE_PIXELS_Y", CompVGetSubtypeString(imageRGBfamily->subType()));
		return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
	}
	
	const size_t width = imageRGBfamily->cols();
	const size_t height = imageRGBfamily->rows();
	const size_t stride = imageRGBfamily->stride(); // in and out images must have same stride
	size_t strideY;
	const uint8_t* rgbPtr = imageRGBfamily->ptr<const uint8_t>();
	uint8_t *yPtr;
	size_t threadsCount;
	CompVAsyncTaskIds taskIds;
	CompVThreadDispatcherPtr threadDisp = CompVParallel::threadDispatcher();
	size_t maxThreads = threadDisp ? static_cast<size_t>(threadDisp->threadsCount()) : 0;
	COMPV_ERROR_CODE err;

	// Create the output YUV444P image (nop operation if image already allocated)
	COMPV_CHECK_CODE_BAIL(err = CompVImage::newObj8u(imageGray, COMPV_SUBTYPE_PIXELS_Y, width, height, stride));
	yPtr = (*imageGray)->ptr<uint8_t>(0, 0, COMPV_PLANE_Y);
	strideY = (*imageGray)->stride(COMPV_PLANE_Y); // same as value 'stride'

	// Compute number of threads
	threadsCount = (threadDisp && !threadDisp->isMotherOfTheCurrentThread())
		? CompVThreadDispatcher::guessNumThreadsDividingAcrossY(stride, height, maxThreads, COMPV_IMAGE_CONV_MIN_SAMPLES_PER_THREAD)
		: 1;

	if (threadsCount > 1) {
		size_t bytesPerPixel;
		COMPV_CHECK_CODE_RETURN(CompVImageUtils::bitsCountForPixelFormat(imageRGBfamily->subType(), &bytesPerPixel));
		bytesPerPixel >>= 3;
		size_t rgbIdx = 0, YIdx = 0;
		taskIds.reserve(threadsCount);
		auto funcPtr = [&](const uint8_t* rgbPtr, uint8_t* outYPtr, compv_uscalar_t height) -> void {
			rgbfamily_to_y(rgbPtr, outYPtr, width, height, strideY);
		};
		size_t heights = (height / threadsCount);
		size_t lastHeight = height - ((threadsCount - 1) * heights);
		for (size_t threadIdx = 0; threadIdx < threadsCount; ++threadIdx) {
			COMPV_CHECK_CODE_BAIL(err = threadDisp->invoke(std::bind(funcPtr, (rgbPtr + rgbIdx), (yPtr + YIdx), static_cast<compv_uscalar_t>((threadIdx == (threadsCount - 1) ? lastHeight : heights))), taskIds), "Dispatching task failed");
			rgbIdx += (heights * stride) * bytesPerPixel;
			YIdx += (heights * strideY);
		}
		COMPV_CHECK_CODE_BAIL(err = threadDisp->wait(taskIds), "Failed to wait for tasks execution");
	}
	else {
		rgbfamily_to_y(rgbPtr, yPtr, width, height, stride);
	}
bail:
	return err;
}

#define yuyv422_sample_to_y() *outYPtr++ = *yuv422Ptr
#define uyvy422_sample_to_y() *outYPtr++ = *(yuv422Ptr + 1)
#define yuv422family_to_y(family) \
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found"); \
	compv_uscalar_t i, j, padY = (stride - width), padYuv422 = padY << 1; \
	for (j = 0; j < height; ++j) { \
		for (i = 0; i < width; ++i) { \
			family##_sample_to_y(); \
			yuv422Ptr += 2; \
		} \
		yuv422Ptr += padYuv422; \
		outYPtr += padY; \
	}

static void yuyv422_to_y_C(const uint8_t* yuv422Ptr, uint8_t* outYPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	yuv422family_to_y(yuyv422);
}

static void uyvy422_to_y_c(const uint8_t* yuv422Ptr, uint8_t* outYPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	yuv422family_to_y(uyvy422);
}

COMPV_NAMESPACE_END()
