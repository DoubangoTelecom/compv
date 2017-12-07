/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/video/compv_video_reader.h"

COMPV_NAMESPACE_BEGIN()

//
//	CompVVideoReader
//

CompVVideoReader::CompVVideoReader()
{

}

CompVVideoReader::~CompVVideoReader()
{
	
}

COMPV_ERROR_CODE CompVVideoReader::newObj(CompVVideoReaderPtrPtr reader)
{
	COMPV_CHECK_EXP_RETURN(!reader, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_CODE_RETURN(CompVVideoReaderFactory::newObj(reader));
	return COMPV_ERROR_CODE_S_OK;
}

//
//	CompVVideoReaderFactory
//
const CompVVideoReaderFactory* CompVVideoReaderFactory::instance = nullptr;

COMPV_NAMESPACE_END()
