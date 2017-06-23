/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/compv_image_conv_hsv.h"
#include "compv/base/image/compv_image_conv_to_rgba32.h"
#include "compv/base/image/compv_image.h"
#include "compv/base/image/compv_image_conv_common.h"
#include "compv/base/math/compv_math_utils.h"
#include "compv/base/parallel/compv_parallel.h"

#include "compv/base/image/intrin/x86/compv_image_conv_hsv_intrin_sse2.h"

#define COMPV_THIS_CLASSNAME	"CompVImageConvToHSV"

COMPV_NAMESPACE_BEGIN()

template <typename xType>
static void rgbx_to_hsv_C(const uint8_t* rgbxPtr, uint8_t* hsvPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride, const int(*scales43)[256], const int(*scales255)[256]);

//
// CompVImageConvToHSV
//

COMPV_ERROR_CODE CompVImageConvToHSV::process(const CompVMatPtr& imageIn, CompVMatPtrPtr imageHSV)
{
	// Internal function, do not check input parameters (already done)

	CompVMatPtr imageRGBx;

	CompVMatPtr imageOut = (*imageHSV == imageIn) ? nullptr : *imageHSV;
	COMPV_CHECK_CODE_RETURN(CompVImage::newObj8u(&imageOut, COMPV_SUBTYPE_PIXELS_HSV, imageIn->cols(), imageIn->rows(), imageIn->stride()));

	// Convert to RGBA32 or RGB24
	switch (imageIn->subType()) {
	case COMPV_SUBTYPE_PIXELS_RGBA32:
	case COMPV_SUBTYPE_PIXELS_RGB24:
		imageRGBx = imageIn;
		break;
	default:
		COMPV_CHECK_CODE_RETURN(CompVImageConvToRGBA32::process(imageIn, &imageRGBx));
		break;
	}

	COMPV_CHECK_CODE_RETURN(CompVImageConvToHSV::rgbxToHsv(imageRGBx, imageOut));

	*imageHSV = imageOut;
	return COMPV_ERROR_CODE_S_OK;
}

