/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/compv_image_remap.h"
#include "compv/base/image/compv_image.h"
#include "compv/base/image/compv_image_scale_bicubic.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/math/compv_math_cast.h"
#include "compv/base/parallel/compv_parallel.h"
#include "compv/base/compv_base.h"
#include "compv/base/compv_generic_invoke.h"

#define COMPV_THIS_CLASSNAME	"CompVImageRemap"

#define COMPV_IMAGE_REMAP_BILINEAR_SAMPLES_PER_THREAD (40 * 40) // CPU-friendly
#define COMPV_IMAGE_REMAP_NEAREST_SAMPLES_PER_THREAD (50 * 50) // CPU-friendly
#define COMPV_IMAGE_REMAP_BICUBIC_SAMPLES_PER_THREAD (10 * 10) // CPU-unfriendly

COMPV_NAMESPACE_BEGIN()

//
//	CompVImageRemapGeneric
//
template<class T>
class CompVImageRemapGeneric {
private:
	static COMPV_ERROR_CODE process_nearest(const CompVMatPtr& input, CompVMatPtr& output, const CompVMatPtr& map, const CompVRectFloat32 inputROI)
	{
		// Internal function, no need to check input parameters. Up to the caller.

		const T roi_left = static_cast<T>(inputROI.left);
		const T roi_right = static_cast<T>(inputROI.right);
		const T roi_top = static_cast<T>(inputROI.top);
		const T roi_bottom = static_cast<T>(inputROI.bottom);
		const T* mapXPtr = map->ptr<const T>(0);
		const T* mapYPtr = map->ptr<const T>(1);
		const size_t outputWidth = output->cols();
		const size_t outputHeight = output->rows();
		const size_t  outputStride = output->stride();

		auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
			uint8_t* outputPtr = output->ptr<uint8_t>(ystart);
			size_t i, j, k;
			for (j = ystart, k = (ystart * outputWidth); j < yend; ++j) {
				for (i = 0; i < outputWidth; ++i, ++k) {
					const T x = mapXPtr[k];
					const T y = mapYPtr[k];
					if (x < roi_left || x > roi_right || y < roi_top || y > roi_bottom) {
						outputPtr[i] = 0; // TODO(dmi): or mean
					}
					else {
						outputPtr[i] = *input->ptr<const uint8_t>(
							COMPV_MATH_ROUNDFU_2_NEAREST_INT(y, size_t),
							COMPV_MATH_ROUNDFU_2_NEAREST_INT(x, size_t)
							);
					}
				}
				outputPtr += outputStride;
			}
			return COMPV_ERROR_CODE_S_OK;
		};
		COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
			funcPtr,
			outputWidth,
			outputHeight,
			COMPV_IMAGE_REMAP_NEAREST_SAMPLES_PER_THREAD
		));

		return COMPV_ERROR_CODE_S_OK;
	}
	
	static COMPV_ERROR_CODE process_bilinear(const CompVMatPtr& input, CompVMatPtr& output, const CompVMatPtr& map, const CompVRectFloat32 inputROI)
	{
		// Private function, no need to check input parameters. Up to the caller.

		const T roi_left = static_cast<T>(inputROI.left);
		const T roi_right = static_cast<T>(inputROI.right);
		const T roi_top = static_cast<T>(inputROI.top);
		const T roi_bottom = static_cast<T>(inputROI.bottom);
		const T* mapXPtr = map->ptr<const T>(0);
		const T* mapYPtr = map->ptr<const T>(1);
		const size_t outputWidth = output->cols();
		const size_t outputHeight = output->rows();
		const size_t  outputStride = output->stride();

		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");

		auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
			// Bilinear filtering: https://en.wikipedia.org/wiki/Bilinear_interpolation#Unit_square
			uint8_t* outputPtr = output->ptr<uint8_t>(ystart);
			const uint8_t* inputPtr = input->ptr<uint8_t>();
			const size_t stride = input->stride();
			const int inWidthMinus1 = static_cast<int>(input->cols() - 1);
			const int inHeightMinus1 = static_cast<int>(input->rows() - 1);
			size_t i, j, k;
			for (j = ystart, k = (ystart * outputWidth); j < yend; ++j) {
				for (i = 0; i < outputWidth; ++i, ++k) {
					const T& x = mapXPtr[k];
					const T& y = mapYPtr[k];
					if (x < roi_left || x > roi_right || y < roi_top || y > roi_bottom) {
						outputPtr[i] = 0; // TODO(dmi): or mean
					}
					else {
#if 0
						const int x1 = static_cast<int>(x);
						const int x2 = static_cast<int>(x + 1.f);
						const T xfractpart = x - x1;
						const T one_minus_xfractpart = 1.f - xfractpart;
						const int y1 = static_cast<int>(y);
						const int y2 = static_cast<int>(y + 1.f);
						const T yfractpart = y - y1;
						const T one_minus_yfractpart = 1.f - yfractpart;
						const uint8_t* inputY1 = &inputPtr[y1 * stride];
						const uint8_t* inputY2 = &inputPtr[y2 * stride];
						outputPtr[i] = static_cast<uint8_t>(
							(inputY1[x1] * one_minus_yfractpart * one_minus_xfractpart)
							+ (inputY1[x2] * one_minus_yfractpart * xfractpart)
							+ (inputY2[x1] * yfractpart * one_minus_xfractpart)
							+ (inputY2[x2] * yfractpart * xfractpart)
						);
#else // Next code is faster (less multiplications)
						const int x1 = static_cast<int>(x);
						const int x2 = std::min(static_cast<int>(x + 1.f), inWidthMinus1);
						const T xfractpart = x - x1;
						const int y1 = static_cast<int>(y);
						const int y2 = std::min(static_cast<int>(y + 1.f), inHeightMinus1);
						const T yfractpart = y - y1;
						const T xyfractpart = (xfractpart * yfractpart);
						const uint8_t* inputY1 = &inputPtr[y1 * stride];
						const uint8_t* inputY2 = &inputPtr[y2 * stride];
						outputPtr[i] = static_cast<uint8_t>(
							(inputY1[x1] * (1.f - xfractpart - yfractpart + xyfractpart))
							+ (inputY1[x2] * (xfractpart - xyfractpart))
							+ (inputY2[x1] * (yfractpart - xyfractpart))
							+ (inputY2[x2] * xyfractpart)
							);
#endif
					}
				}
				outputPtr += outputStride;
			}
			return COMPV_ERROR_CODE_S_OK;
		};
		COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
			funcPtr,
			outputWidth,
			outputHeight,
			COMPV_IMAGE_REMAP_BILINEAR_SAMPLES_PER_THREAD
		));

		return COMPV_ERROR_CODE_S_OK;
	}

	static COMPV_ERROR_CODE process_bicubic(const CompVMatPtr& input, CompVMatPtr& output, const CompVMatPtr& map, const CompVRectFloat32 inputROI)
	{
		// Private function, no need to check input parameters. Up to the caller.

		const T roi_left = static_cast<T>(inputROI.left);
		const T roi_right = static_cast<T>(inputROI.right);
		const T roi_top = static_cast<T>(inputROI.top);
		const T roi_bottom = static_cast<T>(inputROI.bottom);
		const T* mapXPtr = map->ptr<const T>(0);
		const T* mapYPtr = map->ptr<const T>(1);
		const size_t outputWidth = output->cols();
		const size_t outputHeight = output->rows();
		const size_t  outputStride = output->stride();
		const compv_uscalar_t inStride = static_cast<compv_uscalar_t>(input->stride());
		const compv_uscalar_t inWidthMinus1 = static_cast<compv_uscalar_t>(input->cols() - 1);
		const compv_uscalar_t inHeightMinus1 = static_cast<compv_uscalar_t>(input->rows() - 1);

		CompVImageScaleBicubicProcessor processor;
		COMPV_CHECK_CODE_RETURN(processor.init());

		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");

		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Convert from 8u to 32f not MT");
		CompVMatPtr input32f;
		COMPV_CHECK_CODE_RETURN((CompVMathCast::process_static<uint8_t, compv_float32_t>(input, &input32f)));
		const compv_float32_t* inputPtr = input32f->ptr<const compv_float32_t>();

		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD for COMPV_MATH_ROUNDFU_2_NEAREST_INT at the end");

		auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
			// Bilinear filtering: https://en.wikipedia.org/wiki/Bilinear_interpolation#Unit_square
			uint8_t* outputPtr = output->ptr<uint8_t>(ystart);
			size_t i, j, k;
			compv_float32_t out;
			for (j = ystart, k = (ystart * outputWidth); j < yend; ++j) {
				for (i = 0; i < outputWidth; ++i, ++k) {
					const T& x = mapXPtr[k];
					const T& y = mapYPtr[k];
					if (x < roi_left || x > roi_right || y < roi_top || y > roi_bottom) {
						outputPtr[i] = 0; // TODO(dmi): or mean
					}
					else {
						const int32_t yint = static_cast<int32_t>(y);
						const compv_float32_t yfract = static_cast<compv_float32_t>(y - std::floor(y));

						const int32_t xint = static_cast<int32_t>(x);
						const compv_float32_t xfract = static_cast<compv_float32_t>(x - std::floor(x));

						processor.bicubic_32f32s(
							&out,
							inputPtr, 
							&xint, 
							&xfract, 
							&yint, 
							&yfract,
							inWidthMinus1,
							inHeightMinus1,
							inStride
						);
						outputPtr[i] = COMPV_MATH_ROUNDFU_2_NEAREST_INT(COMPV_MATH_CLIP3(0, 255.f, out), uint8_t);
					}
				}
				outputPtr += outputStride;
			}
			return COMPV_ERROR_CODE_S_OK;
		};
		COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
			funcPtr,
			outputWidth,
			outputHeight,
			COMPV_IMAGE_REMAP_BICUBIC_SAMPLES_PER_THREAD
		));

		return COMPV_ERROR_CODE_S_OK;
	}

