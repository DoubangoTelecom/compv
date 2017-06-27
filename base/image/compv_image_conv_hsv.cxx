/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/compv_image_conv_hsv.h"
#include "compv/base/image/compv_image_conv_to_rgbx.h"
#include "compv/base/image/compv_image.h"
#include "compv/base/image/compv_image_conv_common.h"
#include "compv/base/math/compv_math_utils.h"
#include "compv/base/parallel/compv_parallel.h"

#include "compv/base/image/intrin/x86/compv_image_conv_hsv_intrin_ssse3.h"

#define COMPV_THIS_CLASSNAME	"CompVImageConvToHSV"

COMPV_NAMESPACE_BEGIN()

template <typename xType>
static void rgbx_to_hsv_C(const uint8_t* rgbxPtr, uint8_t* hsvPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride, const compv_float32_t(*scales43)[256], const compv_float32_t(*scales255)[256]);

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
		// On X86: "RGBA32 -> HSV" is faster than "RGB24 -> HSV" because of (de-)interleaving RGB24 which is slow
		// On ARM: "RGB24 -> HSV" is faster than "RGBA32 -> HSV" because less data and more cache-friendly. No (de-)interleaving issues, thanks to vld3.u8 and vst3.u8.
		// Another good reason to use RGB24: "input === output" -> Cache-friendly
		// Another good reason to use RGB24: there is very faaast ASM code for NEON, SSSE3 and AVX2
#if COMPV_ARCH_ARM || 1
		COMPV_CHECK_CODE_RETURN(CompVImageConvToRGBx::process(imageIn, COMPV_SUBTYPE_PIXELS_RGB24, &imageOut));
		// Call 'newObj8u' to change the subtype, no memory will be allocated as HSV and RGB24 have the same size.
		const void* oldPtr = imageOut->ptr<const void*>();
		COMPV_CHECK_CODE_RETURN(CompVImage::newObj8u(&imageOut, COMPV_SUBTYPE_PIXELS_HSV, imageIn->cols(), imageIn->rows(), imageIn->stride()));
		COMPV_CHECK_EXP_RETURN(oldPtr != imageOut->ptr<const void*>(), COMPV_ERROR_CODE_E_INVALID_CALL, "Data reallocation not expected to change the pointer address");
		imageRGBx = imageOut;
#else
		COMPV_CHECK_CODE_RETURN(CompVImageConvToRGBx::process(imageIn, COMPV_SUBTYPE_PIXELS_RGBA32, &imageRGBx));
#endif
		break;
	}

	//!\\ Important: 'imageRGBx' and 'imageOut' are the same data when intermediate format is RGB24. No data
	// override at conversion because RGB24 and HSV have the same size. Not the case for RGBA32.
	COMPV_CHECK_CODE_RETURN(CompVImageConvToHSV::rgbxToHsv(imageRGBx, imageOut));

	*imageHSV = imageOut;
	return COMPV_ERROR_CODE_S_OK;
}