// RGBx = RGBA32 or RGB24
COMPV_ERROR_CODE CompVImageConvToHSV::rgbxToHsv(const CompVMatPtr& imageRGBx, CompVMatPtr& imageHSV)
{
	// Internal function, do not check input parameters (already done)

	void(*rgbx_to_hsv)(const uint8_t* rgbxPtr, uint8_t* hsvPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride, const int(*scales43)[256], const int(*scales255)[256])
		= nullptr;

	switch (imageRGBx->subType()) {
	case COMPV_SUBTYPE_PIXELS_RGB24:
		rgbx_to_hsv = rgbx_to_hsv_C<compv_uint8x3_t>;
#if COMPV_ARCH_X86
		if (CompVCpu::isEnabled(kCpuFlagSSE2) && imageRGBx->isAlignedSSE() && imageHSV->isAlignedSSE()) {
			//COMPV_EXEC_IFDEF_INTRIN_X86(rgbx_to_hsv = CompVImageConvRgb24ToHsv_Intrin_SSE2);
		}
#elif COMPV_ARCH_ARM
#endif
		break;
	case COMPV_SUBTYPE_PIXELS_RGBA32:
		rgbx_to_hsv = rgbx_to_hsv_C<compv_uint8x4_t>;
#if COMPV_ARCH_X86
		if (CompVCpu::isEnabled(kCpuFlagSSE2) && imageRGBx->isAlignedSSE() && imageHSV->isAlignedSSE()) {
			//COMPV_EXEC_IFDEF_INTRIN_X86(rgbx_to_hsv = CompVImageConvRgba32ToHsv_Intrin_SSE2);
		}
#elif COMPV_ARCH_ARM
#endif
		break;
	default:
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "%s -> HSV not supported", CompVGetSubtypeString(imageRGBx->subType()));
		return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
	}

	COMPV_ALIGN_DEFAULT() static int __hsv_scales43[256];
	COMPV_ALIGN_DEFAULT() static int __hsv_scales255[256];
	static bool __hsv_init = false;

	const size_t widthInSamples = imageRGBx->cols();
	const size_t heightInSamples = imageRGBx->rows();
	const size_t strideInSamples = imageRGBx->stride();

	const uint8_t* rgbxPtr = imageRGBx->ptr<const uint8_t>();
	uint8_t* hsvPtr = imageHSV->ptr<uint8_t>();

	CompVThreadDispatcherPtr threadDisp = CompVParallel::threadDispatcher();
	size_t maxThreads = threadDisp ? static_cast<size_t>(threadDisp->threadsCount()) : 0;

	// Initialize tables
	if (!__hsv_init) {
		__hsv_scales43[0] = 0;
		__hsv_scales255[0] = 0;
		int scale;
		for (int b = 1; b < 256; ++b) {
			scale = COMPV_MATH_ROUNDFU_2_NEAREST_INT((65535.f / static_cast<compv_float32_t>(b)), int); // 65535 = 0xFFFF (16 bits)
			__hsv_scales43[b] = 43 * scale;
			__hsv_scales255[b] = 255 * scale;
		}
		__hsv_init = true;
	}

	// Compute number of threads
	const size_t threadsCount = (threadDisp && !threadDisp->isMotherOfTheCurrentThread())
		? CompVThreadDispatcher::guessNumThreadsDividingAcrossY(widthInSamples, heightInSamples, maxThreads, COMPV_IMAGE_CONV_MIN_SAMPLES_PER_THREAD)
		: 1;

	// Process
	if (threadsCount > 1) {
		
		const size_t heights = (heightInSamples / threadsCount);
		const size_t lastHeight = heights + (heightInSamples % threadsCount);
		const size_t rgbxPaddingInBytes = (imageRGBx->strideInBytes() * heights);
		const size_t hsvPaddingInBytes = (imageHSV->strideInBytes() * heights);
		CompVAsyncTaskIds taskIds;
		taskIds.reserve(threadsCount);
		auto funcPtr = [&](const uint8_t* rgbxPtr_, uint8_t* hsvPtr_, compv_uscalar_t height_) -> void {
			rgbx_to_hsv(rgbxPtr_, hsvPtr_, widthInSamples, height_, strideInSamples, &__hsv_scales43, &__hsv_scales255);
		};

		for (size_t threadIdx = 0; threadIdx < threadsCount - 1; ++threadIdx) {
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtr, rgbxPtr, hsvPtr, heights), taskIds), "Dispatching task failed");
			rgbxPtr += rgbxPaddingInBytes;
			hsvPtr += hsvPaddingInBytes;
		}
		if (lastHeight > 0) {
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtr, rgbxPtr, hsvPtr, lastHeight), taskIds), "Dispatching task failed");
		}
		COMPV_CHECK_CODE_RETURN(threadDisp->wait(taskIds), "Failed to wait for tasks execution");
	}
	else {
		rgbx_to_hsv(rgbxPtr, hsvPtr, widthInSamples, heightInSamples, strideInSamples, &__hsv_scales43, &__hsv_scales255);
	}

	return COMPV_ERROR_CODE_S_OK;
}

template <typename xType>
static void rgbx_to_hsv_C(const uint8_t* rgbxPtr, uint8_t* hsvPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride, const int(*scales43)[256], const int(*scales255)[256])
{
	// Optimization: use SSE and CMOV to suppress branches
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");

	size_t i, j;
	const xType* rgbxPtr_ = reinterpret_cast<const xType*>(rgbxPtr);
	compv_uint8x3_t* hsvPtr_ = reinterpret_cast<compv_uint8x3_t*>(hsvPtr);

	int minVal, maxVal, minus, r, g, b;
	int diff, m0, m1, m2;
	for (j = 0; j <height; ++j) {
		for (i = 0; i < width; ++i) {
			const xType& rgbx = rgbxPtr_[i];
			compv_uint8x3_t& hsv = hsvPtr_[i];
			r = rgbx[0], g = rgbx[1], b = rgbx[2];

			minVal = COMPV_MATH_MIN_INT(g, b);
			minVal = COMPV_MATH_MIN_INT(r, minVal);
			maxVal = COMPV_MATH_MAX_INT(g, b);
			maxVal = COMPV_MATH_MAX_INT(r, maxVal); // ASM: SSE / NEON

			m0 = -(maxVal == r);
			m1 = -(maxVal == g) & ~m0;
			m2 = ~(m0 | m1);
			diff = ((g - b) & m0) | ((b - r) & m1) | ((r - g) & m2);
			minus = maxVal - minVal;
			hsv[0] = ((diff * (*scales43)[minus]) >> 16) + ((85 & m1) | (171 & m2));
			hsv[1] = ((minus * (*scales255)[maxVal]) >> 16);
			hsv[2] = (maxVal);
		}
		rgbxPtr_ += stride;
		hsvPtr_ += stride;
	}
}

COMPV_NAMESPACE_END()
