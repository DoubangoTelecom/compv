/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/compv_image_conv_to_rgb24.h"
#include "compv/base/image/compv_image.h"
#include "compv/base/math/compv_math_utils.h"

COMPV_NAMESPACE_BEGIN()

static void yuv420p_to_rgb24_C(const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, uint8_t* outRgbaPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
static void nvx_to_rgb24_C(const uint8_t* yPtr, const uint8_t* uvPtr, uint8_t* outRgbaPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
static void yuyv422_to_rgb24_C(const uint8_t* yuyvPtr, uint8_t* outRgbaPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);

COMPV_ERROR_CODE CompVImageConvToRGB24::process(const CompVMatPtr& imageIn, CompVMatPtrPtr imageRGB24)
{
	CompVMatPtr imageOut = (*imageRGB24 == imageIn) ? nullptr : *imageRGB24;
	COMPV_CHECK_CODE_RETURN(CompVImage::newObj8u(&imageOut, COMPV_SUBTYPE_PIXELS_RGB24, imageIn->cols(), imageIn->rows(), imageIn->stride()));
	
	// Internal function, do not check input parameters (already done)
	switch (imageIn->subType()) {
		/*case COMPV_SUBTYPE_PIXELS_Y:*/
		case COMPV_SUBTYPE_PIXELS_NV12:
		/*case COMPV_SUBTYPE_PIXELS_NV21:*/
		case COMPV_SUBTYPE_PIXELS_YUV420P:
		/*case COMPV_SUBTYPE_PIXELS_YVU420P:*/
		/*case COMPV_SUBTYPE_PIXELS_YUV422P:*/
		/*case COMPV_SUBTYPE_PIXELS_YUV444P:*/ 
		case COMPV_SUBTYPE_PIXELS_YUYV422:
		/*case COMPV_SUBTYPE_PIXELS_UYVY422:*/ {
			// Planar Y -> RGB24
			if (imageIn->subType() == COMPV_SUBTYPE_PIXELS_YUV420P) {
				yuv420p_to_rgb24_C(
					imageIn->ptr<const uint8_t>(0, 0, COMPV_PLANE_Y),
					imageIn->ptr<const uint8_t>(0, 0, COMPV_PLANE_U),
					imageIn->ptr<const uint8_t>(0, 0, COMPV_PLANE_V),
					imageOut->ptr<uint8_t>(),
					imageOut->cols(),
					imageOut->rows(),
					imageOut->stride());
			}
			else if (imageIn->subType() == COMPV_SUBTYPE_PIXELS_NV12) {
				nvx_to_rgb24_C(
					imageIn->ptr<const uint8_t>(0, 0, COMPV_PLANE_Y),
					imageIn->ptr<const uint8_t>(0, 0, COMPV_PLANE_UV),
					imageOut->ptr<uint8_t>(),
					imageOut->cols(),
					imageOut->rows(),
					imageOut->stride());
			}
			else if (imageIn->subType() == COMPV_SUBTYPE_PIXELS_YUYV422) {
				yuyv422_to_rgb24_C(
					imageIn->ptr<const uint8_t>(),
					imageOut->ptr<uint8_t>(),
					imageOut->cols(),
					imageOut->rows(),
					imageOut->stride());
			}
			else {
				COMPV_ASSERT(false);
			}
	#if 0
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
	#endif
			break;
		}
		default: {
			COMPV_DEBUG_ERROR("%s -> RGB24 not supported", CompVGetSubtypeString(imageIn->subType()));
			return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
		}
	}


	*imageRGB24 = imageOut;

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

static void yuv420p_to_rgb24_C(const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, uint8_t* outRgbaPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
	const compv_uscalar_t strideRGB = stride * 3;
	const compv_uscalar_t strideY = stride;
	const compv_uscalar_t strideUV = ((stride + 1) >> 1);
	int16_t Yp, Up, Vp;
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0, k = 0; i < width; ++i, k+=3) {
			Yp = (yPtr[i] - 16);
			Up = (uPtr[i >> 1] - 127);
			Vp = (vPtr[i >> 1] - 127);
			COMPV_YUV_TO_RGB(Yp, Up, Vp, outRgbaPtr[k + 0], outRgbaPtr[k + 1], outRgbaPtr[k + 2]);
		}
		outRgbaPtr += strideRGB;
		yPtr += strideY;
		if (j & 1) {
			uPtr += strideUV;
			vPtr += strideUV;
		}
	}
}

// NV12 or NV21
static void nvx_to_rgb24_C(const uint8_t* yPtr, const uint8_t* uvPtr, uint8_t* outRgbaPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
	const compv_uscalar_t strideRGB = stride * 3;
	int16_t Yp, Up, Vp;
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0, k = 0; i < width; ++i, k += 3) {
			Yp = (yPtr[i] - 16);
			Up = (uvPtr[(i & -2) + 0] - 127);
			Vp = (uvPtr[(i & -2) + 1] - 127);
			COMPV_YUV_TO_RGB(Yp, Up, Vp, outRgbaPtr[k + 0], outRgbaPtr[k + 1], outRgbaPtr[k + 2]);
		}
		outRgbaPtr += strideRGB;
		yPtr += stride;
		if (j & 1) {
			uvPtr += stride;
		}
	}
}

static void yuyv422_to_rgb24_C(const uint8_t* yuyvPtr, uint8_t* outRgbaPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
	const compv_uscalar_t padSample = (stride - width);
	const compv_uscalar_t strideRGB = stride * 3;
	const compv_uscalar_t maxWidth = (width << 1);
	int16_t Yp, Up, Vp;
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0, k = 0; i < maxWidth; i += 2, k += 3) {
			Yp = (yuyvPtr[i] - 16);
			Up = (yuyvPtr[((i >> 2) << 2) + 1] - 127);
			Vp = (yuyvPtr[((i >> 2) << 2) + 3] - 127);
			COMPV_YUV_TO_RGB(Yp, Up, Vp, outRgbaPtr[k + 0], outRgbaPtr[k + 1], outRgbaPtr[k + 2]);
		}
		outRgbaPtr += strideRGB;
		if (j & 1) {
			yuyvPtr += (stride << 2);
		}
	}
}

COMPV_NAMESPACE_END()
