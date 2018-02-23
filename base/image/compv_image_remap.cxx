/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/compv_image_remap.h"
#include "compv/base/image/compv_image.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/parallel/compv_parallel.h"
#include "compv/base/compv_base.h"

#define COMPV_THIS_CLASSNAME	"CompVImageRemap"

#define COMPV_IMAGE_REMAP_BILINEAR_SAMPLES_PER_THREAD (40 * 20)
#define COMPV_IMAGE_REMAP_NEAREST_SAMPLES_PER_THREAD (40 * 40)

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

		// Compute number of threads
		CompVThreadDispatcherPtr threadDisp = CompVParallel::threadDispatcher();
		const size_t maxThreads = threadDisp ? static_cast<size_t>(threadDisp->threadsCount()) : 1;
		const size_t threadsCount = (threadDisp && !threadDisp->isMotherOfTheCurrentThread())
			? CompVThreadDispatcher::guessNumThreadsDividingAcrossY(outputWidth, outputHeight, maxThreads, COMPV_IMAGE_REMAP_NEAREST_SAMPLES_PER_THREAD)
			: 1;

		auto funcPtr = [&](const size_t ystart, const size_t yend) -> void {
			uint8_t* outputPtr = output->ptr<uint8_t>(ystart);
			size_t i, j, k;
			T x, y;
			for (j = ystart, k = (ystart * outputWidth); j < yend; ++j) {
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
		};

		if (threadsCount > 1) {
			CompVAsyncTaskIds taskIds;
			taskIds.reserve(threadsCount);
			const size_t heights = (outputHeight / threadsCount);
			size_t YStart = 0, YEnd;
			for (size_t threadIdx = 0; threadIdx < threadsCount; ++threadIdx) {
				YEnd = (threadIdx == (threadsCount - 1)) ? outputHeight : (YStart + heights);
				COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtr, YStart, YEnd), taskIds), "Dispatching task failed");
				YStart += heights;
			}
			COMPV_CHECK_CODE_RETURN(threadDisp->wait(taskIds), "Failed to wait for tasks execution");
		}
		else {
			funcPtr(0, outputHeight);
		}

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

		// Compute number of threads
		CompVThreadDispatcherPtr threadDisp = CompVParallel::threadDispatcher();
		const size_t maxThreads = threadDisp ? static_cast<size_t>(threadDisp->threadsCount()) : 1;
		const size_t threadsCount = (threadDisp && !threadDisp->isMotherOfTheCurrentThread())
			? CompVThreadDispatcher::guessNumThreadsDividingAcrossY(outputWidth, outputHeight, maxThreads, COMPV_IMAGE_REMAP_BILINEAR_SAMPLES_PER_THREAD)
			: 1;

		auto funcPtr = [&](const size_t ystart, const size_t yend) -> void {
			// Bilinear filtering: https://en.wikipedia.org/wiki/Bilinear_interpolation#Unit_square
			uint8_t* outputPtr = output->ptr<uint8_t>(ystart);
			size_t i, j, k;
			T x, y;
			int x1, x2, y1, y2;
			T xfractpart, one_minus_xfractpart, yfractpart, one_minus_yfractpart;
			const uint8_t *inputY1, *inputY2;
			for (j = ystart, k = (ystart * outputWidth); j < yend; ++j) {
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
						inputY1 = input->ptr<const uint8_t>(y1);
						inputY2 = input->ptr<const uint8_t>(y2);
						outputPtr[i] = static_cast<uint8_t>(
							(inputY1[x1] * one_minus_yfractpart * one_minus_xfractpart)
							+ (inputY1[x2] * one_minus_yfractpart * xfractpart)
							+ (inputY2[x1] * yfractpart * one_minus_xfractpart)
							+ (inputY2[x2] * yfractpart * xfractpart)
						);
					}
				}
				outputPtr += outputStride;
			}
		};
		if (threadsCount > 1) {
			CompVAsyncTaskIds taskIds;
			taskIds.reserve(threadsCount);
			const size_t heights = (outputHeight / threadsCount);
			size_t YStart = 0, YEnd;
			for (size_t threadIdx = 0; threadIdx < threadsCount; ++threadIdx) {
				YEnd = (threadIdx == (threadsCount - 1)) ? outputHeight : (YStart + heights);
				COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtr, YStart, YEnd), taskIds), "Dispatching task failed");
				YStart += heights;
			}
			COMPV_CHECK_CODE_RETURN(threadDisp->wait(taskIds), "Failed to wait for tasks execution");
		}
		else {
			funcPtr(0, outputHeight);
		}

		return COMPV_ERROR_CODE_S_OK;
	}

public:
	static COMPV_ERROR_CODE process(const CompVMatPtr& input, CompVMatPtrPtr output, const CompVMatPtr& map, COMPV_INTERPOLATION_TYPE interType COMPV_DEFAULT(COMPV_INTERPOLATION_TYPE_BILINEAR), const CompVRectFloat32* inputROI COMPV_DEFAULT(nullptr))
	{
		// For now only grayscale images are supported
		COMPV_CHECK_EXP_RETURN(!input || !output || !map || input->isEmpty() || input->elmtInBytes() != sizeof(uint8_t) || input->planeCount() != 1, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

		// interpolation type must be bilinear or nearest
		COMPV_CHECK_EXP_RETURN(interType != COMPV_INTERPOLATION_TYPE_BILINEAR && interType != COMPV_INTERPOLATION_TYPE_NEAREST, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Interpolation type must be bilinear or nearest");

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
		outputSize_.width = COMPV_MATH_ROUNDFU_2_NEAREST_INT(((inputROI_.right - inputROI_.left) + 1), size_t);
		outputSize_.height = COMPV_MATH_ROUNDFU_2_NEAREST_INT(((inputROI_.bottom - inputROI_.top) + 1), size_t);

		// map must contain at least #2 rows (x, y) or (x, y, z) and with exactly n elements (n = (outSize.w*outSize.h)
		const size_t outputElmtCount = (outputSize_.width * outputSize_.height);
		COMPV_CHECK_EXP_RETURN(map->rows() < 2 || map->cols() != outputElmtCount, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Invalid map size");

		// Create output
		COMPV_CHECK_CODE_RETURN(CompVImage::newObj8u(&output_, input->subType(), outputSize_.width, outputSize_.height, input->stride()));

		// Perform interpolation
		switch (interType) {
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
COMPV_ERROR_CODE CompVImageRemap::process(const CompVMatPtr& input, CompVMatPtrPtr output, const CompVMatPtr& map, COMPV_INTERPOLATION_TYPE interType COMPV_DEFAULT(COMPV_INTERPOLATION_TYPE_BILINEAR), const CompVRectFloat32* inputROI COMPV_DEFAULT(nullptr))
{
	COMPV_CHECK_EXP_RETURN(!map, COMPV_ERROR_CODE_E_INVALID_PARAMETER); // other parameters will be tested in the private functions
	switch (map->subType()) {
	case COMPV_SUBTYPE_RAW_FLOAT32: COMPV_CHECK_CODE_RETURN((CompVImageRemapGeneric<compv_float32_t>::process(input, output, map, interType, inputROI))); break;
	case COMPV_SUBTYPE_RAW_FLOAT64: COMPV_CHECK_CODE_RETURN((CompVImageRemapGeneric<compv_float64_t>::process(input, output, map, interType, inputROI))); break;
	default: COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED, "map must constain float32 or float64 indices");  break;
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
