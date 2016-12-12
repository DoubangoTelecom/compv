/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/compv_image.h"
#include "compv/base/image/compv_image_utils.h"
#include "compv/base/math/compv_math_utils.h"
#include "compv/base/compv_mem.h"
#include "compv/base/compv_fileutils.h"


#define COMPV_IMAGE_NEWOBJ_CASE(elmType,pixelFormat) \
		case COMPV_SUBTYPE_PIXELS_##pixelFormat: \
			COMPV_CHECK_CODE_RETURN((CompVMat::newObjAligned<elmType, COMPV_MAT_TYPE_PIXELS, COMPV_SUBTYPE_PIXELS_##pixelFormat>(mat, height, width))); \
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
		COMPV_IMAGE_NEWOBJ_CASE(elmType, Y); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, NV12); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, NV21); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, YUV420P); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, YVU420P); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, YUV422P); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, YUYV422); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, YVYU422); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, UYVY422); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, YUV444P); \
	default: \
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED); \
		break; \
	}

COMPV_NAMESPACE_BEGIN()

COMPV_ERROR_CODE CompVImage::newObj8u(CompVMatPtrPtr mat, size_t width, size_t height, COMPV_SUBTYPE subType)
{
    COMPV_IMAGE_NEWOBJ_SWITCH(uint8_t, subType);
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImage::newObj16u(CompVMatPtrPtr mat, size_t width, size_t height, COMPV_SUBTYPE subType)
{
    COMPV_IMAGE_NEWOBJ_SWITCH(uint16_t, subType);
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImage::readPixels(COMPV_SUBTYPE ePixelFormat, size_t width, size_t height, size_t stride, const char* filePath, CompVMatPtrPtr mat)
{
	COMPV_CHECK_EXP_RETURN(!filePath || !width || !height || stride < width || !mat, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVBufferPtr buffer;
	COMPV_CHECK_CODE_RETURN(CompVFileUtils::read(filePath, &buffer));
	size_t expectedFileSize;
	COMPV_CHECK_CODE_RETURN(CompVImageUtils::sizeForPixelFormat(ePixelFormat, stride, height, &expectedFileSize));
	COMPV_CHECK_EXP_RETURN(expectedFileSize != buffer->size(), COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT);
	COMPV_CHECK_CODE_RETURN(CompVImage::wrap(ePixelFormat, buffer->ptr(), width, height, stride, mat));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImage::wrap(COMPV_SUBTYPE ePixelFormat, const void* dataPtr, size_t width, size_t height, size_t stride, CompVMatPtrPtr mat)
{
	COMPV_CHECK_EXP_RETURN(!dataPtr || !width || !height || stride < width || !mat, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;

	// Compute best stride
	size_t bestStride = static_cast<size_t>(CompVMem::alignForward(width, COMPV_SIMD_ALIGNV_DEFAULT)); 

#if 1
	bool bAllocNewImage = !(*mat) ||
		(*mat)->type() != COMPV_MAT_TYPE_PIXELS ||
		(*mat)->subType() != ePixelFormat ||
		(*mat)->cols() != width ||
		(*mat)->rows() != height ||
		(*mat)->stride() != bestStride;
#else // to test copy and alloc
	COMPV_DEBUG_INFO_CODE_FOR_TESTING();
	bool bAllocNewImage = true;
#endif

	CompVMatPtr mat_;

	if (bAllocNewImage) {
		COMPV_CHECK_CODE_BAIL(err_ = CompVImage::newObj8u(&mat_, width, height, ePixelFormat));
		COMPV_CHECK_EXP_BAIL(bestStride != mat_->stride(), err_ = COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT);
	}
	else {
		mat_ = *mat;
	}

	if (dataPtr) {
		COMPV_CHECK_CODE_BAIL(err_ = CompVImageUtils::copy(ePixelFormat,
			dataPtr, width, height, stride,
			(void*)mat_->ptr(), mat_->cols(), mat_->rows(), mat_->stride())); // copy data
	}
bail:
	if (COMPV_ERROR_CODE_IS_OK(err_)) {
		*mat = mat_;
	}
	return err_;
}


COMPV_NAMESPACE_END()