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

template<typename T>
class CompVMathMorphOpMin { /* Op for Erode */
public:
	COMPV_ALWAYS_INLINE T operator()(const T& x, const T& y) const { return COMPV_MATH_MIN(x, y); }
};

template<typename T>
class CompVMathMorphOpMax { /* Op for Dilate */
public:
	COMPV_ALWAYS_INLINE T operator()(const T& x, const T& y) const { return COMPV_MATH_MAX(x, y); }
};

template <typename T, class CompVMathMorphOp>
static void CompVMathMorphProcess_C(const compv_uscalar_t* strelInputPtrsPtr, const compv_uscalar_t strelInputPtrsCount, T* outPtr, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride);

// In the next code we use 0xff but any non-zero sample is ok
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

COMPV_ERROR_CODE CompVMathMorph::process(const CompVMatPtr& input, const CompVMatPtr& strel, CompVMatPtrPtr output, COMPV_MATH_MORPH_OP_TYPE opType, COMPV_BORDER_TYPE borderType COMPV_DEFAULT(COMPV_BORDER_TYPE_REPLICATE))
{
	COMPV_CHECK_EXP_RETURN(
		!input || input->isEmpty() || input->elmtInBytes() != sizeof(uint8_t) || input->planeCount() != 1 ||
		!strel || strel->isEmpty() || strel->elmtInBytes() != sizeof(uint8_t) || strel->planeCount() != 1 ||
		input->cols() < (strel->cols() >> 1) || input->rows() < (strel->rows() >> 1) ||
		!output || (input == *output),
		COMPV_ERROR_CODE_E_INVALID_PARAMETER
	);
	
	const size_t input_width = input->cols();
	const size_t input_height = input->rows();
	const size_t input_stride = input->stride();

	// output can't be equal to input
	// create new output only if doesn't match the required format
	CompVMatPtr output_ = *output;
	if (!output_ || output_->planeCount() != 1 || output_->elmtInBytes() != sizeof(uint8_t) || output_->cols() != input_width || output_->rows() != input_height || output_->stride() != input_stride) {
		COMPV_CHECK_CODE_RETURN(CompVImage::newObj8u(&output_, COMPV_SUBTYPE_PIXELS_Y, input_width, input_height, input_stride));
	}

	/* Local variables */
	const size_t strel_width = strel->cols();
	const size_t strel_height = strel->rows();
	const size_t strel_width_div2 = strel_width >> 1;
	const size_t strel_height_div2 = strel_height >> 1;
	uint8_t* outPtr = output_->ptr<uint8_t>(strel_height_div2, strel_width_div2);
	const compv_uscalar_t op_height = static_cast<compv_uscalar_t>(input_height - (strel_height_div2 << 1));
	const compv_uscalar_t op_width = static_cast<compv_uscalar_t>(input_width - (strel_width_div2 << 1));

	/* Collect strel input pointers */
	CompVMatPtr strelInputPtrs;
	COMPV_CHECK_CODE_RETURN(CompVMathMorph::buildStructuringElementInputPtrs(strel, input, &strelInputPtrs));
	const compv_uscalar_t* strelInputPtrsPtr = strelInputPtrs->ptr<const compv_uscalar_t>();
	const compv_uscalar_t strelInputPtrsCount = strelInputPtrs->cols();

	/* Process */
	CompVMathMorphProcess_C<uint8_t, CompVMathMorphOpMin<uint8_t> >(
		strelInputPtrsPtr, strelInputPtrsCount, 
		outPtr, op_width, op_height, static_cast<compv_uscalar_t>(input_stride)
	);

	/* Add Borders (must be after processing and vt then Hz) */
	COMPV_CHECK_CODE_RETURN(addBordersVt(input, output_, strel_height, borderType, borderType));
	COMPV_CHECK_CODE_RETURN(addBordersHz(input, output_, strel_width, borderType));

	*output = *output_;

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMathMorph::buildStructuringElementInputPtrs(const CompVMatPtr& strel, const CompVMatPtr& input, CompVMatPtrPtr strelInputPtrs)
{
	// Internal function -> minimum test for input parameters
	COMPV_CHECK_EXP_RETURN(!strelInputPtrs, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

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

	// Collect non-zero elements
	COMPV_CHECK_EXP_RETURN(!count, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Structured element is full of zeros");
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_uscalar_t>(strelInputPtrs, 1, count));
	strelPtr = strel->ptr<const uint8_t>(0, 0);
	compv_uscalar_t* strelInputPtrsPtr = (*strelInputPtrs)->ptr<compv_uscalar_t>();
	for (compv_uscalar_t j = 0, index = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; ++i) {
			if (strelPtr[i]) {
				strelInputPtrsPtr[index++] = reinterpret_cast<compv_uscalar_t>(input->ptr<const uint8_t>(j, i));
			}
		}
		strelPtr += stride;
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMathMorph::addBordersVt(const CompVMatPtr input, CompVMatPtr output, const size_t strelHeight, const COMPV_BORDER_TYPE borderTypeTop, const COMPV_BORDER_TYPE borderTypeBottom)
{
	// Internal function -> minimum test for input parameters
	COMPV_CHECK_EXP_RETURN(
		!input || input->isEmpty() || input->planeCount() != 1 || input->elmtInBytes() != sizeof(uint8_t) ||
		!output || output->planeCount() != 1 || output->elmtInBytes() != sizeof(uint8_t) || 
		output->cols() != input->cols() || output->rows() != input->rows() || output->stride() != input->stride(),
		COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	const size_t strelHeightDiv2 = (strelHeight >> 1);
	const size_t width = input->cols();
	const size_t height = input->rows();
	const size_t stride = input->stride();

	// Segmentation fault when the input image has an offset (e.g. bound to non-zero cols):
	//		-> for the last row reset 'width' only instead of 'stride'.
	const size_t bSizeInSamples = (((strelHeightDiv2 - 1) * stride) + width);
	uint8_t* outPtr = output->ptr<uint8_t>();
	const uint8_t* inPtr = input->ptr<const uint8_t>();

	// Top
	if (borderTypeTop == COMPV_BORDER_TYPE_ZERO) {
		CompVMem::zero(outPtr, bSizeInSamples * sizeof(uint8_t));
	}
	else if (borderTypeTop == COMPV_BORDER_TYPE_REPLICATE) {
		memcpy(outPtr, inPtr, bSizeInSamples * sizeof(uint8_t));
	}
	else if (borderTypeTop != COMPV_BORDER_TYPE_IGNORE) {
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	}

	// Bottom
	if (borderTypeBottom == COMPV_BORDER_TYPE_ZERO) {
		const size_t offsetInSamples = ((height - strelHeightDiv2) * stride);
		CompVMem::zero(outPtr + offsetInSamples, bSizeInSamples * sizeof(uint8_t));
	}
	else if (borderTypeBottom == COMPV_BORDER_TYPE_REPLICATE) {
		const size_t offsetInSamples = ((height - strelHeightDiv2) * stride);
		memcpy(&outPtr[offsetInSamples], &inPtr[offsetInSamples], bSizeInSamples * sizeof(uint8_t));
	}
	else if (borderTypeBottom != COMPV_BORDER_TYPE_IGNORE) {
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMathMorph::addBordersHz(const CompVMatPtr input, CompVMatPtr output, const size_t strelWidth, const COMPV_BORDER_TYPE borderType)
{
	// Internal function -> minimum test for input parameters
	COMPV_CHECK_EXP_RETURN(
		!input || input->isEmpty() || input->planeCount() != 1 || input->elmtInBytes() != sizeof(uint8_t) ||
		!output || output->planeCount() != 1 || output->elmtInBytes() != sizeof(uint8_t) ||
		output->cols() != input->cols() || output->rows() != input->rows() || output->stride() != input->stride(),
		COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	const size_t strelWidthDiv2 = (strelWidth >> 1);
	const size_t width = input->cols();
	const size_t height = input->rows();
	const size_t stride = input->stride();
	uint8_t* outPtr = output->ptr<uint8_t>();
	const uint8_t* inPtr = input->ptr<const uint8_t>();

	// Set hz borders to zero
	// We must not accept garbage in the border (could be used by the calling function -e.g to find the max value for normalization)
	if (borderType == COMPV_BORDER_TYPE_ZERO) {
		uint8_t *outPtr0 = outPtr, *outPtr1 = outPtr + (width - strelWidthDiv2);
		switch (strelWidthDiv2) { // 1 and 2 (kernel sizes 3 and 5 are very common)
		case 1: {
			const size_t kmax = (stride * height);
			for (size_t k = 0; k < kmax; k += stride) {
				outPtr0[k] = 0, outPtr1[k] = 0;
			}
			break;
		}
		case 2: {
			const size_t kmax = (stride * height);
			for (size_t k = 0; k < kmax; k += stride) {
				outPtr0[k] = 0, outPtr0[k + 1] = 0, outPtr1[k] = 0, outPtr1[k + 1] = 0;
			}
			break;
		}
		default: {
			for (size_t row = 0; row < height; ++row) {
				for (size_t col = 0; col < strelWidthDiv2; ++col) {
					outPtr0[col] = 0, outPtr1[col] = 0;
				}
				outPtr0 += stride;
				outPtr1 += stride;
			}
			break;
		}
		}
	}
	else if (borderType == COMPV_BORDER_TYPE_REPLICATE) {
		const uint8_t *inPtr0 = inPtr, *inPtr1 = inPtr + (width - strelWidthDiv2);
		uint8_t *outPtr0 = outPtr, *outPtr1 = outPtr + (width - strelWidthDiv2);
		for (size_t row = 0; row < height; ++row) {
			for (size_t col = 0; col < strelWidthDiv2; ++col) {
				outPtr0[col] = inPtr0[col], outPtr1[col] = inPtr1[col];
			}
			outPtr0 += stride;
			outPtr1 += stride;
			inPtr0 += stride;
			inPtr1 += stride;
		}
	}
	else if (borderType != COMPV_BORDER_TYPE_IGNORE) {
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	}

	return COMPV_ERROR_CODE_S_OK;
}

template <typename T, class CompVMathMorphOp>
static void CompVMathMorphProcess_C(const compv_uscalar_t* strelInputPtrsPtr, const compv_uscalar_t strelInputPtrsCount, T* outPtr, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
	CompVMathMorphOp Op;
	const compv_uscalar_t strelInputPtrsPad = (stride - width);
	for (compv_uscalar_t j = 0, k = 0; j < height; ++j, k += strelInputPtrsPad) {
		for (compv_uscalar_t i = 0; i < width; ++i, ++k) {
			T rr = *reinterpret_cast<const T*>(k + strelInputPtrsPtr[0]);
			for (compv_uscalar_t v = 1; v < strelInputPtrsCount; ++v) {
				rr = Op(rr, *reinterpret_cast<const T*>(k + strelInputPtrsPtr[v]));
			}
			outPtr[i] = rr;
		}
		outPtr += stride;
	}
}

COMPV_NAMESPACE_END()