public:
	static COMPV_ERROR_CODE process(const CompVMatPtr& input, CompVMatPtrPtr output, const CompVMatPtr& map, COMPV_INTERPOLATION_TYPE interType COMPV_DEFAULT(COMPV_INTERPOLATION_TYPE_BILINEAR), const CompVRectFloat32* inputROI COMPV_DEFAULT(nullptr), const CompVSizeSz* outSize COMPV_DEFAULT(nullptr))
	{
		// For now only grayscale images are supported
		COMPV_CHECK_EXP_RETURN(!input || !output || !map || input->isEmpty() || input->elmtInBytes() != sizeof(uint8_t) || input->planeCount() != 1 || (outSize && (!outSize->height || !outSize->width)), COMPV_ERROR_CODE_E_INVALID_PARAMETER);

		CompVMatPtr output_ = (*output == input) ? nullptr : *output;

		const size_t inputWidth = input->cols();
		const size_t inputHeight = input->rows();

		/* Set inputROI */
		CompVRectFloat32 inputROI_;
		CompVSizeSz outputSize_;
		if (inputROI) {
			inputROI_ = *inputROI;
			inputROI_.left = COMPV_MATH_CLIP3(0.f, static_cast<compv_float32_t>(inputWidth - 1), inputROI_.left);
			inputROI_.right = COMPV_MATH_CLIP3(inputROI_.left, static_cast<compv_float32_t>(inputWidth - 1), inputROI_.right);
			inputROI_.top = COMPV_MATH_CLIP3(0.f, static_cast<compv_float32_t>(inputHeight - 1), inputROI_.top);;
			inputROI_.bottom = COMPV_MATH_CLIP3(inputROI_.top, static_cast<compv_float32_t>(inputHeight - 1), inputROI_.bottom);
		}
		else {
			inputROI_.left = 0.f;
			inputROI_.right = static_cast<compv_float32_t>(inputWidth - 1);
			inputROI_.top = 0.f;
			inputROI_.bottom = static_cast<compv_float32_t>(inputHeight - 1);
		}
		if (outSize) {
			outputSize_ = *outSize;
		}
		else {
			outputSize_.width = COMPV_MATH_ROUNDFU_2_NEAREST_INT(((inputROI_.right - inputROI_.left) + 1), size_t);
			outputSize_.height = COMPV_MATH_ROUNDFU_2_NEAREST_INT(((inputROI_.bottom - inputROI_.top) + 1), size_t);
		}

		// map must contain at least #2 rows (x, y) or (x, y, z) and with exactly n elements (n = (outSize.w*outSize.h)
		const size_t outputElmtCount = (outputSize_.width * outputSize_.height);
		COMPV_CHECK_EXP_RETURN(map->rows() < 2 || map->cols() != outputElmtCount, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Invalid map size");

		// Create output
		const COMPV_SUBTYPE subType = ((input->planeCount() == 1 && input->subType() == COMPV_SUBTYPE_RAW_UINT8) ? COMPV_SUBTYPE_PIXELS_Y : input->subType());
		COMPV_CHECK_CODE_RETURN(CompVImage::newObj8u(&output_, subType, outputSize_.width, outputSize_.height, (input->stride() >= outputSize_.width) ? input->stride() : 0));

		// Perform interpolation
		switch (interType) {
		case COMPV_INTERPOLATION_TYPE_BICUBIC:
			COMPV_CHECK_CODE_RETURN(CompVImageRemapGeneric::process_bicubic(input, output_, map, inputROI_));
			break;
		case COMPV_INTERPOLATION_TYPE_BILINEAR:
			COMPV_CHECK_CODE_RETURN(CompVImageRemapGeneric::process_bilinear(input, output_, map, inputROI_));
			break;
		case COMPV_INTERPOLATION_TYPE_NEAREST:
			COMPV_CHECK_CODE_RETURN(CompVImageRemapGeneric::process_nearest(input, output_, map, inputROI_));
			break;
		default:
			COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Invalid interpolation type: not implemented");
			break;
		}

		*output = output_;
		return COMPV_ERROR_CODE_S_OK;
	}
};