// RGBx = RGBA32 or RGB24
COMPV_ERROR_CODE CompVImageConvToHSV::rgbxToHsv(const CompVMatPtr& imageRGBx, CompVMatPtr& imageHSV)
{
	// Internal function, do not check input parameters (already done)

	void(*rgbx_to_hsv)(const uint8_t* rgbxPtr, uint8_t* hsvPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride, const compv_float32_t(*scales43)[256], const compv_float32_t(*scales255)[256])
		= nullptr;

	switch (imageRGBx->subType()) {
	case COMPV_SUBTYPE_PIXELS_RGB24:
	case COMPV_SUBTYPE_PIXELS_HSV: // See above, this is a hack use to overwrite the memory (input == output)
		rgbx_to_hsv = rgbx_to_hsv_C<compv_uint8x3_t>;
#if COMPV_ARCH_X86
		if (CompVCpu::isEnabled(kCpuFlagSSSE3) && imageRGBx->isAlignedSSE() && imageHSV->isAlignedSSE()) {
			COMPV_EXEC_IFDEF_INTRIN_X86(rgbx_to_hsv = CompVImageConvRgb24ToHsv_Intrin_SSSE3);
		}
#elif COMPV_ARCH_ARM
#endif
		break;
	case COMPV_SUBTYPE_PIXELS_RGBA32:
		rgbx_to_hsv = rgbx_to_hsv_C<compv_uint8x4_t>;
#if COMPV_ARCH_X86
		if (CompVCpu::isEnabled(kCpuFlagSSSE3) && imageRGBx->isAlignedSSE() && imageHSV->isAlignedSSE()) {
			//COMPV_EXEC_IFDEF_INTRIN_X86(rgbx_to_hsv = CompVImageConvRgba32ToHsv_Intrin_SSSE3);
		}
#elif COMPV_ARCH_ARM
#endif
		break;
	default:
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "%s -> HSV not supported", CompVGetSubtypeString(imageRGBx->subType()));
		return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
	}

	COMPV_ALIGN_DEFAULT() static compv_float32_t __hsv_scales43[256];
	COMPV_ALIGN_DEFAULT() static compv_float32_t __hsv_scales255[256];
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
		__hsv_scales43[0] = 0.f;
		__hsv_scales255[0] = 0.f;
		compv_float32_t scale;
		for (int b = 1; b < 256; ++b) {
			scale = 1.f / static_cast<compv_float32_t>(b);
			__hsv_scales43[b] = 43.f * scale;
			__hsv_scales255[b] = 255.f * scale;
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
static void rgbx_to_hsv_C(const uint8_t* rgbxPtr, uint8_t* hsvPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride, const compv_float32_t(*scales43)[256], const compv_float32_t(*scales255)[256])
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
#if 1
	size_t i, j;
	const xType* rgbxPtr_ = reinterpret_cast<const xType*>(rgbxPtr);
	compv_uint8x3_t* hsvPtr_ = reinterpret_cast<compv_uint8x3_t*>(hsvPtr);
	int minVal, maxVal, minus, r, g, b;
	int diff;
	compv_float32_t s255, s43;
	for (j = 0; j <height; ++j) {
		for (i = 0; i < width; ++i) {
			const xType& rgbx = rgbxPtr_[i];
			compv_uint8x3_t& hsv = hsvPtr_[i];
			r = rgbx[0], g = rgbx[1], b = rgbx[2];

			minVal = COMPV_MATH_MIN_INT(g, b);
			minVal = COMPV_MATH_MIN_INT(r, minVal);
			maxVal = COMPV_MATH_MAX_INT(g, b);
			maxVal = COMPV_MATH_MAX_INT(r, maxVal);

			diff = (maxVal == r) ? (g - b) : ((maxVal == g) ? (b - r) : (r - g)); // ASM: CMOV
			minus = maxVal - minVal;
			s43 = diff * (*scales43)[minus];
			s255 = minus * (*scales255)[maxVal];

			hsv[0] = COMPV_MATH_ROUNDF_2_NEAREST_INT(s43, uint8_t) +
				((maxVal == r) ? 0 : ((maxVal == g) ? 85 : 171)); // ASM: CMOV

			hsv[1] = COMPV_MATH_ROUNDF_2_NEAREST_INT(s255, uint8_t);

			hsv[2] = static_cast<uint8_t>(maxVal);
		}
		rgbxPtr_ += stride;
		hsvPtr_ += stride;
}
#else
	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Branchless code for SIMD implementations (SSE, AVX and ARM NEON)");

	size_t i, j;
	const xType* rgbxPtr_ = reinterpret_cast<const xType*>(rgbxPtr);
	compv_uint8x3_t* hsvPtr_ = reinterpret_cast<compv_uint8x3_t*>(hsvPtr);
	int minVal, maxVal, minus, r, g, b;
	int diff, m0, m1, m2;
	compv_float32_t s255, s43;
	for (j = 0; j <height; ++j) {
		for (i = 0; i < width; ++i) {
			const xType& rgbx = rgbxPtr_[i];
			compv_uint8x3_t& hsv = hsvPtr_[i];
			r = rgbx[0], g = rgbx[1], b = rgbx[2];

			minVal = COMPV_MATH_MIN_INT(g, b);
			minVal = COMPV_MATH_MIN_INT(r, minVal);
			maxVal = COMPV_MATH_MAX_INT(g, b);
			maxVal = COMPV_MATH_MAX_INT(r, maxVal);

			m0 = -(maxVal == r); // (maxVal == r) ? 0xff : 0x00;
			m1 = -(maxVal == g) & ~m0; // ((maxVal == r) ? 0xff : 0x00) & ~m0
			m2 = ~(m0 | m1);
			diff = ((g - b) & m0) | ((b - r) & m1) | ((r - g) & m2);
			minus = maxVal - minVal;
			s43 = diff * (*scales43)[minus];
			s255 = minus * (*scales255)[maxVal];
			hsv[0] = COMPV_MATH_ROUNDF_2_NEAREST_INT(s43, uint8_t)
				+ ((85 & m1) | (171 & m2));
			hsv[1] = COMPV_MATH_ROUNDF_2_NEAREST_INT(s255, uint8_t);
			hsv[2] = static_cast<uint8_t>(maxVal);
		}
		rgbxPtr_ += stride;
		hsvPtr_ += stride;
	}
#endif
}

COMPV_NAMESPACE_END()
