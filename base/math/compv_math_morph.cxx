/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/compv_math_morph.h"
#include "compv/base/math/compv_math_utils.h"
#include "compv/base/image/compv_image.h"

#define COMPV_THIS_CLASSNAME	"CompVMathMorph"

// Mathematical morphology: https://en.wikipedia.org/wiki/Mathematical_morphology

COMPV_NAMESPACE_BEGIN()

COMPV_ERROR_CODE CompVMathMorph::buildStructuringElement(CompVMatPtrPtr strel, const CompVSizeSz size, COMPV_MATH_MORPH_STREL_TYPE type COMPV_DEFAULT(COMPV_MATH_MORPH_STREL_TYPE_RECT))
{
	COMPV_CHECK_EXP_RETURN(!strel || !size.width || !size.width, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVMatPtr strel_ = *strel;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<uint8_t>(&strel_, size.height, size.width)); // For now only 8u elements are supported
	uint8_t* strelPtr_ = strel_->ptr<uint8_t>();
	const size_t stride = strel_->stride();
	const size_t width = strel_->cols();
	const size_t height = strel_->rows();
	switch (type) {
	case COMPV_MATH_MORPH_STREL_TYPE_RECT: {
		for (size_t j = 0; j < height; ++j) {
			for (size_t i = 0; i < width; ++i) {
				strelPtr_[i] = 0xff;
			}
			strelPtr_ += stride;
		}
		break;
	}
	case COMPV_MATH_MORPH_STREL_TYPE_CROSS: {
		COMPV_CHECK_CODE_RETURN(strel_->zero_rows());
		strelPtr_ = strel_->ptr<uint8_t>(height >> 1);
		for (size_t i = 0; i < width; ++i) {
			strelPtr_[i] = 0xff;
		}
		strelPtr_ = strel_->ptr<uint8_t>(0, width >> 1);
		for (size_t j = 0; j < height; ++j) {
			strelPtr_[0] = 0xff;
			strelPtr_ += stride;
		}
		break;
	}
	case COMPV_MATH_MORPH_STREL_TYPE_DIAMOND: {
		COMPV_CHECK_CODE_RETURN(strel_->zero_rows());
		size_t i, j, count;
		const size_t height_div2 = height >> 1;
		const size_t width_div2 = width >> 1;
		const size_t stride_plus1 = stride + 1;
		const size_t stride_minus1 = stride - 1;
		// Top
		strelPtr_ = strel_->ptr<uint8_t>(0, width_div2);
		for (j = 0, count = 1; j < height_div2; ++j, count += 2) {
			for (i = 0; i < count; ++i) {
				strelPtr_[i] = 0xff;
			}
			strelPtr_ += stride_minus1;
		}
		// Bottom
		for (j = 0; j <= height_div2; ++j, count -= 2) {
			for (i = 0; i < count; ++i) {
				strelPtr_[i] = 0xff;
			}
			strelPtr_ += stride_plus1;
		}
		break;
	}
	default:
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED, "Invalid structuring element type");
		break;
	}

	*strel = strel_;
	return COMPV_ERROR_CODE_S_OK;
}

