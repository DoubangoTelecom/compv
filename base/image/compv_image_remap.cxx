/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
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
#include "compv/base/math/compv_math_clip.h"
#include "compv/base/parallel/compv_parallel.h"
#include "compv/base/compv_base.h"
#include "compv/base/compv_generic_invoke.h"
#include "compv/base/compv_cpu.h"

#include "compv/base/image/intrin/x86/compv_image_remap_intrin_sse41.h"
#include "compv/base/image/intrin/x86/compv_image_remap_intrin_avx2.h"
#include "compv/base/image/intrin/arm/compv_image_remap_intrin_neon.h"

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
	static COMPV_ERROR_CODE process_nearest(const CompVMatPtr& input, CompVMatPtr& output, const CompVMatPtr& map, const CompVRectFloat32 inputROI, const uint8_t defaultPixelValue = 0x00)
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
						outputPtr[i] = defaultPixelValue; // TODO(dmi): or mean
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

	// "outputPtr" values always within [0, 256[ which means we can use static cast without clip(0, 255)
	template <typename U>
	static void remapBilinear_8uXf_C(
		const U* mapXPtr, const U* mapYPtr,
		const uint8_t* inputPtr, U* outputPtr,
		const U* roi, const int32_t* size,
		const U* defaultPixelValue1,
		const compv_uscalar_t count
	)
	{
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");

		const U& roi_left = roi[0];
		const U& roi_right = roi[1];
		const U& roi_top = roi[2];
		const U& roi_bottom = roi[3];

		const int32_t& inWidthMinus1 = size[0];
		const int32_t& inHeightMinus1 = size[1];
		const int32_t& stride = size[2];

		const U& defaultPixelValue = *defaultPixelValue1;

		for (compv_uscalar_t i = 0; i < count; ++i) {
			const U& x = mapXPtr[i];
			const U& y = mapYPtr[i];
			if (x >= roi_left && x <= roi_right && y >= roi_top && y <= roi_bottom) {
				const int32_t x1 = static_cast<int32_t>(x);
				const int32_t x2 = std::min(static_cast<int32_t>(x + 1.f), inWidthMinus1);
				const U xfractpart = x - x1;
				int32_t y1 = static_cast<int32_t>(y);
				int32_t y2 = std::min(static_cast<int32_t>(y + 1.f), inHeightMinus1);
				const U yfractpart = y - y1;
				const U xyfractpart = (xfractpart * yfractpart);
				y1 *= stride;
				y2 *= stride;
				const int32_t y1x1 = y1 + x1;
				const int32_t y1x2 = y1 + x2;
				const int32_t y2x1 = y2 + x1;
				const int32_t y2x2 = y2 + x2;
				const U pixel = (
					(inputPtr[y1x1] * (1 - xfractpart - yfractpart + xyfractpart))
					+ (inputPtr[y1x2] * (xfractpart - xyfractpart))
					+ (inputPtr[y2x1] * (yfractpart - xyfractpart))
					+ (inputPtr[y2x2] * xyfractpart)
					);
#if ((defined(_DEBUG) && _DEBUG != 0) || (defined(DEBUG) && DEBUG != 0))
				COMPV_ASSERT(pixel >= 0 && pixel < 256);
#endif
				outputPtr[i] = pixel;
			}
			else {
				outputPtr[i] = defaultPixelValue;
			}
		}
	}
	
	static COMPV_ERROR_CODE process_bilinear(const CompVMatPtr& input, CompVMatPtr& output, const CompVMatPtr& map, const CompVRectFloat32 inputROI, const COMPV_INTERPOLATION_TYPE bilinearType, const uint8_t defaultPixelValue = 0x00)
	{
		// Private function, no need to check input parameters. Up to the caller.

		COMPV_ALIGN_DEFAULT() const T roi[] = { static_cast<T>(inputROI.left), static_cast<T>(inputROI.right), static_cast<T>(inputROI.top), static_cast<T>(inputROI.bottom) };
		COMPV_ALIGN_DEFAULT() const int32_t size[] = { static_cast<int32_t>(input->cols() - 1), static_cast<int32_t>(input->rows() - 1), static_cast<int32_t>(input->stride()) };
		const T* mapXPtr = map->ptr<const T>(0);
		const T* mapYPtr = map->ptr<const T>(1);
		const size_t outputWidth = output->cols();
		const size_t outputHeight = output->rows();
		const size_t  outputStride = output->stride();
		const bool isT32f = std::is_same<T, compv_float32_t>::value;

		CompVMatPtr outT;
		if (bilinearType == COMPV_INTERPOLATION_TYPE_BILINEAR_FLOAT32 && std::is_same<T, compv_float32_t>::value && output->isRawTypeMatch<compv_float32_t>()) {
			outT = output;
		}
		else {
			COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<T>(&outT, outputHeight, outputWidth, outputStride));
		}

		int align = 1;
		const T defaultPixelValueT = static_cast<T>(defaultPixelValue);
		void(*CompVImageRemapBilinear_8u32f)(
			const compv_float32_t* mapXPtr, const compv_float32_t* mapYPtr,
			const uint8_t* inputPtr, compv_float32_t* outputPtr,
			const compv_float32_t* roi, const int32_t* size,
			const compv_float32_t* defaultPixelValue1,
			const compv_uscalar_t count
			) = remapBilinear_8uXf_C;
