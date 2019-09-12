/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/compv_exif.h"
#include "compv/base/compv_debug.h"
#include "compv/base/easyexif/easyexif.h"

#define COMPV_THIS_CLASSNAME "CompVExif"

COMPV_NAMESPACE_BEGIN()

COMPV_ERROR_CODE CompVExif::parse(const void* jpegBufferPtr, const size_t jpegBufferSizeInBytes, std::map<std::string, std::string>& fieldsValues, const CompVExifField& fieldsToExtract COMPV_DEFAULT(CompVExifField_All))
{
	COMPV_CHECK_EXP_RETURN(!jpegBufferPtr || !jpegBufferSizeInBytes, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	// Clear
	fieldsValues.clear();

	// Nop if none
	if (fieldsToExtract == CompVExifField_None) {
		return COMPV_ERROR_CODE_S_OK;
	}

	// Parse info
	easyexif::EXIFInfo result;
#if !COMPV_OS_ANDROID // Exception handling on Android
	try {
#endif
		int err = result.parseFrom(reinterpret_cast<const unsigned char*>(jpegBufferPtr), static_cast<unsigned>(jpegBufferSizeInBytes));
		if (err == PARSE_EXIF_ERROR_NO_EXIF) {
			return COMPV_ERROR_CODE_S_OK;
		}
		else if (err != PARSE_EXIF_SUCCESS) {
			COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "[easyexif] Parse returned %d", err);
			COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_THIRD_PARTY_LIB);
		}
#if !COMPV_OS_ANDROID
	}
	catch (std::exception& e) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "[easyexif] Parse error: %s", e.what());
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_THIRD_PARTY_LIB);
	}
#endif

	if (fieldsToExtract & CompVExifField_CameraMake) {
		fieldsValues["CameraMake"] = result.Make;
	}
	if (fieldsToExtract & CompVExifField_CameraModel) {
		fieldsValues["CameraModel"] = result.Model;
	}
	if (fieldsToExtract & CompVExifField_Software) {
		fieldsValues["Software"] = result.Software;
	}
	if (fieldsToExtract & CompVExifField_BitsPerSample) {
		fieldsValues["BitsPerSample"] = CompVBase::to_string(result.BitsPerSample);
	}
	if (fieldsToExtract & CompVExifField_Width) {
		fieldsValues["Width"] = CompVBase::to_string(result.ImageWidth);
	}
	if (fieldsToExtract & CompVExifField_Height) {
		fieldsValues["Height"] = CompVBase::to_string(result.ImageHeight);
	}
	if (fieldsToExtract & CompVExifField_Orientation) { // [1, 8]: https://www.impulseadventure.com/photo/exif-orientation.html
		fieldsValues["Orientation"] = CompVBase::to_string(result.Orientation);
	}

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
