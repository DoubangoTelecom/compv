/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/compv_image.h"
#include "compv/base/image/compv_image_utils.h"
#include "compv/base/image/compv_image_conv_to_yuv444p.h"
#include "compv/base/image/compv_image_conv_to_grayscale.h"
#include "compv/base/image/compv_image_conv_to_rgbx.h"
#include "compv/base/image/compv_image_conv_hsv.h"
#include "compv/base/image/compv_image_integral.h"
#include "compv/base/image/compv_image_scale_bilinear.h"
#include "compv/base/image/compv_image_scale_bicubic.h"
#include "compv/base/image/compv_image_threshold.h"
#include "compv/base/image/compv_image_remap.h"
#include "compv/base/parallel/compv_parallel.h"
#include "compv/base/math/compv_math_utils.h"
#include "compv/base/math/compv_math_histogram.h"
#include "compv/base/math/compv_math_cast.h"
#include "compv/base/math/compv_math_transform.h"
#include "compv/base/math/compv_math_matrix.h"
#include "compv/base/compv_base.h"
#include "compv/base/compv_mem.h"
#include "compv/base/compv_fileutils.h"
#include "compv/base/compv_gradient_fast.h"
#include "compv/base/compv_generic_invoke.h"
#include "compv/base/android/compv_android_fileutils.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#define STB_IMAGE_WRITE_STATIC
#define STB_IMAGE_WRITE_IMPLEMENTATION
COMPV_GCC_DISABLE_WARNINGS_BEGIN("-Wunused-function")
#include "compv/base/image/stb_image.h"
#include "compv/base/image/stb_image_write.h"
COMPV_GCC_DISABLE_WARNINGS_END()

#define COMPV_IMAGE_GAMMA_CORRECTION_SAMPLES_PER_THREAD (40 * 40)

#define COMPV_THIS_CLASSNAME	"CompVImage"

#define COMPV_IMAGE_NEWOBJ_CASE(elmType,pixelFormat) \
		case COMPV_SUBTYPE_PIXELS_##pixelFormat: \
			COMPV_CHECK_CODE_RETURN((CompVMat::newObjAligned<elmType, COMPV_MAT_TYPE_PIXELS, COMPV_SUBTYPE_PIXELS_##pixelFormat>(image, height, width, stride))); \
			return COMPV_ERROR_CODE_S_OK;
#define COMPV_IMAGE_NEWOBJ_SWITCH(elmType, subType) \
	switch (subType) { \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, RGB24); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, BGR24); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, RGBA32); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, BGRA32); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, ABGR32); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, ARGB32); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, RGB565LE); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, RGB565BE); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, BGR565LE); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, BGR565BE); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, HSV); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, Y); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, NV12); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, NV21); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, YUV420P); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, YVU420P); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, YUV422P); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, YUYV422); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, UYVY422); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, YUV444P); \
	default: \
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED); \
		break; \
	}

COMPV_NAMESPACE_BEGIN()

