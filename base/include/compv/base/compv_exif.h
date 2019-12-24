/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_EXIF_H_)
#define _COMPV_BASE_EXIF_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_obj.h"
#include "compv/base/compv_common.h"

#include <map>

COMPV_NAMESPACE_BEGIN()

enum CompVExifField {
	CompVExifField_None = 0,
	CompVExifField_CameraMake = (1 << 0),
	CompVExifField_CameraModel = (1 << 1),
	CompVExifField_Software = (1 << 2),
	CompVExifField_BitsPerSample = (1 << 3),
	CompVExifField_Width = (1 << 4),
	CompVExifField_Height = (1 << 5),
	CompVExifField_Orientation = (1 << 6),

	CompVExifField_All = 0xffffffff
};

class COMPV_BASE_API CompVExif
{
public:
	static COMPV_ERROR_CODE parse(const void* jpegBufferPtr, const size_t jpegBufferSizeInBytes, std::map<std::string, std::string>& fieldsValues, const CompVExifField& fieldsToExtract = CompVExifField_All);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_EXIF_H_ */
