/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/compv_image.h"
#include "compv/base/math/compv_math_utils.h"
#include "compv/base/compv_mem.h"


#define COMPV_IMAGE_NEWOBJ_CASE(elmType,pixelFormat) \
		case COMPV_PIXEL_FORMAT_##pixelFormat: \
			COMPV_CHECK_CODE_RETURN((CompVMat::newObjAligned<elmType, COMPV_MAT_TYPE_PIXELS, COMPV_MAT_SUBTYPE_PIXELS_##pixelFormat>(mat, rows, cols))); \
			return COMPV_ERROR_CODE_S_OK;
#define COMPV_IMAGE_NEWOBJ_SWITCH(elmType, subType) \
	switch (subType) { \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, R8G8B8); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, B8G8R8); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, R8G8B8A8); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, B8G8R8A8); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, A8B8G8R8); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, A8R8G8B8); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, GRAYSCALE); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, I420); \
	default: \
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED); \
		break; \
	}

COMPV_NAMESPACE_BEGIN()

COMPV_ERROR_CODE CompVImage::newObj8u(CompVMatPtrPtr mat, size_t rows, size_t cols, COMPV_PIXEL_FORMAT subType)
{
    COMPV_IMAGE_NEWOBJ_SWITCH(uint8_t, subType);
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImage::newObj16u(CompVMatPtrPtr mat, size_t rows, size_t cols, COMPV_PIXEL_FORMAT subType)
{
    COMPV_IMAGE_NEWOBJ_SWITCH(uint16_t, subType);
    return COMPV_ERROR_CODE_S_OK;
}


COMPV_NAMESPACE_END()