COMPV_ERROR_CODE CompVImage::newObj8u(CompVMatPtrPtr image, COMPV_SUBTYPE subType, size_t width, size_t height, size_t stride COMPV_DEFAULT(0))
{
    COMPV_IMAGE_NEWOBJ_SWITCH(uint8_t, subType);
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImage::newObj16u(CompVMatPtrPtr image, COMPV_SUBTYPE subType, size_t width, size_t height, size_t stride COMPV_DEFAULT(0))
{
    COMPV_IMAGE_NEWOBJ_SWITCH(uint16_t, subType);
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImage::newObj16s(CompVMatPtrPtr image, COMPV_SUBTYPE subType, size_t width, size_t height, size_t stride COMPV_DEFAULT(0))
{
	COMPV_IMAGE_NEWOBJ_SWITCH(int16_t, subType);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImage::read(COMPV_SUBTYPE ePixelFormat, size_t width, size_t height, size_t stride, const char* filePath, CompVMatPtrPtr image)
{
	COMPV_CHECK_EXP_RETURN(!filePath || !width || !height || stride < width || !image, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVBufferPtr buffer;
	COMPV_CHECK_CODE_RETURN(CompVFileUtils::read(filePath, &buffer));
	size_t expectedFileSize;
	COMPV_CHECK_CODE_RETURN(CompVImageUtils::sizeForPixelFormat(ePixelFormat, stride, height, &expectedFileSize));
	if (expectedFileSize != buffer->size()) {
		// FFmpeg requires outputs with even width when converting from RGB to YUV
		if ((stride & 1) || (height & 1)) {
			COMPV_CHECK_CODE_RETURN(CompVImageUtils::sizeForPixelFormat(ePixelFormat, (stride + 1) & ~1, (height + 1) & ~1, &expectedFileSize));
			COMPV_CHECK_EXP_RETURN(expectedFileSize != buffer->size(), COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT, "Size mismatch");
		}
		else {
			COMPV_CHECK_EXP_RETURN(expectedFileSize != buffer->size(), COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT, "Size mismatch");
		}
	}
	// The input stride from the file is probably miss-aligned:
	//	-> use wrap() to make sure the ouput image will be created with the stride possible (aligned on SIMD and GPU pages)
	COMPV_CHECK_CODE_RETURN(CompVImage::wrap(ePixelFormat, buffer->ptr(), width, height, stride, image));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImage::decode(const char* filePath, CompVMatPtrPtr image)
{
	COMPV_CHECK_EXP_RETURN(!filePath || !image, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(
		"This function uses STBI instead of libjpeg-turbo or libpng to decode pictures."
		"This is a quick and dirty way to do it for testing purpose only. You *must not* use it in your final application"
	);
	FILE* file_ = nullptr;
#if COMPV_OS_ANDROID
	if (compv_android_have_assetmgr()) {
		file_ = compv_android_asset_fopen(filePath, "rb");
	}
	else {
		COMPV_DEBUG_INFO_CODE_ONCE("Not using asset manager");
	}
#endif /* COMPV_OS_ANDROID */
	if (!file_ && (file_ = fopen(filePath, "rb")) == nullptr) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Can't open %s", filePath);
		return COMPV_ERROR_CODE_E_FILE_NOT_FOUND;
	}

	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	int width = 0, height = 0, channels = 0;
	stbi_uc* data = stbi_load_from_file(file_, &width, &height, &channels, 0);
	CompVMatPtr image_ = *image;
	COMPV_SUBTYPE subType;
	COMPV_CHECK_EXP_BAIL(!width || !height || (channels != 1 && channels != 3 && channels != 4) || !data, (err = COMPV_ERROR_CODE_E_STBI));
	subType = (channels == 1)
		? (COMPV_SUBTYPE_PIXELS_Y)
		: (channels == 3 ? COMPV_SUBTYPE_PIXELS_RGB24 : COMPV_SUBTYPE_PIXELS_RGBA32);
	COMPV_CHECK_CODE_BAIL(err = CompVImage::newObj8u(&image_, subType, static_cast<size_t>(width), static_cast<size_t>(height)));
	COMPV_CHECK_CODE_BAIL(err = CompVImageUtils::copy(
		subType,
		data, static_cast<size_t>(width), static_cast<size_t>(height), static_cast<size_t>(width),
		image_->ptr<void>(), image_->cols(), image_->rows(), image_->stride()
	));
bail:
	if (COMPV_ERROR_CODE_IS_OK(err)) {
		*image = image_;
	}
	else {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Failed to decode %s", filePath);
	}
	stbi_image_free(data);
	fclose(file_);
	return err;
}

static void CompVImage_stbi_write_func(void *context, void *data, int size)
{
	fwrite(data, 1, size, reinterpret_cast<FILE*>(context));
}

COMPV_ERROR_CODE CompVImage::encode(const char* filePath, const CompVMatPtr& image)
{
	COMPV_CHECK_EXP_RETURN(!filePath || !image, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_EXP_RETURN(
		image->subType() != COMPV_SUBTYPE_PIXELS_RGB24 &&
		image->subType() != COMPV_SUBTYPE_PIXELS_RGBA32 &&
		image->subType() != COMPV_SUBTYPE_PIXELS_Y && 
		(image->elmtInBytes() != sizeof(uint8_t) || image->planeCount() != 1),
		COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT);
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(
		"This function uses STBI instead of libjpeg-turbo or libpng to decode pictures."
		"This is a quick and dirty way to do it for testing purpose only. You *must not* use it in your final application"
	);

	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Encoding %s file... (w=%zu, h=%zu, s=%zu)", filePath, image->cols(), image->rows(), image->stride());

	std::string filePath_ = filePath;
	const COMPV_IMAGE_FORMAT format = CompVFileUtils::getImageFormat(filePath);
	int(*write_fn)(stbi_write_func *func, void *context, int x, int y, int comp, const void *data, int stride_bytes)
		= stbi_write_png_to_func;
	switch (format) {
#if 0 // disabled for now -> no support for stridding
	case COMPV_IMAGE_FORMAT_JPEG:
		write_fn = stbi_write_jpg_to_func;
		break;
	case COMPV_IMAGE_FORMAT_BMP:
		write_fn = stbi_write_bmp_to_func;
		break;
#endif
	default:
	case COMPV_IMAGE_FORMAT_PNG:
		write_fn = stbi_write_png_to_func;
		if (filePath_.size() <= 4 || filePath_.rfind(".png") != (filePath_.size() - 4)) {
			filePath_ += ".png";
		}
		break;
	}

	FILE* file_ = nullptr;
	if (!(file_ = CompVFileUtils::open(filePath_.c_str(), "wb+")) ) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Can't create %s", filePath_.c_str());
		return COMPV_ERROR_CODE_E_FILE_NOT_FOUND;
	}

	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	const COMPV_SUBTYPE subType =
		(image->subType() == COMPV_SUBTYPE_PIXELS_Y || (image->elmtInBytes() == sizeof(uint8_t) && image->planeCount() == 1))
		? COMPV_SUBTYPE_PIXELS_Y
		: image->subType();
	int comp = 1;
	switch (subType){
	case COMPV_SUBTYPE_PIXELS_Y: comp = 1; break;
	case COMPV_SUBTYPE_PIXELS_RGB24: comp = 3; break;
	case COMPV_SUBTYPE_PIXELS_RGBA32: comp = 4; break;
	default:
		COMPV_ASSERT(false); // never happens, already checked above
		break;
	}
	
	COMPV_CHECK_EXP_BAIL(write_fn(CompVImage_stbi_write_func, file_,
		static_cast<int>(image->cols()), static_cast<int>(image->rows()), comp,
		image->ptr<const void>(),
		static_cast<int>(image->strideInBytes())) != 1,
		(err = COMPV_ERROR_CODE_E_STBI), "Failed to write file");
bail:
	if (file_) {
		fclose(file_);
	}
	return err;
}

COMPV_ERROR_CODE CompVImage::write(const char* filePath, const CompVMatPtr& image)
{
	COMPV_CHECK_EXP_RETURN(!image || image->isEmpty() || !filePath, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	FILE* file = fopen(filePath, "wb+");
	COMPV_CHECK_EXP_RETURN(!file, COMPV_ERROR_CODE_E_FILE_NOT_FOUND, "Failed to catere file");

	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Writing %s file... (w=%zu, h=%zu, s=%zu)", filePath, image->cols(), image->rows(), image->stride());
	const int32_t planes = static_cast<int32_t>(image->planeCount());
	for (int32_t plane = 0; plane < planes; ++plane) {
		size_t planeHeight = image->rows(plane);
		size_t planeWidth = image->rowInBytes(plane);
		size_t planeStride = image->strideInBytes(plane);
		const uint8_t* planePtr = image->ptr<const uint8_t>(0, 0, plane);
		for (size_t i = 0; i < planeHeight; ++i) {
			COMPV_ASSERT(fwrite(planePtr, 1, planeWidth, file) == planeWidth);
			planePtr += planeStride;
		}
	}
	
	fclose(file);

	return COMPV_ERROR_CODE_S_OK;
}

// "uvPixelStride" should be 1 for planar and 2 for packed/interleaved formats. Used by Android Camera2 to fake NV12/NV21 (a.k.a YUV420SP) as YUV20P
COMPV_ERROR_CODE CompVImage::wrapYuv(
	COMPV_SUBTYPE ePixelFormat,
	const void* yPtr,
	const void* uPtr,
	const void* vPtr,
	const size_t width,
	const size_t height,
	const size_t yStrideInBytes,
	const size_t uStrideInBytes,
	const size_t vStrideInBytes,
	CompVMatPtrPtr outImage,
	const size_t uvPixelStrideInBytes COMPV_DEFAULT(0),
	const size_t outImageStrideInBytes COMPV_DEFAULT(0)
)
{
	COMPV_CHECK_EXP_RETURN(!yPtr || !uPtr || !vPtr || !width || !height || !yStrideInBytes || !uStrideInBytes || !vStrideInBytes || yStrideInBytes < width,
		COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_EXP_RETURN(uvPixelStrideInBytes != 0 && uvPixelStrideInBytes != 1 && uvPixelStrideInBytes != 2,
		COMPV_ERROR_CODE_E_INVALID_PARAMETER, "uvPixelStride should be 1 for planar and 2 for packed/interleaved formats. Set to zero for auto detection");

	switch (ePixelFormat) {
		case COMPV_SUBTYPE_PIXELS_NV12:
		case COMPV_SUBTYPE_PIXELS_NV21:
		case COMPV_SUBTYPE_PIXELS_YUV420P:
		case COMPV_SUBTYPE_PIXELS_YVU420P:
		case COMPV_SUBTYPE_PIXELS_YUV422P:
		case COMPV_SUBTYPE_PIXELS_YUV444P: {
			const bool outPacked = (ePixelFormat == COMPV_SUBTYPE_PIXELS_NV12 || ePixelFormat == COMPV_SUBTYPE_PIXELS_NV21);
			const bool inPackedUV = (outPacked || uvPixelStrideInBytes == 2);
			COMPV_CHECK_EXP_RETURN(inPackedUV && (uStrideInBytes != vStrideInBytes), COMPV_ERROR_CODE_E_INVALID_PARAMETER, "For interleaved UV uStrideInBytes must be equal to vStrideInBytes");
			COMPV_CHECK_EXP_RETURN(inPackedUV && (std::abs((long long)(reinterpret_cast<const uintptr_t>(uPtr) - reinterpret_cast<const uintptr_t>(vPtr))) != 1), COMPV_ERROR_CODE_E_INVALID_PARAMETER, "For interleaved UV distance(uPtr, vPtr) must be equal to 1");
			CompVMatPtr outImage_; // do not set to "*outImage" as the can try to override the input (input == output)
			size_t bestStride = outImageStrideInBytes;
			if (bestStride < width) { // Compute newStride for the wrapped image is not defined or invalid
				COMPV_CHECK_CODE_RETURN(CompVImageUtils::bestStride(width, &bestStride));
			}
			COMPV_CHECK_CODE_RETURN(CompVImage::newObj8u(&outImage_, ePixelFormat, width, height, bestStride));
			COMPV_CHECK_EXP_RETURN(uStrideInBytes < outImage_->rowInBytes(COMPV_PLANE_UV) || vStrideInBytes < outImage_->rowInBytes(COMPV_PLANE_UV), COMPV_ERROR_CODE_E_INVALID_PARAMETER, "uvStride too short to be valid");
			// Copy Luma - Y
			COMPV_CHECK_CODE_RETURN(CompVImageUtils::copy(
				COMPV_SUBTYPE_PIXELS_Y,
				yPtr, width, height, yStrideInBytes,
				outImage_->ptr<void>(0, 0, COMPV_PLANE_Y), outImage_->rowInBytes(COMPV_PLANE_Y), outImage_->rows(COMPV_PLANE_Y), outImage_->strideInBytes(COMPV_PLANE_Y)
			));
			// Copy Luma - UV
			const size_t uvHeight = outImage_->rows(COMPV_PLANE_UV);
			if (inPackedUV) { // Interleaved/packed UV
				COMPV_ASSERT(uStrideInBytes == vStrideInBytes); // already tested, see above
				const bool uFirst = (reinterpret_cast<const uintptr_t>(uPtr) < reinterpret_cast<const uintptr_t>(vPtr)); // For NV21: V is first: https://www.fourcc.org/pixel-format/yuv-nv21/
				const void* uvPtr = uFirst ? uPtr : vPtr; // distance is equal to #1 and take the min
				const size_t uvStrideInBytes = uStrideInBytes;
				if (outPacked) { // In and Out are both packed -> copy "AS IS"
					const size_t uvWidthInBytes = outImage_->rowInBytes(COMPV_PLANE_UV); // Same width as they're both packed
					COMPV_CHECK_CODE_RETURN(CompVImageUtils::copy(
						COMPV_SUBTYPE_PIXELS_Y,
						uvPtr, uvWidthInBytes, uvHeight, uvStrideInBytes,
						outImage_->ptr<void>(0, 0, COMPV_PLANE_UV), outImage_->rowInBytes(COMPV_PLANE_UV), outImage_->rows(COMPV_PLANE_UV), outImage_->strideInBytes(COMPV_PLANE_UV)
					));
				}
				else { // In packed and Out planer -> unpack and copy
					CompVMatPtr ptr8uImgU, ptr8uImgV;
					const size_t uvWidthInBytes = width; // Out is planar while In is packed -> do not use "outImage_->rowInBytes(COMPV_PLANE_UV)", same width as luma
					COMPV_CHECK_CODE_RETURN(CompVImage::newObj8u(&ptr8uImgU, COMPV_SUBTYPE_PIXELS_Y, (uvWidthInBytes + 1) >> 1, uvHeight, (uvStrideInBytes + 1) >> 1));
					COMPV_CHECK_CODE_RETURN(CompVImage::newObj8u(&ptr8uImgV, COMPV_SUBTYPE_PIXELS_Y, (uvWidthInBytes + 1) >> 1, uvHeight, (uvStrideInBytes + 1) >> 1));
					COMPV_CHECK_CODE_RETURN(CompVMem::unpack2(ptr8uImgU->ptr<uint8_t>(), ptr8uImgV->ptr<uint8_t>(),
						reinterpret_cast<const compv_uint8x2_t*>(uvPtr), (uvWidthInBytes + 1) >> 1, uvHeight, (uvStrideInBytes + 1) >> 1));
					const int planeU = uFirst ? COMPV_PLANE_U : COMPV_PLANE_V;
					const int planeV = uFirst ? COMPV_PLANE_V : COMPV_PLANE_U;
					COMPV_CHECK_CODE_RETURN(CompVImageUtils::copy(
						COMPV_SUBTYPE_PIXELS_Y,
						ptr8uImgU->ptr<const void>(), ptr8uImgU->cols(), ptr8uImgU->rows(), ptr8uImgU->stride(),
						outImage_->ptr<void>(0, 0, planeU), outImage_->cols(planeU), outImage_->rows(planeU), outImage_->stride(planeU)
					));
					COMPV_CHECK_CODE_RETURN(CompVImageUtils::copy(
						COMPV_SUBTYPE_PIXELS_Y,
						ptr8uImgV->ptr<const void>(), ptr8uImgV->cols(), ptr8uImgV->rows(), ptr8uImgV->stride(),
						outImage_->ptr<void>(0, 0, planeV), outImage_->cols(planeV), outImage_->rows(planeV), outImage_->stride(planeV)
					));
				}
			}
			else { // Planar UV for both input and output
				const size_t uvWidthInBytes = outImage_->rowInBytes(COMPV_PLANE_UV); // Same width as they're both planar
				COMPV_CHECK_CODE_RETURN(CompVImageUtils::copy(
					COMPV_SUBTYPE_PIXELS_Y,
					uPtr, uvWidthInBytes, uvHeight, uStrideInBytes,
					outImage_->ptr<void>(0, 0, COMPV_PLANE_U), outImage_->rowInBytes(COMPV_PLANE_U), outImage_->rows(COMPV_PLANE_U), outImage_->strideInBytes(COMPV_PLANE_U)
				));
				COMPV_CHECK_CODE_RETURN(CompVImageUtils::copy(
					COMPV_SUBTYPE_PIXELS_Y,
					vPtr, uvWidthInBytes, uvHeight, vStrideInBytes,
					outImage_->ptr<void>(0, 0, COMPV_PLANE_V), outImage_->rowInBytes(COMPV_PLANE_V), outImage_->rows(COMPV_PLANE_V), outImage_->strideInBytes(COMPV_PLANE_V)
				));
			}

			*outImage = outImage_;

			return COMPV_ERROR_CODE_S_OK;
		}
		default: {
			COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Wrapping %s not supported yet", CompVGetSubtypeString(ePixelFormat));
			return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
		}
	}
}

COMPV_ERROR_CODE CompVImage::wrap(COMPV_SUBTYPE ePixelFormat, const void* dataPtr, const size_t dataWidth, const size_t dataHeight, const size_t dataStride, CompVMatPtrPtr image, const size_t imageStride COMPV_DEFAULT(0))
{
	COMPV_CHECK_EXP_RETURN(!dataPtr || !dataWidth || !dataHeight || dataStride < dataWidth || !image, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	// Compute best stride
	size_t bestStride = imageStride;
	if (bestStride < dataWidth) { // Compute newStride for the wrapped image is not defined or invalid
		COMPV_CHECK_CODE_RETURN(CompVImageUtils::bestStride(dataWidth, &bestStride));
	}
	CompVMatPtr image_ = ((*image) && ((*image)->data<const void>() == dataPtr || !(*image)->isMemoryOwed())) ? nullptr : *image;
	COMPV_CHECK_CODE_RETURN(CompVImage::newObj8u(&image_, ePixelFormat, dataWidth, dataHeight, bestStride)
		, "Failed to allocate new image");
	
	// Copy data using different strides
	COMPV_CHECK_CODE_RETURN(CompVImageUtils::copy(ePixelFormat,
		dataPtr, dataWidth, dataHeight, dataStride,
		(void*)image_->ptr(), image_->cols(), image_->rows(), image_->stride()), "Failed to copy image"); // copy data
	
	*image = image_;

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImage::clone(const CompVMatPtr& imageIn, CompVMatPtrPtr imageOut)
{
	COMPV_CHECK_EXP_RETURN(!imageIn || imageIn->isEmpty() || !imageOut || imageIn->type() != COMPV_MAT_TYPE_PIXELS, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_CODE_RETURN(CompVImage::wrap(imageIn->subType(), imageIn->ptr(), imageIn->cols(), imageIn->rows(), imageIn->stride(), imageOut, imageIn->stride()));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImage::crop(const CompVMatPtr& imageIn, const CompVRectFloat32& roi, CompVMatPtrPtr imageOut)
{
	COMPV_CHECK_EXP_RETURN(!imageIn || imageIn->isEmpty() || roi.isEmpty() || !imageOut || imageIn->type() != COMPV_MAT_TYPE_PIXELS || roi.left < 0.f || roi.right < 0.f || roi.top < 0.f || roi.bottom < 0.f, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	
	const size_t colStart = COMPV_MATH_ROUNDFU_2_NEAREST_INT(roi.left, size_t);
	COMPV_CHECK_EXP_RETURN(colStart > imageIn->cols(), COMPV_ERROR_CODE_E_OUT_OF_BOUND);
	const size_t colEnd = COMPV_MATH_ROUNDFU_2_NEAREST_INT(roi.right, size_t);
	COMPV_CHECK_EXP_RETURN(colEnd > imageIn->cols() || colStart >= colEnd, COMPV_ERROR_CODE_E_OUT_OF_BOUND);
	const size_t colCount = (colEnd - colStart);
	
	const size_t rowStart = COMPV_MATH_ROUNDFU_2_NEAREST_INT(roi.top, size_t);
	COMPV_CHECK_EXP_RETURN(rowStart > imageIn->rows(), COMPV_ERROR_CODE_E_OUT_OF_BOUND);
	const size_t rowEnd = COMPV_MATH_ROUNDFU_2_NEAREST_INT(roi.bottom, size_t);
	COMPV_CHECK_EXP_RETURN(rowEnd > imageIn->rows() || rowStart >= rowEnd, COMPV_ERROR_CODE_E_OUT_OF_BOUND);
	const size_t rowCount = (rowEnd - rowStart) /*& -2*/;
	
	CompVMatPtr imageOut_ = (*imageOut == imageIn) ? nullptr : *imageOut;
	COMPV_CHECK_CODE_RETURN(CompVImage::newObj8u(&imageOut_, imageIn->subType(), colCount, rowCount, imageIn->stride())); //!\\ must use same stride because we're using memcpy instead of row by row copy

	const int numPlanes = static_cast<int>(imageIn->planeCount());
	const COMPV_SUBTYPE pixelFormat = imageIn->subType();
	size_t rowStartInPlane, colStartInPlane;
	for (int planeId = 0;  planeId < numPlanes; ++planeId) {
		COMPV_CHECK_CODE_RETURN(CompVImageUtils::planeSizeForPixelFormat(pixelFormat, planeId, colStart, rowStart, &colStartInPlane, &rowStartInPlane));
		COMPV_CHECK_CODE_RETURN(CompVMem::copy(
			imageOut_->ptr<void>(0, 0, planeId),
			imageIn->ptr<const void>(rowStartInPlane, colStartInPlane, planeId),
			imageOut_->planeSizeInBytes(planeId) - (colStartInPlane * imageOut_->elmtInBytes())
		));
	}

	*imageOut = imageOut_;

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImage::unpack(const CompVMatPtr& imageIn, CompVMatPtrVector& outputs)
{
	COMPV_CHECK_EXP_RETURN(!imageIn || imageIn->isEmpty(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	switch (imageIn->subType()) {
	case COMPV_SUBTYPE_PIXELS_HSV:
	case COMPV_SUBTYPE_PIXELS_HSL:
	case COMPV_SUBTYPE_PIXELS_RGB24:
	case COMPV_SUBTYPE_PIXELS_BGR24: {
		CompVMatPtr ptr8uImg0, ptr8uImg1, ptr8uImg2;
		if (outputs.size() == 3) {
			ptr8uImg0 = outputs[0];
			ptr8uImg1 = outputs[1];
			ptr8uImg2 = outputs[2];
		}
		else {
			outputs.resize(3);
		}
		const size_t width = imageIn->cols();
		const size_t height = imageIn->rows();
		const size_t stride = imageIn->stride();
		if (!ptr8uImg0 || ptr8uImg0->cols() != width || ptr8uImg0->rows() != height || ptr8uImg0->stride() != stride || ptr8uImg0->elmtInBytes() != sizeof(uint8_t) || ptr8uImg0->planeCount() != 1) {
			COMPV_CHECK_CODE_RETURN(CompVImage::newObj8u(&ptr8uImg0, COMPV_SUBTYPE_PIXELS_Y, width, height, stride));
		}
		if (!ptr8uImg1 || ptr8uImg1->cols() != width || ptr8uImg1->rows() != height || ptr8uImg1->stride() != stride || ptr8uImg1->elmtInBytes() != sizeof(uint8_t) || ptr8uImg1->planeCount() != 1) {
			COMPV_CHECK_CODE_RETURN(CompVImage::newObj8u(&ptr8uImg1, COMPV_SUBTYPE_PIXELS_Y, width, height, stride));
		}
		if (!ptr8uImg2 || ptr8uImg2->cols() != width || ptr8uImg2->rows() != height || ptr8uImg2->stride() != stride || ptr8uImg2->elmtInBytes() != sizeof(uint8_t) || ptr8uImg2->planeCount() != 1) {
			COMPV_CHECK_CODE_RETURN(CompVImage::newObj8u(&ptr8uImg2, COMPV_SUBTYPE_PIXELS_Y, width, height, stride));
		}
		COMPV_CHECK_CODE_RETURN(CompVMem::unpack3(ptr8uImg0->ptr<uint8_t>(), ptr8uImg1->ptr<uint8_t>(), ptr8uImg2->ptr<uint8_t>(),
			imageIn->ptr<const compv_uint8x3_t>(), width, height, stride));
		outputs[0] = ptr8uImg0;
		outputs[1] = ptr8uImg1;
		outputs[2] = ptr8uImg2;
		return COMPV_ERROR_CODE_S_OK;
	}
	case COMPV_SUBTYPE_PIXELS_NV12:
	case COMPV_SUBTYPE_PIXELS_NV21: 
	case COMPV_SUBTYPE_PIXELS_YUV420P:
	case COMPV_SUBTYPE_PIXELS_YVU420P:
	case COMPV_SUBTYPE_PIXELS_YUV422P: 
	case COMPV_SUBTYPE_PIXELS_YUV444P: {
		CompVMatPtr ptr8uImgY, ptr8uImgU, ptr8uImgV;
		const bool packedUV = (imageIn->subType() == COMPV_SUBTYPE_PIXELS_NV12 || imageIn->subType() == COMPV_SUBTYPE_PIXELS_NV21);
		const int planeU = packedUV ? COMPV_PLANE_UV : COMPV_PLANE_U;
		const int planeV = packedUV ? COMPV_PLANE_UV : COMPV_PLANE_V;
		if (outputs.size() == 3) {
			ptr8uImgY = outputs[0];
			ptr8uImgU = outputs[1];
			ptr8uImgV = outputs[2];
		}
		else {
			outputs.resize(3);
		}
		// Wrap Luma - Y
		COMPV_CHECK_CODE_RETURN(CompVImage::wrap(COMPV_SUBTYPE_PIXELS_Y,
			imageIn->ptr<uint8_t>(0, 0, COMPV_PLANE_Y), imageIn->cols(COMPV_PLANE_Y), imageIn->rows(COMPV_PLANE_Y), imageIn->stride(COMPV_PLANE_Y),
			&ptr8uImgY, imageIn->stride(COMPV_PLANE_Y)));
		// UV
		COMPV_CHECK_CODE_RETURN(CompVImage::newObj8u(&ptr8uImgU, COMPV_SUBTYPE_PIXELS_Y, imageIn->cols(planeU), imageIn->rows(planeU), imageIn->stride(planeU)));
		COMPV_CHECK_CODE_RETURN(CompVImage::newObj8u(&ptr8uImgV, COMPV_SUBTYPE_PIXELS_Y, imageIn->cols(planeV), imageIn->rows(planeV), imageIn->stride(planeV)));
		if (packedUV) { // Interleaved UV
			COMPV_CHECK_CODE_RETURN(CompVMem::unpack2(ptr8uImgU->ptr<uint8_t>(), ptr8uImgV->ptr<uint8_t>(),
				imageIn->ptr<const compv_uint8x2_t>(0, 0, COMPV_PLANE_UV), imageIn->cols(COMPV_PLANE_UV), imageIn->rows(COMPV_PLANE_UV), imageIn->stride(COMPV_PLANE_UV)));
		}
		else { // Planar UV
			COMPV_CHECK_CODE_RETURN(CompVImage::wrap(COMPV_SUBTYPE_PIXELS_Y,
				imageIn->ptr<uint8_t>(0, 0, planeU), imageIn->cols(planeU), imageIn->rows(planeU), imageIn->stride(planeU),
				&ptr8uImgU, imageIn->stride(planeU)));
			COMPV_CHECK_CODE_RETURN(CompVImage::wrap(COMPV_SUBTYPE_PIXELS_Y,
				imageIn->ptr<uint8_t>(0, 0, planeV), imageIn->cols(planeV), imageIn->rows(planeV), imageIn->stride(planeV),
				&ptr8uImgV, imageIn->stride(planeV)));
		}
		// Set result
		outputs[0] = ptr8uImgY;
		outputs[1] = ptr8uImgU;
		outputs[2] = ptr8uImgV;
		return COMPV_ERROR_CODE_S_OK;
	}
	default:
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Splitting %s not supported yet", CompVGetSubtypeString(imageIn->subType()));
		return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
	}
}

// For now we only support: pack(#3,#8u,#1dim)
COMPV_ERROR_CODE CompVImage::pack(const CompVMatPtrVector& inputs, const COMPV_SUBTYPE& pixelFormat, CompVMatPtrPtr output)
{
	COMPV_CHECK_EXP_RETURN(inputs.empty() || !output, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	switch (pixelFormat) {
	case COMPV_SUBTYPE_PIXELS_HSV:
	case COMPV_SUBTYPE_PIXELS_HSL:
	case COMPV_SUBTYPE_PIXELS_RGB24:
	case COMPV_SUBTYPE_PIXELS_BGR24: {
		// Make sure there are #3 inputs and they are 8u 1dim values and with same size
		COMPV_CHECK_EXP_RETURN(inputs.size() != 3, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Requires #3 lanes");
		const size_t width = inputs[0]->cols();
		const size_t height = inputs[0]->rows();
		const size_t stride = inputs[0]->stride();
		for (const auto& it : inputs) {
			COMPV_CHECK_EXP_RETURN(it->cols() != width || it->rows() != height || it->stride() != stride, COMPV_ERROR_CODE_E_INVALID_IMAGE_FORMAT, "Invalid size. All inputs must have same size.");
			COMPV_CHECK_EXP_RETURN(it->elmtInBytes() != sizeof(uint8_t) || it->planeCount() != 1, COMPV_ERROR_CODE_E_INVALID_IMAGE_FORMAT, "Inputs must be #8 bits values and #1 dimension.");
		}
		// Packing
		CompVMatPtr& output_ = *output;
		COMPV_CHECK_CODE_RETURN(CompVImage::newObj8u(&output_, pixelFormat, width, height, stride));
		COMPV_CHECK_CODE_RETURN(CompVMem::pack3(output_->ptr<compv_uint8x3_t>(), inputs[0]->ptr<const uint8_t>(), inputs[1]->ptr<const uint8_t>(), inputs[2]->ptr<const uint8_t>(),
			width, height, stride));

		return COMPV_ERROR_CODE_S_OK;
	}
	case COMPV_SUBTYPE_PIXELS_NV12:
	case COMPV_SUBTYPE_PIXELS_NV21:
	case COMPV_SUBTYPE_PIXELS_YUV420P:
	case COMPV_SUBTYPE_PIXELS_YVU420P:
	case COMPV_SUBTYPE_PIXELS_YUV422P:
	case COMPV_SUBTYPE_PIXELS_YUV444P: {
		// Make sure there are #3 inputs and they are 8u 1dim values
		COMPV_CHECK_EXP_RETURN(inputs.size() != 3, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Requires #3 lanes");
		CompVMatPtr& output_ = *output;
		COMPV_CHECK_CODE_RETURN(CompVImage::newObj8u(&output_, pixelFormat, inputs[0]->cols(), inputs[0]->rows(), inputs[0]->stride()));
		const bool packedUV = (pixelFormat == COMPV_SUBTYPE_PIXELS_NV12 || pixelFormat == COMPV_SUBTYPE_PIXELS_NV21);
		const int planeU = packedUV ? COMPV_PLANE_UV : COMPV_PLANE_U;
		const int planeV = packedUV ? COMPV_PLANE_UV : COMPV_PLANE_V;
		for (size_t i = 0; i < 3; ++i) { // make sure the memory layout matches
			const CompVMatPtr& imagePlane = inputs[i];
			COMPV_CHECK_EXP_RETURN(imagePlane->elmtInBytes() != sizeof(uint8_t) || imagePlane->planeCount() != 1, COMPV_ERROR_CODE_E_INVALID_IMAGE_FORMAT, "Inputs must be #8 bits values and #1 dimension.");
			const int planeId = (i == 0) ? COMPV_PLANE_Y : (i == 1 ? planeU : planeV);
			COMPV_CHECK_EXP_RETURN(
				output_->cols(planeId) != imagePlane->cols()
				|| output_->rows(planeId) != imagePlane->rows()
				|| (packedUV && output_->stride(planeId) != imagePlane->stride()) // Non-packed formats use "CompVImageUtils::copy" which support having different strides
				, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Invalid size for the planes"
			);
		}		
		// Copy Luma - Y
		COMPV_CHECK_CODE_RETURN(CompVImageUtils::copy(
			COMPV_SUBTYPE_PIXELS_Y,
			inputs[0]->ptr<const void>(), inputs[0]->cols(), inputs[0]->rows(), inputs[0]->stride(),
			output_->ptr<void>(0, 0, COMPV_PLANE_Y), output_->cols(COMPV_PLANE_Y), output_->rows(COMPV_PLANE_Y), output_->stride(COMPV_PLANE_Y)
		));
		// Copy UV
		if (packedUV) { // Interleaved UV
			COMPV_CHECK_CODE_RETURN(CompVMem::pack2(
				output_->ptr<compv_uint8x2_t>(0, 0, COMPV_PLANE_UV),
				inputs[1]->ptr<uint8_t>(), inputs[2]->ptr<uint8_t>(), 
				output_->cols(COMPV_PLANE_UV), output_->rows(COMPV_PLANE_UV), output_->stride(COMPV_PLANE_UV))
			);
		}
		else { // Planar UV
			COMPV_CHECK_CODE_RETURN(CompVImageUtils::copy(
				COMPV_SUBTYPE_PIXELS_Y,
				inputs[1]->ptr<uint8_t>(), inputs[1]->cols(), inputs[1]->rows(), inputs[1]->stride(),
				output_->ptr<void>(0, 0, planeU), output_->cols(planeU), output_->rows(planeU), output_->stride(planeU)
			));
			COMPV_CHECK_CODE_RETURN(CompVImageUtils::copy(
				COMPV_SUBTYPE_PIXELS_Y,
				inputs[2]->ptr<uint8_t>(), inputs[2]->cols(), inputs[2]->rows(), inputs[2]->stride(),
				output_->ptr<void>(0, 0, planeV), output_->cols(planeV), output_->rows(planeV), output_->stride(planeV)
			));
		}

		return COMPV_ERROR_CODE_S_OK;
	}
	default:
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Splitting %s not supported yet", CompVGetSubtypeString(pixelFormat));
		return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
	}

	return  COMPV_ERROR_CODE_S_OK;
}

// map = (x, y) values
// map must contain at least #2 rows (x, y) or (x, y, z) and with exactly n elements (n = (outSize.w*outSize.h)
COMPV_ERROR_CODE CompVImage::remap(const CompVMatPtr& imageIn, CompVMatPtrPtr output, const CompVMatPtr& map, COMPV_INTERPOLATION_TYPE interType COMPV_DEFAULT(COMPV_INTERPOLATION_TYPE_BILINEAR), const CompVRectFloat32* inputROI COMPV_DEFAULT(nullptr))
{
	COMPV_CHECK_CODE_RETURN(CompVImageRemap::process(imageIn, output, map, interType, inputROI));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImage::convert(const CompVMatPtr& imageIn, COMPV_SUBTYPE pixelFormatOut, CompVMatPtrPtr imageOut)
{
	COMPV_CHECK_EXP_RETURN(!imageIn || imageIn->isEmpty() || !imageOut || imageIn->type() != COMPV_MAT_TYPE_PIXELS, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	switch (pixelFormatOut) {
	case COMPV_SUBTYPE_PIXELS_YUV444P:
		COMPV_CHECK_CODE_RETURN(CompVImageConvToYUV444P::process(imageIn, imageOut));
		return COMPV_ERROR_CODE_S_OK;
	case COMPV_SUBTYPE_PIXELS_Y:
		COMPV_CHECK_CODE_RETURN(CompVImageConvToGrayscale::process(imageIn, imageOut));
		return COMPV_ERROR_CODE_S_OK;
	case COMPV_SUBTYPE_PIXELS_RGBA32:
	case COMPV_SUBTYPE_PIXELS_RGB24:
		COMPV_CHECK_CODE_RETURN(CompVImageConvToRGBx::process(imageIn, pixelFormatOut, imageOut));
		return COMPV_ERROR_CODE_S_OK;
	case COMPV_SUBTYPE_PIXELS_HSV:
		COMPV_CHECK_CODE_RETURN(CompVImageConvToHSV::process(imageIn, imageOut));
		return COMPV_ERROR_CODE_S_OK;
	default:
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Chroma conversion not supported: %s -> %s", CompVGetSubtypeString(imageIn->subType()), CompVGetSubtypeString(pixelFormatOut));
		return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
	}
}

COMPV_ERROR_CODE CompVImage::convertGrayscale(const CompVMatPtr& imageIn, CompVMatPtrPtr imageGray)
{
	// Input parameters will be checked in 'convert'
	COMPV_CHECK_CODE_RETURN(CompVImage::convert(imageIn, COMPV_SUBTYPE_PIXELS_Y, imageGray));
	return COMPV_ERROR_CODE_S_OK;
}

// This function is faster when the input data is planar (or semi-planar) YUV as we'll just reshape the data.
// It requires the input to be equal to the output to avoid copying, this is whay we require a single parameter
COMPV_ERROR_CODE CompVImage::convertGrayscaleFast(CompVMatPtr& imageInOut)
{
	// Input parameters will be checked in 'convert'
	COMPV_CHECK_CODE_RETURN(CompVImage::convert(imageInOut, COMPV_SUBTYPE_PIXELS_Y, &imageInOut));
	return COMPV_ERROR_CODE_S_OK;
}

// https://en.wikipedia.org/wiki/Image_histogram
COMPV_ERROR_CODE CompVImage::histogramBuild(const CompVMatPtr& input, CompVMatPtrPtr histogram)
{
	COMPV_CHECK_CODE_RETURN(CompVMathHistogram::build(input, histogram));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImage::gradientX(const CompVMatPtr& input, CompVMatPtrPtr outputX, bool outputFloat COMPV_DEFAULT(false))
{
	if (outputFloat) {
		COMPV_CHECK_CODE_RETURN(CompVGradientFast::gradX<compv_float32_t>(input, outputX));
	}
	else {
		COMPV_CHECK_CODE_RETURN(CompVGradientFast::gradX<int16_t>(input, outputX));
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImage::gradientY(const CompVMatPtr& input, CompVMatPtrPtr outputY, bool outputFloat COMPV_DEFAULT(false))
{
	if (outputFloat) {
		COMPV_CHECK_CODE_RETURN(CompVGradientFast::gradY<compv_float32_t>(input, outputY));
	}
	else {
		COMPV_CHECK_CODE_RETURN(CompVGradientFast::gradY<int16_t>(input, outputY));
	}
	return COMPV_ERROR_CODE_S_OK;
}

// Project the image on the vertical axis (sum cols)
COMPV_ERROR_CODE CompVImage::histogramBuildProjectionY(const CompVMatPtr& dataIn, CompVMatPtrPtr ptr32sProjection)
{
	COMPV_CHECK_CODE_RETURN(CompVMathHistogram::buildProjectionY(dataIn, ptr32sProjection));
	return COMPV_ERROR_CODE_S_OK;
}

// Project the image on the horizontal axis (sum rows)
COMPV_ERROR_CODE CompVImage::histogramBuildProjectionX(const CompVMatPtr& dataIn, CompVMatPtrPtr ptr32sProjection)
{
	COMPV_CHECK_CODE_RETURN(CompVMathHistogram::buildProjectionX(dataIn, ptr32sProjection));
	return COMPV_ERROR_CODE_S_OK;
}

// https://en.wikipedia.org/wiki/Histogram_equalization
COMPV_ERROR_CODE CompVImage::histogramEqualiz(const CompVMatPtr& input, CompVMatPtrPtr output)
{
	COMPV_CHECK_CODE_RETURN(CompVMathHistogram::equaliz(input, output));
	return COMPV_ERROR_CODE_S_OK;
}

// https://en.wikipedia.org/wiki/Histogram_equalization
COMPV_ERROR_CODE CompVImage::histogramEqualiz(const CompVMatPtr& input, const CompVMatPtr& histogram, CompVMatPtrPtr output)
{
	COMPV_CHECK_CODE_RETURN(CompVMathHistogram::equaliz(input, histogram, output));
	return COMPV_ERROR_CODE_S_OK;
}

// https://en.wikipedia.org/wiki/Gamma_correction
// A = 1
COMPV_ERROR_CODE CompVImage::gammaCorrection(const CompVMatPtr& input, const double& gamma, CompVMatPtrPtr output)
{
	COMPV_CHECK_EXP_RETURN(!input || input->elmtInBytes() != sizeof(uint8_t) || input->planeCount() != 1 || !output, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("If gamma value is constant over time then, you should pre-compute LUT once and use the overrided function");
	compv_uint8x256_t gammaLUT;
	static const double scale = 1.0 / 255.0;
	for (int i = 0; i < 256; ++i) {
		const double v = std::pow((static_cast<double>(i) * scale), gamma) * 255.0;
		gammaLUT[i] = COMPV_MATH_ROUNDFU_2_NEAREST_INT(v, uint8_t);
	}
	COMPV_CHECK_CODE_RETURN(CompVImage::gammaCorrection(input, gammaLUT, output));
	return COMPV_ERROR_CODE_S_OK;
}

// https://en.wikipedia.org/wiki/Gamma_correction
// A = 1
COMPV_ERROR_CODE CompVImage::gammaCorrection(const CompVMatPtr& input, const compv_uint8x256_t& gammaLUT, CompVMatPtrPtr output)
{
	COMPV_CHECK_EXP_RETURN(!input || input->elmtInBytes() != sizeof(uint8_t) || input->planeCount() != 1 || !output, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	const size_t width = input->cols();
	const size_t height = input->rows();
	const size_t stride = input->stride();

	CompVMatPtr output_ = (*output == input) ? nullptr : *output; // this function doesn't allow output to be equal to input
	COMPV_CHECK_CODE_RETURN(CompVImage::newObj8u(&output_, COMPV_SUBTYPE_PIXELS_Y, width, height, stride));

	auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		const uint8_t* ptrIn = input->ptr<const uint8_t>(ystart);
		uint8_t* ptrOut = output_->ptr<uint8_t>(ystart);
		for (size_t j = ystart; j < yend; ++j) {
			for (size_t i = 0; i < width; ++i) {
				ptrOut[i] = gammaLUT[ptrIn[i]];
			}
			ptrOut += stride;
			ptrIn += stride;
		}
		return COMPV_ERROR_CODE_S_OK;
	};
	
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtr,
		width,
		height,
		COMPV_IMAGE_GAMMA_CORRECTION_SAMPLES_PER_THREAD
	));

	*output = output_;

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImage::thresholdOtsu(const CompVMatPtr& input, double& threshold, CompVMatPtrPtr output COMPV_DEFAULT(nullptr))
{
	COMPV_CHECK_CODE_RETURN(CompVImageThreshold::otsu(input, threshold, output));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImage::thresholdOtsu(const CompVMatPtr& input, CompVMatPtrPtr output)
{
	double threshold = 0.0;
	COMPV_CHECK_CODE_RETURN(CompVImageThreshold::otsu(input, threshold, output));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImage::thresholdGlobal(const CompVMatPtr& input, CompVMatPtrPtr output, const double& threshold)
{
	COMPV_CHECK_CODE_RETURN(CompVImageThreshold::global(input, output, threshold));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImage::thresholdAdaptive(const CompVMatPtr& input, CompVMatPtrPtr output, const size_t& blockSize, const double& delta, const double& maxVal COMPV_DEFAULT(255.0), bool invert COMPV_DEFAULT(false))
{
	COMPV_CHECK_CODE_RETURN(CompVImageThreshold::adaptive(input, output, blockSize, delta, maxVal, invert));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImage::thresholdAdaptive(const CompVMatPtr& input, CompVMatPtrPtr output, const CompVMatPtr& kernel, const double& delta, const double& maxVal COMPV_DEFAULT(255.0), bool invert COMPV_DEFAULT(false))
{
	COMPV_CHECK_CODE_RETURN(CompVImageThreshold::adaptive(input, output, kernel, delta, maxVal, invert));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImage::integral(const CompVMatPtr& imageIn, CompVMatPtrPtr imageSum, CompVMatPtrPtr imageSumsq)
{
	COMPV_CHECK_CODE_RETURN(CompVImageIntegral::process(imageIn, imageSum, imageSumsq));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImage::scale(const CompVMatPtr& imageIn, CompVMatPtrPtr imageOut, const size_t widthOut, const size_t heightOut, const COMPV_INTERPOLATION_TYPE scaleType COMPV_DEFAULT(COMPV_INTERPOLATION_TYPE_BILINEAR))
{
	COMPV_CHECK_EXP_RETURN(!imageIn || !imageOut || imageIn->isEmpty() || !widthOut || !heightOut, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	bool bSelfTransfer = imageIn == (*imageOut);
	bool bScaleFactor1 = widthOut == imageIn->cols() && heightOut == imageIn->rows();

	if (bSelfTransfer && bScaleFactor1) {
		// No scaling and output is equal to input
		return COMPV_ERROR_CODE_S_OK;
	}
	if (bScaleFactor1 & !CompVBase::isTestingMode()) { // In testing mode we may want to encode the same image several times to check CPU, Memory, Latency...
		if (bSelfTransfer) {
			// *outImage = This is enought
			return COMPV_ERROR_CODE_S_OK;
		}
		if (scaleType == COMPV_INTERPOLATION_TYPE_BICUBIC_FLOAT32 || scaleType == COMPV_INTERPOLATION_TYPE_BILINEAR_FLOAT32) {
			COMPV_CHECK_CODE_RETURN((CompVMathCast::process_static<uint8_t, compv_float32_t>(imageIn, imageOut)));
		}
		else {
			COMPV_CHECK_CODE_RETURN(imageIn->clone(imageOut));
		}
		return COMPV_ERROR_CODE_S_OK;
	}

	CompVMatPtr imageOut_ = (imageIn == *imageOut) ? nullptr : *imageOut; // When (imageIn == imageOut) we have to save imageIn
	const COMPV_SUBTYPE subType = ((imageIn->planeCount() == 1 && imageIn->subType() == COMPV_SUBTYPE_RAW_UINT8) ? COMPV_SUBTYPE_PIXELS_Y : imageIn->subType());
	if (scaleType == COMPV_INTERPOLATION_TYPE_BICUBIC_FLOAT32) {
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float32_t>(&imageOut_, heightOut, widthOut));
	}
	else {
		COMPV_CHECK_CODE_RETURN(CompVImage::newObj8u(&imageOut_, subType, widthOut, heightOut));
	}

	switch (scaleType) {
	case COMPV_INTERPOLATION_TYPE_BICUBIC:
	case COMPV_INTERPOLATION_TYPE_BICUBIC_FLOAT32:
		COMPV_CHECK_CODE_RETURN(CompVImageScaleBicubic::process(imageIn, imageOut_, scaleType));
		break;
	case COMPV_INTERPOLATION_TYPE_BILINEAR:
		COMPV_CHECK_CODE_RETURN(CompVImageScaleBilinear::process(imageIn, imageOut_));
		break;
	case COMPV_INTERPOLATION_TYPE_BILINEAR_FLOAT32: // TODO(dmi): This is a hack, for now the scaler cannot output float type
		COMPV_CHECK_CODE_RETURN(CompVImageScaleBilinear::process(imageIn, imageOut_));
		COMPV_CHECK_CODE_RETURN((CompVMathCast::process_static<uint8_t, compv_float32_t>(imageOut_, &imageOut_)));
		break;
	default:
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "%d not supported as scaling type", scaleType);
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED, "Invalid scaling type");
		break;
	}
	*imageOut = imageOut_;
	return COMPV_ERROR_CODE_S_OK;
}

// Used by many deep-learning function to scale an image to a fixed size and convert it to rgb24
COMPV_ERROR_CODE CompVImage::scaleYuvToRGB24(const CompVMatPtr& imageIn, CompVMatPtrPtr imageOut, const size_t widthOut, const size_t heightOut, const COMPV_INTERPOLATION_TYPE scaleType COMPV_DEFAULT(COMPV_INTERPOLATION_TYPE_BILINEAR))
{
	COMPV_CHECK_EXP_RETURN(!imageIn || !imageOut || !widthOut || !heightOut, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	
	const COMPV_SUBTYPE subType = imageIn->subType();	

	// Make sure it's YUV
	if (subType != COMPV_SUBTYPE_PIXELS_NV12
		&& subType != COMPV_SUBTYPE_PIXELS_NV21 // Android Camera
		&& subType != COMPV_SUBTYPE_PIXELS_YUV420P
		&& subType != COMPV_SUBTYPE_PIXELS_YVU420P
		&& subType != COMPV_SUBTYPE_PIXELS_YUV422P
		&& subType != COMPV_SUBTYPE_PIXELS_YUYV422 // DirectShow/MediaFoundation
		&& subType != COMPV_SUBTYPE_PIXELS_UYVY422
		&& subType != COMPV_SUBTYPE_PIXELS_YUV444P)
	{
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Input type not YUV (%s)", CompVGetSubtypeString(subType));
		return COMPV_ERROR_CODE_E_INVALID_IMAGE_FORMAT;
	}

	// Convert only if size matches
	if (imageIn->cols() == widthOut && imageIn->rows() == heightOut) {
		COMPV_CHECK_CODE_RETURN(CompVImage::convert(imageIn, COMPV_SUBTYPE_PIXELS_RGB24, imageOut));
		return COMPV_ERROR_CODE_S_OK;
	}
	
	CompVMatPtrVector planes;
	COMPV_CHECK_CODE_RETURN(CompVImage::unpack(imageIn, planes));
	const double sx = double(widthOut) / double(imageIn->cols());
	const double sy = double(heightOut) / double(imageIn->rows());
	auto funcPtr = [&](const size_t start, const size_t end) -> COMPV_ERROR_CODE {
		for (size_t i = start; i < end; ++i) {
			const size_t plane_widthOut = COMPV_MATH_ROUNDFU_2_NEAREST_INT(planes[i]->cols() * sx, size_t);
			const size_t plane_heightOut = COMPV_MATH_ROUNDFU_2_NEAREST_INT(planes[i]->rows() * sy, size_t);
			COMPV_CHECK_CODE_RETURN(CompVImage::scale(planes[i], &planes[i], plane_widthOut, plane_heightOut, scaleType));
		}
		return COMPV_ERROR_CODE_S_OK;
	};
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtr,
		1,
		planes.size(),
		1
	));

	// Convert to planar (faster to convert to RGB24) if packed
	const bool packedUV = (subType == COMPV_SUBTYPE_PIXELS_NV12 || subType == COMPV_SUBTYPE_PIXELS_NV21); // Android Camera == NV21
	CompVMatPtr yuvPlanar;
	if (packedUV) {
		COMPV_CHECK_CODE_RETURN(CompVImage::newObj8u(&yuvPlanar, COMPV_SUBTYPE_PIXELS_YUV420P, widthOut, heightOut));
		if (subType == COMPV_SUBTYPE_PIXELS_NV21) {
			std::iter_swap(planes.begin() + 1, planes.begin() + 2); // Swap U/V
		}
		auto funcPtr = [&](const size_t start, const size_t end) -> COMPV_ERROR_CODE {
			COMPV_ASSERT(start >= 0 && end <= 3);
			for (int planeId = static_cast<int>(start); planeId < static_cast<int>(end); ++planeId) {
				COMPV_ASSERT(planes[planeId]->cols() == yuvPlanar->cols(planeId) && planes[planeId]->rows() == yuvPlanar->rows(planeId)); // Output size should be even
				COMPV_CHECK_CODE_RETURN(CompVImageUtils::copy( // will copy minimum size
					COMPV_SUBTYPE_PIXELS_Y,
					planes[planeId]->ptr<const void>(), planes[planeId]->cols(), planes[planeId]->rows(), planes[planeId]->stride(),
					yuvPlanar->ptr<void>(0, 0, planeId), yuvPlanar->cols(planeId), yuvPlanar->rows(planeId), yuvPlanar->stride(planeId)
				));
			}
			return COMPV_ERROR_CODE_S_OK;
		};
		COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
			funcPtr,
			1,
			planes.size(),
			1
		));
	}
	else {
		COMPV_CHECK_CODE_RETURN(CompVImage::pack(planes, subType, &yuvPlanar));
	}
	
	// Convert from planar YUV to RGB24
	COMPV_CHECK_CODE_RETURN(CompVImage::convert(yuvPlanar, COMPV_SUBTYPE_PIXELS_RGB24, imageOut));
	COMPV_ASSERT((*imageOut)->cols() == widthOut && (*imageOut)->rows() == heightOut);
		
	return COMPV_ERROR_CODE_S_OK;
}

// dst(x,y) = src*M
// M = (2x3) or (3x3) matrix
template<typename T>
static COMPV_ERROR_CODE CompVImageWarpInverse(const CompVMatPtr& imageIn, CompVMatPtrPtr imageOut, const CompVMatPtr& M, const CompVSizeSz& outSize, COMPV_INTERPOLATION_TYPE interpType COMPV_DEFAULT(COMPV_INTERPOLATION_TYPE_BILINEAR), const uint8_t defaultPixelValue COMPV_DEFAULT(0x00))
{
	COMPV_CHECK_EXP_RETURN(!imageIn || !imageOut || !M || !outSize.height || !outSize.width, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_EXP_RETURN((M->rows() != 2 && M->rows() != 3) || M->cols() != 3 || (M->subType() != COMPV_SUBTYPE_RAW_FLOAT32 && M->subType() != COMPV_SUBTYPE_RAW_FLOAT64), COMPV_ERROR_CODE_E_INVALID_PARAMETER, "M must be (2x3) float or double matrix");
	COMPV_CHECK_EXP_RETURN(!M->isRawTypeMatch<T>() || (M->subType() != COMPV_SUBTYPE_RAW_FLOAT32 && M->subType() != COMPV_SUBTYPE_RAW_FLOAT64), COMPV_ERROR_CODE_E_INVALID_SUBTYPE);
	
	const size_t count = (outSize.width * outSize.height);
	const size_t& width = outSize.width;
	const size_t width4 = width & -4;
	const size_t& height = outSize.height;

	CompVMatPtr map;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<T>(&map, M->rows(), count));

	if (M->rows() == 2) {
		const T* M0 = M->ptr<const T>(0);
		const T* M1 = M->ptr<const T>(1);
		const T& a = M0[0];
		const T& b = M0[1];
		const T& c = M0[2];
		const T& d = M1[0];
		const T& e = M1[1];
		const T& f = M1[2];

		CompVMatPtr ac; // (a*0+c*1), (a*1+c*1), (a*2+c*1)....(a*width+c*1)
		CompVMatPtr df; // (d*0+f*1), (d*1+f*1), (d*2+f*1)....(d*width+f*1)
		CompVMatPtr by; // (b*0), (b*1), (b*2)....(b*height)
		CompVMatPtr ey; // (e*0), (e*1), (e*2)....(e*height)
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjStrideless<T>(&ac, 1, width));
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjStrideless<T>(&df, 1, width));
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjStrideless<T>(&by, 1, height));
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjStrideless<T>(&ey, 1, height));
		
		// Build indices for a single row
		T* acPtr = ac->ptr<T>();
		T* dfPtr = df->ptr<T>();
		acPtr[0] = c; // (a*0+c*1)
		dfPtr[0] = f; // (d*0+f*1)
		for (size_t x = 1; x < width; ++x) {
			acPtr[x] = acPtr[x - 1] + a;
			dfPtr[x] = dfPtr[x - 1] + d;
		}

		T* byPtr = by->ptr<T>();
		T* eyPtr = ey->ptr<T>();
		byPtr[0] = 0; //(b*0)
		eyPtr[0] = 0; //(e*0)
		for (size_t y = 1; y < height; ++y) {
			byPtr[y] = byPtr[y - 1] + b;
			eyPtr[y] = eyPtr[y - 1] + e;
		}

		// Build entire map using the single row indices built for #0 (just increment by "b" or "e" for each row)
		auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
			T* mapPtrX = map->ptr<T>(0, (ystart * width));
			T* mapPtrY = map->ptr<T>(1, (ystart * width));
			const T* byPtr = by->data<T>(); // b * y
			const T* eyPtr = ey->data<T>(); // e * y
			size_t x;
			for (size_t y = ystart, k = 0; y < yend; ++y) {
				const T& by_ = byPtr[y]; // b*y
				const T& ey_ = eyPtr[y]; // e*y
				// mapPtrX[k] = (a*x+c) + b*y
				// mapPtrY[k] = (d*x+f) + e*y
				for (x = 0; x < width4; x += 4, k += 4) {
					mapPtrX[k] = acPtr[x] + by_;
					mapPtrX[k + 1] = acPtr[x + 1] + by_;
					mapPtrX[k + 2] = acPtr[x + 2] + by_;
					mapPtrX[k + 3] = acPtr[x + 3] + by_;

					mapPtrY[k] = dfPtr[x] + ey_;
					mapPtrY[k + 1] = dfPtr[x + 1] + ey_;
					mapPtrY[k + 2] = dfPtr[x + 2] + ey_;
					mapPtrY[k + 3] = dfPtr[x + 3] + ey_;
				}
				for (; x < width; ++x, ++k) {
					mapPtrX[k] = acPtr[x] + by_;
					mapPtrY[k] = dfPtr[x] + ey_;
				}
			}
			return COMPV_ERROR_CODE_S_OK;
		};
		COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
			funcPtr,
			width,
			height,
			(100 * 100)
		));
	}
	else {
		const T* M0 = M->ptr<const T>(0);
		const T* M1 = M->ptr<const T>(1);
		const T* M2 = M->ptr<const T>(2);
		const T& a = M0[0];
		const T& b = M0[1];
		const T& c = M0[2];
		const T& d = M1[0];
		const T& e = M1[1];
		const T& f = M1[2];
		const T& g = M2[0];
		const T& h = M2[1];
		const T& i = M2[2];
		
		CompVMatPtr ac; // (a*0+c*1), (a*1+c*1), (a*2+c*1)....(a*width+c*1)
		CompVMatPtr df; // (d*0+f*1), (d*1+f*1), (d*2+f*1)....(d*width+f*1)
		CompVMatPtr gi; // (g*0+i*1), (g*1+i*1), (g*2+i*1)....(g*width+i*1)		
		CompVMatPtr by; // (b*0), (b*1), (b*2)....(b*height)
		CompVMatPtr ey; // (e*0), (e*1), (e*2)....(e*height)
		CompVMatPtr hy; // (h*0), (h*1), (h*2)....(h*height)
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjStrideless<T>(&ac, 1, width));
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjStrideless<T>(&df, 1, width));
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjStrideless<T>(&gi, 1, width));
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjStrideless<T>(&by, 1, height));
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjStrideless<T>(&ey, 1, height));
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjStrideless<T>(&hy, 1, height));

		// Build indices for a single row
		T* acPtr = ac->ptr<T>();
		T* dfPtr = df->ptr<T>();
		T* giPtr = gi->ptr<T>();
		acPtr[0] = c; // (a*0+c*1)
		dfPtr[0] = f; // (d*0+f*1)			
		giPtr[0] = i; // (g*0+i*1)
		for (size_t x = 1; x < width; ++x) {
			acPtr[x] = acPtr[x - 1] + a;
			dfPtr[x] = dfPtr[x - 1] + d;
			giPtr[x] = giPtr[x - 1] + g;
		}

		T* byPtr = by->ptr<T>();
		T* eyPtr = ey->ptr<T>();
		T* hyPtr = hy->ptr<T>();
		byPtr[0] = 0; //(b*0)
		eyPtr[0] = 0; //(e*0)
		hyPtr[0] = 0; //(h*0)
		for (size_t y = 1; y < height; ++y) {
			byPtr[y] = byPtr[y - 1] + b;
			eyPtr[y] = eyPtr[y - 1] + e;
			hyPtr[y] = hyPtr[y - 1] + h;
		}
		
		// Build entire map using the single row indices built for #0 (just increment by "b" or "e" for each row)
		auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
			T* mapPtrX = map->ptr<T>(0, (ystart * width));
			T* mapPtrY = map->ptr<T>(1, (ystart * width));
			T* mapPtrZ = map->ptr<T>(2, (ystart * width));
			const T* byPtr = by->data<T>(); // b * y
			const T* eyPtr = ey->data<T>(); // e * y
			const T* hyPtr = hy->data<T>(); // h * y
			size_t x;
			for (size_t y = ystart, k = 0; y < yend; ++y) {
				const T& by_ = byPtr[y]; // b*y
				const T& ey_ = eyPtr[y]; // e*y
				const T& hy_ = hyPtr[y]; // h*y
				// mapPtrX[k] = (a*x+c) + b*y
				// mapPtrY[k] = (d*x+f) + e*y
				// mapPtrZ[k] = (g*x+f) + h*y
				for (x = 0; x < width4; x += 4, k += 4) {
					mapPtrX[k] = acPtr[x] + by_;
					mapPtrX[k + 1] = acPtr[x + 1] + by_;
					mapPtrX[k + 2] = acPtr[x + 2] + by_;
					mapPtrX[k + 3] = acPtr[x + 3] + by_;

					mapPtrY[k] = dfPtr[x] + ey_;
					mapPtrY[k + 1] = dfPtr[x + 1] + ey_;
					mapPtrY[k + 2] = dfPtr[x + 2] + ey_;
					mapPtrY[k + 3] = dfPtr[x + 3] + ey_;

					mapPtrZ[k] = giPtr[x] + hy_;
					mapPtrZ[k + 1] = giPtr[x + 1] + hy_;
					mapPtrZ[k + 2] = giPtr[x + 2] + hy_;
					mapPtrZ[k + 3] = giPtr[x + 3] + hy_;
				}
				for (; x < width; ++x, ++k) {
					mapPtrX[k] = acPtr[x] + by_;
					mapPtrY[k] = dfPtr[x] + ey_;
					mapPtrZ[k] = giPtr[x] + hy_;
				}
			}
			return COMPV_ERROR_CODE_S_OK;
		};
		COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
			funcPtr,
			width,
			height,
			(100 * 100)
		));

		// Homogeneous to cartesian (3*n) -> (2*n)
		COMPV_CHECK_CODE_RETURN(CompVMathTransform::homogeneousToCartesian2D(map, &map));
	}

	// remap
	const CompVRectFloat32 inputROI = { 0.f, 0.f, static_cast<compv_float32_t>(imageIn->cols() - 1), static_cast<compv_float32_t>(imageIn->rows() - 1) };
	COMPV_CHECK_CODE_RETURN(CompVImageRemap::process(imageIn, imageOut, map, interpType, &inputROI, &outSize, defaultPixelValue));

	return COMPV_ERROR_CODE_S_OK;
}

// dst(x,y) = src*inverse(M)
// M = (2x3) or (3 x 3) matrix
template<typename T>
static COMPV_ERROR_CODE CompVImageWarp(const CompVMatPtr& imageIn, CompVMatPtrPtr imageOut, const CompVMatPtr& M, const CompVSizeSz& outSize, COMPV_INTERPOLATION_TYPE interpType COMPV_DEFAULT(COMPV_INTERPOLATION_TYPE_BILINEAR), const uint8_t defaultPixelValue COMPV_DEFAULT(0x00))
{
	COMPV_CHECK_EXP_RETURN(!imageIn || !imageOut || !M || !outSize.height || !outSize.width, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_EXP_RETURN((M->rows() != 2 && M->rows() != 3) || M->cols() != 3, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "M must be (2x3) float or double matrix");
	COMPV_CHECK_EXP_RETURN(!M->isRawTypeMatch<T>() || (M->subType() != COMPV_SUBTYPE_RAW_FLOAT32 && M->subType() != COMPV_SUBTYPE_RAW_FLOAT64), COMPV_ERROR_CODE_E_INVALID_SUBTYPE);
	// No need to check other parameters -> up2 CompVImage::warpInverse

	// inverse(M)
	CompVMatPtr Minverse;
	if (M->rows() == 2) {
		COMPV_CHECK_CODE_RETURN(M->clone(&Minverse));
		T* M0ptr = Minverse->ptr<T>(0);
		T* M1ptr = Minverse->ptr<T>(1);
		T D = M0ptr[0] * M1ptr[1] - M0ptr[1] * M1ptr[0];
		D = D != 0 ? static_cast<T>(1.0 / D) : 0;
		const T A11 = M1ptr[1] * D, A22 = M0ptr[0] * D;
		M0ptr[0] = A11; M0ptr[1] *= -D;
		M1ptr[0] *= -D; M1ptr[1] = A22;
		const T b1 = -M0ptr[0] * M0ptr[2] - M0ptr[1] * M1ptr[2];
		const T b2 = -M1ptr[0] * M0ptr[2] - M1ptr[1] * M1ptr[2];
		M0ptr[2] = b1; M1ptr[2] = b2;
	}
	else {
		COMPV_CHECK_CODE_RETURN(CompVMatrix::invA3x3(M, &Minverse));
	}

	// Perform action
	COMPV_CHECK_CODE_RETURN(CompVImageWarpInverse<T>(imageIn, imageOut, Minverse, outSize, interpType, defaultPixelValue));

	return COMPV_ERROR_CODE_S_OK;
}

// dst(x,y) = src*inverse(M)
// M = (2x3) matrix
COMPV_ERROR_CODE CompVImage::warp(const CompVMatPtr& imageIn, CompVMatPtrPtr imageOut, const CompVMatPtr& M, const CompVSizeSz& outSize, COMPV_INTERPOLATION_TYPE interpType COMPV_DEFAULT(COMPV_INTERPOLATION_TYPE_BILINEAR), const uint8_t defaultPixelValue COMPV_DEFAULT(0x00))
{
	CompVGenericFloatInvokeCodeRawType(M->subType(), CompVImageWarp, imageIn, imageOut, M, outSize, interpType, defaultPixelValue);
	return COMPV_ERROR_CODE_S_OK;
}

// dst(x,y) = src*M
// M = (2x3) matrix
COMPV_ERROR_CODE CompVImage::warpInverse(const CompVMatPtr& imageIn, CompVMatPtrPtr imageOut, const CompVMatPtr& M, const CompVSizeSz& outSize, COMPV_INTERPOLATION_TYPE interpType COMPV_DEFAULT(COMPV_INTERPOLATION_TYPE_BILINEAR), const uint8_t defaultPixelValue COMPV_DEFAULT(0x00))
{
	CompVGenericFloatInvokeCodeRawType(M->subType(), CompVImageWarpInverse, imageIn, imageOut, M, outSize, interpType, defaultPixelValue);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
