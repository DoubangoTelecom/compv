/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_DRAWING_IMAGE_LIBJPEG_H_)
#define _COMPV_DRAWING_IMAGE_LIBJPEG_H_

#include "compv/drawing/compv_drawing_config.h"
#if defined(HAVE_JPEGLIB_H)
#include "compv/drawing/compv_drawing_common.h"
#include "compv/base/compv_mat.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

COMPV_ERROR_CODE libjpegDecodeFile(const char* filePath, CompVMatPtrPtr mat);
COMPV_ERROR_CODE libjpegDecodeInfo(const char* filePath, CompVImageInfo& info);

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_JPEGLIB_H) */

#endif /* _COMPV_DRAWING_IMAGE_LIBJPEG_H_ */