//
//	CompVImageRemap
//

// map = (x, y) values
// map must contain at least #2 rows (x, y) or (x, y, z) and with exactly n elements (n = (outSize.w*outSize.h)
COMPV_ERROR_CODE CompVImageRemap::process(const CompVMatPtr& input, CompVMatPtrPtr output, const CompVMatPtr& map, COMPV_INTERPOLATION_TYPE interType COMPV_DEFAULT(COMPV_INTERPOLATION_TYPE_BILINEAR), const CompVRectFloat32* inputROI COMPV_DEFAULT(nullptr), const CompVSizeSz* outSize COMPV_DEFAULT(nullptr))
{
	COMPV_CHECK_EXP_RETURN(!map, COMPV_ERROR_CODE_E_INVALID_PARAMETER); // other parameters will be tested in the private functions
	switch (map->subType()) {
	case COMPV_SUBTYPE_RAW_FLOAT32: COMPV_CHECK_CODE_RETURN((CompVImageRemapGeneric<compv_float32_t>::process(input, output, map, interType, inputROI, outSize))); break;
	case COMPV_SUBTYPE_RAW_FLOAT64: COMPV_CHECK_CODE_RETURN((CompVImageRemapGeneric<compv_float64_t>::process(input, output, map, interType, inputROI, outSize))); break;
	default: COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED, "map must constain float32 or float64 indices");  break;
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