// strel must contain 0x00 and type_max (e.g. 0xff for uint8_t) values.
COMPV_ERROR_CODE CompVMathMorph::process(const CompVMatPtr input, const CompVMatPtr strel, CompVMatPtrPtr output, COMPV_MATH_MORPH_OP_TYPE opType, COMPV_BORDER_TYPE borderType COMPV_DEFAULT(COMPV_BORDER_TYPE_REPLICATE))
{
	COMPV_CHECK_EXP_RETURN(
		!input || input->isEmpty() || input->elmtInBytes() != sizeof(uint8_t) || input->planeCount() != 1 ||
		!strel || strel->isEmpty() || strel->elmtInBytes() != sizeof(uint8_t) || strel->planeCount() != 1 ||
		input->cols() < (strel->cols() >> 1) || input->rows() < (strel->rows() >> 1) ||
		!output || (input == *output),
		COMPV_ERROR_CODE_E_INVALID_PARAMETER
	);
	
	const size_t width = input->cols();
	const size_t height = input->rows();
	const size_t stride = input->stride();

	// output can't be equal to input
	// create new output only if doesn't match the required format
	CompVMatPtr output_ = *output;
	if (!output_ || output_->planeCount() != 1 || output_->elmtInBytes() != sizeof(uint8_t) || output_->cols() != width || output_->rows() != height || output_->stride() != stride) {
		COMPV_CHECK_CODE_RETURN(CompVImage::newObj8u(&output_, COMPV_SUBTYPE_PIXELS_Y, width, height, stride));
	}

	COMPV_DEBUG_INFO_CODE_FOR_TESTING("remove zero_all");
	COMPV_CHECK_CODE_RETURN(output_->zero_all());

	// Default: Dialate -> replicate, Erode -> zero (MUST be after the operation is done)
	COMPV_DEBUG_INFO_CODE_TODO("replicate or zero borders");

	const size_t strel_width = strel->cols();
	const size_t strel_width_div2 = strel_width >> 1;

	const size_t strel_height = strel->rows();
	const size_t strel_height_div2 = strel_height >> 1;

	const size_t strel_stride = strel->stride();

	const uint8_t* strelPtr = strel->ptr<const uint8_t>(0, 0);
	const uint8_t* inputPtr = input->ptr<const uint8_t>(0, 0);
	uint8_t* outPtr = output_->ptr<uint8_t>(strel_height_div2, strel_width_div2);
	//const size_t op_pad = ((stride - width) + strel_width_div2 + strel_width_div2);
	const compv_uscalar_t op_height = static_cast<compv_uscalar_t>(height - strel_height);
	const compv_uscalar_t op_width = static_cast<compv_uscalar_t>(width - strel_width);

	/* Build flat strel indices */
	CompVMatPtr flattenStrel;
	COMPV_CHECK_CODE_RETURN(CompVMathMorph::flattenStructuringElement(strel, stride, &flattenStrel));
	const compv_uscalar_t* flattenStrelPtr = flattenStrel->ptr<const compv_uscalar_t>();
	const compv_uscalar_t flattenStrelCount = flattenStrel->cols();

	for (compv_uscalar_t op_j = 0; op_j < op_height; ++op_j) {
		for (compv_uscalar_t op_i = 0; op_i < op_width; ++op_i) {
			uint8_t minn = inputPtr[op_i + flattenStrelPtr[0]];
			for (compv_uscalar_t strel_i = 1; strel_i < flattenStrelCount; ++strel_i) {
				minn = std::min(minn, inputPtr[op_i + flattenStrelPtr[strel_i]]);
			}
			outPtr[op_i] = minn;
		}
		outPtr += stride;
		inputPtr += stride;
	}

	*output = *output_;

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMathMorph::flattenStructuringElement(const CompVMatPtr strel, const size_t inputStride, CompVMatPtrPtr flattenStrel)
{
	// Internal function -> minimum test for input parameters
	COMPV_CHECK_EXP_RETURN(
		!strel || strel->isEmpty() || strel->planeCount() != 1 || strel->elmtInBytes() != sizeof(uint8_t) ||
		!inputStride ||
		!flattenStrel,
		COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	const compv_uscalar_t width = static_cast<compv_uscalar_t>(strel->cols());
	const compv_uscalar_t height = static_cast<compv_uscalar_t>(strel->rows());
	const compv_uscalar_t stride = static_cast<compv_uscalar_t>(strel->stride());

	// Count number of non-zero elements
	compv_uscalar_t count = 0;
	const uint8_t* strelPtr = strel->ptr<const uint8_t>(0, 0);
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; ++i) {
			if (strelPtr[i]) {
				++count;
			}
		}
		strelPtr += stride;
	}

	// Fill flattened strel
	COMPV_CHECK_EXP_RETURN(!count, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Structured element is full of zeros");
	CompVMatPtr strel_;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_uscalar_t>(&strel_, 1, count));
	strelPtr = strel->ptr<const uint8_t>(0, 0);
	compv_uscalar_t* flat_strelPtr = strel_->ptr<compv_uscalar_t>();
	const compv_uscalar_t inputStride_ = static_cast<compv_uscalar_t>(inputStride);
	for (compv_uscalar_t j = 0, index = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; ++i) {
			if (strelPtr[i]) {
				flat_strelPtr[index++] = (j * inputStride_) + i;
			}
		}
		strelPtr += stride;
	}

	*flattenStrel = strel_;

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