#if COMPV_ARCH_X86
		if (CompVCpu::isEnabled(kCpuFlagSSE41) && map->isAlignedSSE()) {
			COMPV_EXEC_IFDEF_INTRIN_X86((CompVImageRemapBilinear_8u32f = CompVImageRemapBilinear_8u32f_Intrin_SSE41, align = 4));
		}
		if (CompVCpu::isEnabled(kCpuFlagAVX2) && map->isAlignedAVX()) {
			COMPV_EXEC_IFDEF_INTRIN_X86((CompVImageRemapBilinear_8u32f = CompVImageRemapBilinear_8u32f_Intrin_AVX2, align = 8));
		}
#elif COMPV_ARCH_ARM
		if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && map->isAlignedNEON()) {
			COMPV_EXEC_IFDEF_INTRIN_ARM((CompVImageRemapBilinear_8u32f = CompVImageRemapBilinear_8u32f_Intrin_NEON, align = 4));
		}
#endif

		auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
			// Bilinear filtering: https://en.wikipedia.org/wiki/Bilinear_interpolation#Unit_square
			T* outTPtr = outT->ptr<T>(ystart);
			const uint8_t* inputPtr = input->ptr<uint8_t>();
			for (size_t j = ystart, k = (ystart * outputWidth); j < yend; ++j) {
				const int kalignedSIMD = static_cast<int>(k & -align); // make SIMD-friendly (unchanged when SIMD disabled)
				const int extraSIMD = static_cast<int>(k - kalignedSIMD); // extra (zero when SIMD disabled)
				if (isT32f) {
					CompVImageRemapBilinear_8u32f(
						reinterpret_cast<const compv_float32_t*>(&mapXPtr[kalignedSIMD]), reinterpret_cast<const compv_float32_t*>(&mapYPtr[kalignedSIMD]),
						inputPtr, reinterpret_cast<compv_float32_t*>(&outTPtr[-extraSIMD]), // no-aligned access
						reinterpret_cast<const compv_float32_t*>(roi), size,
						reinterpret_cast<const compv_float32_t*>(&defaultPixelValueT),
						outputWidth + extraSIMD
						);
				}
				else {
					remapBilinear_8uXf_C<T>(
						&mapXPtr[kalignedSIMD], &mapYPtr[kalignedSIMD],
						inputPtr, &outTPtr[-extraSIMD],
						roi, size,
						&defaultPixelValueT,
						outputWidth + extraSIMD
						);
				}
				k += outputWidth;
				outTPtr += outputStride;
			}
			return COMPV_ERROR_CODE_S_OK;
		};
		COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
			funcPtr,
			outputWidth,
			outputHeight,
			COMPV_IMAGE_REMAP_BILINEAR_SAMPLES_PER_THREAD
		));

		if (bilinearType == COMPV_INTERPOLATION_TYPE_BILINEAR_FLOAT32) {
			if (!isT32f) {
				COMPV_CHECK_CODE_RETURN((CompVMathCast::process_static<T, compv_float32_t>(outT, &output))); // no need for clip, output already within [0, 256[
			}
			else if (outT != output) {
				output = outT;
			}
		}
		else {
			COMPV_CHECK_CODE_RETURN((CompVMathCast::process_static_pixel8(outT, &output)));
		}

		return COMPV_ERROR_CODE_S_OK;
	}

	static COMPV_ERROR_CODE process_bicubic(const CompVMatPtr& input, CompVMatPtr& output, const CompVMatPtr& map, const CompVRectFloat32 inputROI, const COMPV_INTERPOLATION_TYPE bicubicType, const uint8_t defaultPixelValue = 0x00)
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

		// TODO(dmi): add faster implementation
		// https://github.com/DoubangoTelecom/compv/issues/167
		// xfract and yfract could be computed once using Matrix (y=0 then x = 0) then, use postprocess (see scale)
