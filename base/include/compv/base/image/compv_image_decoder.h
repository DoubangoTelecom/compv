/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_IMAGE_DECODER_H_)
#define _COMPV_BASE_IMAGE_DECODER_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_mat.h"

COMPV_NAMESPACE_BEGIN()

typedef COMPV_ERROR_CODE(*CompVDecodeFileFuncPtr)(const char* filePath, CompVMatPtrPtr mat);
typedef COMPV_ERROR_CODE(*CompVDecodeInfoFuncPtr)(const char* filePath, CompVImageInfo& info);

class COMPV_BASE_API CompVImageDecoder
{
public:
	static COMPV_ERROR_CODE init();
	static COMPV_ERROR_CODE deInit();
	static COMPV_ERROR_CODE setFuncPtrs(COMPV_IMAGE_FORMAT format, CompVDecodeFileFuncPtr funcptrDecodeFile, CompVDecodeInfoFuncPtr funcptrDecodeInfo);
	static COMPV_ERROR_CODE decodeFile(const char* filePath, CompVMatPtrPtr mat);
	static COMPV_ERROR_CODE decodeInfo(const char* filePath, CompVImageInfo& info);

private:
	static bool s_bInitialize;
	static CompVDecodeFileFuncPtr s_funcptrDecodeFileJpeg;
	static CompVDecodeInfoFuncPtr s_funcptrDecodeInfoJpeg;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_IMAGE_DECODER_H_ */
