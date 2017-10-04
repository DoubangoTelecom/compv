/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/compv_image_remap.h"
#include "compv/base/image/compv_image.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_base.h"

#define COMPV_THIS_CLASSNAME	"CompVImageRemap"

COMPV_NAMESPACE_BEGIN()

// map = (x, y) values
COMPV_ERROR_CODE CompVImageRemap::remap(const CompVMatPtr& input, CompVMatPtrPtr output, const CompVMatPtr& map, COMPV_INTERPOLATION_TYPE interType COMPV_DEFAULT(COMPV_INTERPOLATION_TYPE_BILINEAR), const CompVSizeSz* outputSize COMPV_DEFAULT(nullptr), const CompVRectFloat32* mapROI COMPV_DEFAULT(nullptr))
{
	// For now only grayscale images are supported
	COMPV_CHECK_EXP_RETURN(!input || !output || !map || input->isEmpty() || map->subType() != COMPV_SUBTYPE_RAW_FLOAT32 || input->elmtInBytes() != sizeof(uint8_t) || input->planeCount() != 1, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	// interpolation type must be bilinear or nearest
	COMPV_CHECK_EXP_RETURN(interType != COMPV_INTERPOLATION_TYPE_BILINEAR && interType != COMPV_INTERPOLATION_TYPE_NEAREST, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Interpolation type must be bilinear or nearest");

	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("This code isn't optimized (no fixed-point, no MT, no SIMD). You should create a CompVImageRemapPtr object and keep calling process for each frame.");

	CompVSizeSz outputSize_;
	CompVMatPtr output_ = (*output == input) ? nullptr : *output;
	CompVRectFloat32 mapROI_;

	if (outputSize) {
		outputSize_ = *outputSize;
		COMPV_CHECK_EXP_RETURN(!outputSize_.width || !outputSize_.height, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Invalid output size");
	}
	else {
		outputSize_.width = input->cols();
		outputSize_.height = input->rows();
	}
	const size_t outputElmtCount = (outputSize_.width * outputSize_.height);

	// map must contain at leat #2 rows (x, y) or (x, y, z) and with exactly n elements (n = (outSize.w*outSize.h)
	COMPV_CHECK_EXP_RETURN(map->rows() < 2 || map->cols() != outputElmtCount, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Invalid map size");

	const size_t inputWidth = input->cols();
	const size_t inputHeight = input->rows();

	if (mapROI) {
		mapROI_ = *mapROI;
		mapROI_.left = COMPV_MATH_CLIP3(0.f, static_cast<compv_float32_t>(inputWidth - 1), mapROI_.left);
		mapROI_.right = COMPV_MATH_CLIP3(mapROI_.left, static_cast<compv_float32_t>(inputWidth - 1), mapROI_.right);
		mapROI_.top = COMPV_MATH_CLIP3(0.f, static_cast<compv_float32_t>(inputHeight - 1), mapROI_.top);;
		mapROI_.bottom = COMPV_MATH_CLIP3(mapROI_.top, static_cast<compv_float32_t>(inputHeight - 1), mapROI_.bottom);
	}
	else {
		mapROI_.left = 0.f;
		mapROI_.right = static_cast<compv_float32_t>(inputWidth - 1);
		mapROI_.top = 0.f;
		mapROI_.bottom = static_cast<compv_float32_t>(inputHeight - 1);
	}

	COMPV_CHECK_CODE_RETURN(CompVImage::newObj8u(&output_, input->subType(), outputSize_.width, outputSize_.height));

	const compv_float32_t roi_left = mapROI_.left;
	const compv_float32_t roi_right = mapROI_.right;
	const compv_float32_t roi_top = mapROI_.top;
	const compv_float32_t roi_bottom = mapROI_.bottom;
	const compv_float32_t* mapXPtr = map->ptr<const compv_float32_t>(0);
	const compv_float32_t* mapYPtr = map->ptr<const compv_float32_t>(1);
	const size_t outputWidth = outputSize_.width;
	const size_t outputHeight = outputSize_.height;
	uint8_t* outputPtr = output_->ptr<uint8_t>();
	const size_t  outputStride = output_->stride();
	size_t i, j, k;
	compv_float32_t x, y;

	if (interType == COMPV_INTERPOLATION_TYPE_NEAREST) {
		for (j = 0, k = 0; j < outputHeight; ++j) {
			for (i = 0; i < outputWidth; ++i, ++k) {
				x = mapXPtr[k];
				y = mapYPtr[k];
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
	}
	else if (interType == COMPV_INTERPOLATION_TYPE_BILINEAR) { // Bilinear filtering: https://en.wikipedia.org/wiki/Bilinear_interpolation#Unit_square
		int x1, x2, y1, y2;
		compv_float32_t xfractpart, one_minus_xfractpart, yfractpart, one_minus_yfractpart;
		for (j = 0, k = 0; j < outputHeight; ++j) {
			for (i = 0; i < outputWidth; ++i, ++k) {
				x = mapXPtr[k];
				y = mapYPtr[k];
				if (x < roi_left || x > roi_right || y < roi_top || y > roi_bottom) {
					outputPtr[i] = 0; // TODO(dmi): or mean
				}
				else {
					x1 = static_cast<int>(x);
					x2 = static_cast<int>(x + 1.f);
					xfractpart = x - x1;
					one_minus_xfractpart = 1.f - xfractpart;
					y1 = static_cast<int>(y);
					y2 = static_cast<int>(y + 1.f);
					yfractpart = y - y1;
					one_minus_yfractpart = 1.f - yfractpart;
					outputPtr[i] = static_cast<uint8_t>((*input->ptr<const uint8_t>(y1, x1) * one_minus_yfractpart * one_minus_xfractpart)
						+ (*input->ptr<const uint8_t>(y1, x2) * one_minus_yfractpart * xfractpart)
						+ (*input->ptr<const uint8_t>(y2, x1) * yfractpart * one_minus_xfractpart)
						+ (*input->ptr<const uint8_t>(y2, x2) * yfractpart * xfractpart));
				}
			}
			outputPtr += outputStride;
		}
	}
	else {
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Invalid interpolation type: not implemented");
	}

	*output = output_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
