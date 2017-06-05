/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/video/compv_core_video_reader_ffmpeg.h"

COMPV_NAMESPACE_BEGIN()

//
//	CompVVideoReaderFactoryFFmpeg
//

static COMPV_ERROR_CODE CompVVideoReaderFactoryFFmpeg_newObj(CompVVideoReaderPtrPtr reader)
{
	COMPV_CHECK_EXP_RETURN(!reader, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVVideoReaderFFmpegPtr readerFFmpeg_;
	COMPV_CHECK_CODE_RETURN(CompVVideoReaderFFmpeg::newObj(&readerFFmpeg_));
	*reader = *readerFFmpeg_;
	return COMPV_ERROR_CODE_S_OK;
}

const CompVVideoReaderFactory CompVVideoReaderFactoryFFmpeg = {
	"FFmpeg Video Reader factory",
	CompVVideoReaderFactoryFFmpeg_newObj
};

//
//	CompVVideoReaderFFmpeg
//

CompVVideoReaderFFmpeg::CompVVideoReaderFFmpeg()
	: CompVVideoReader()
	, m_bOpened(false)
{

}

CompVVideoReaderFFmpeg::~CompVVideoReaderFFmpeg()
{
	COMPV_CHECK_CODE_NOP(close());
}


COMPV_ERROR_CODE CompVVideoReaderFFmpeg::open(const std::string& path)
{
	if (!m_bOpened) {

	}
	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}

bool CompVVideoReaderFFmpeg::isOpen()const
{
	return m_bOpened;
}

COMPV_ERROR_CODE CompVVideoReaderFFmpeg::close()
{
	if (m_bOpened) {

	}
	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVVideoReaderFFmpeg::read(CompVMatPtrPtr frame)
{
	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVVideoReaderFFmpeg::newObj(CompVVideoReaderFFmpegPtrPtr reader)
{
	COMPV_CHECK_EXP_RETURN(!reader, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVVideoReaderFFmpegPtr reader_ = new CompVVideoReaderFFmpeg();
	COMPV_CHECK_EXP_RETURN(!reader_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

	*reader = reader_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