#if 0
		if ((outputHeight * outputWidth) > (32 * 32))
#endif
		{
			COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("OpenCV implementation [fixed-point] is faster [but less accurate]");
			COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
		}

		CompVMatPtr input32f;
		if (input->isRawTypeMatch<compv_float32_t>()) {
			input32f = input;
		}
		else {
			COMPV_CHECK_CODE_RETURN((CompVMathCast::process_static<uint8_t, compv_float32_t>(input, &input32f)));
		}
		const compv_float32_t* inputPtr = input32f->ptr<const compv_float32_t>();

		CompVMatPtr output32f;
		if (output->isRawTypeMatch<compv_float32_t>()) {
			output32f = output;
		}
		else {
			COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float32_t>(&output32f, outputHeight, outputWidth, outputStride));
		}

		auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
			// Bilinear filtering: https://en.wikipedia.org/wiki/Bilinear_interpolation#Unit_square
			compv_float32_t* outputPtr = output32f->ptr<compv_float32_t>(ystart);
			size_t i, j, k;
			for (j = ystart, k = (ystart * outputWidth); j < yend; ++j) {
				for (i = 0; i < outputWidth; ++i, ++k) {
					const T& x = mapXPtr[k];
					const T& y = mapYPtr[k];
					if (x < roi_left || x > roi_right || y < roi_top || y > roi_bottom) {
						outputPtr[i] = defaultPixelValue; // TODO(dmi): or mean
					}
					else {
						const int32_t yint = static_cast<int32_t>(y);
						const compv_float32_t yfract = static_cast<compv_float32_t>(y - std::floor(y));

						const int32_t xint = static_cast<int32_t>(x);
						const compv_float32_t xfract = static_cast<compv_float32_t>(x - std::floor(x));

						// Next function not optiz at all, must not use with large images
						processor.NOT_OPTIMIZ_hermite_32f32s(
							&outputPtr[i],
							inputPtr, 
							&xint, 
							&xfract,
							&yint, 
							&yfract,
							inWidthMinus1,
							inHeightMinus1,
							inStride
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
			COMPV_IMAGE_REMAP_BICUBIC_SAMPLES_PER_THREAD
		));

		if (bicubicType == COMPV_INTERPOLATION_TYPE_BICUBIC) {
			COMPV_CHECK_CODE_RETURN((CompVMathCast::process_static_pixel8(output32f, &output)));
		}
		else { // it's "COMPV_INTERPOLATION_TYPE_BICUBIC_FLOAT32"
			COMPV_CHECK_CODE_RETURN(CompVMathClip::clip3(output32f, 0.0, 255.0, &output));
		}

		return COMPV_ERROR_CODE_S_OK;
	}

public:
	static COMPV_ERROR_CODE process(const CompVMatPtr& input, CompVMatPtrPtr output, const CompVMatPtr& map, COMPV_INTERPOLATION_TYPE interType COMPV_DEFAULT(COMPV_INTERPOLATION_TYPE_BILINEAR), const CompVRectFloat32* inputROI COMPV_DEFAULT(nullptr), const CompVSizeSz* outSize COMPV_DEFAULT(nullptr), const uint8_t defaultPixelValue COMPV_DEFAULT(0x00))
	{
		// For now only grayscale images are supported
		COMPV_CHECK_EXP_RETURN(!input || !output || !map || input->isEmpty() || input->planeCount() != 1 || (outSize && (!outSize->height || !outSize->width)), COMPV_ERROR_CODE_E_INVALID_PARAMETER);

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
		if (interType == COMPV_INTERPOLATION_TYPE_BICUBIC_FLOAT32 || interType == COMPV_INTERPOLATION_TYPE_BILINEAR_FLOAT32) {
			COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float32_t>(&output_, outputSize_.height, outputSize_.width));
		}
		else {
			const COMPV_SUBTYPE subType = ((input->planeCount() == 1 && input->subType() == COMPV_SUBTYPE_RAW_UINT8) ? COMPV_SUBTYPE_PIXELS_Y : input->subType());
			COMPV_CHECK_CODE_RETURN(CompVImage::newObj8u(&output_, subType, outputSize_.width, outputSize_.height));
		}

		// Perform interpolation
		switch (interType) {
		case COMPV_INTERPOLATION_TYPE_BICUBIC:
		case COMPV_INTERPOLATION_TYPE_BICUBIC_FLOAT32:
			COMPV_CHECK_CODE_RETURN(CompVImageRemapGeneric::process_bicubic(input, output_, map, inputROI_, interType, defaultPixelValue));
			break;
		case COMPV_INTERPOLATION_TYPE_BILINEAR:
		case COMPV_INTERPOLATION_TYPE_BILINEAR_FLOAT32:
			COMPV_CHECK_EXP_RETURN(input->elmtInBytes() != sizeof(uint8_t), COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Requires 8u as input");
			COMPV_CHECK_CODE_RETURN(CompVImageRemapGeneric::process_bilinear(input, output_, map, inputROI_, interType, defaultPixelValue));
			break;
		case COMPV_INTERPOLATION_TYPE_NEAREST:
			COMPV_CHECK_EXP_RETURN(input->elmtInBytes() != sizeof(uint8_t), COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Requires 8u as input");
			COMPV_CHECK_CODE_RETURN(CompVImageRemapGeneric::process_nearest(input, output_, map, inputROI_, defaultPixelValue));
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
COMPV_ERROR_CODE CompVImageRemap::process(const CompVMatPtr& input, CompVMatPtrPtr output, const CompVMatPtr& map, COMPV_INTERPOLATION_TYPE interType COMPV_DEFAULT(COMPV_INTERPOLATION_TYPE_BILINEAR), const CompVRectFloat32* inputROI COMPV_DEFAULT(nullptr), const CompVSizeSz* outSize COMPV_DEFAULT(nullptr), const uint8_t defaultPixelValue COMPV_DEFAULT(0x00))
{
	COMPV_CHECK_EXP_RETURN(!map, COMPV_ERROR_CODE_E_INVALID_PARAMETER); // other parameters will be tested in the private functions
	switch (map->subType()) {
	case COMPV_SUBTYPE_RAW_FLOAT32: COMPV_CHECK_CODE_RETURN((CompVImageRemapGeneric<compv_float32_t>::process(input, output, map, interType, inputROI, outSize, defaultPixelValue))); break;
	case COMPV_SUBTYPE_RAW_FLOAT64: COMPV_CHECK_CODE_RETURN((CompVImageRemapGeneric<compv_float64_t>::process(input, output, map, interType, inputROI, outSize, defaultPixelValue))); break;
	default: COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED, "map must constain float32 or float64 indices");  break;
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
