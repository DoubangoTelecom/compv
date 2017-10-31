/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_CORE_VIDEO_READER_FFMPEG_H_)
#define _COMPV_CORE_VIDEO_READER_FFMPEG_H_

#include "compv/core/compv_core_config.h"
#if defined(HAVE_FFMPEG)

#include "compv/core/compv_core_common.h"
#include "compv/base/video/compv_video_reader.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

enum AVPixelFormat;
struct AVFormatContext;
struct AVStream;
struct AVCodecContext;
struct AVFrame;

COMPV_NAMESPACE_BEGIN()

COMPV_OBJECT_DECLARE_PTRS(VideoReaderFFmpeg)

extern const CompVVideoReaderFactory CompVVideoReaderFactoryFFmpeg;

class CompVVideoReaderFFmpeg : public CompVVideoReader
{
protected:
	CompVVideoReaderFFmpeg();
public:
	virtual ~CompVVideoReaderFFmpeg();
	COMPV_OBJECT_GET_ID(CompVVideoReaderFFmpeg);

	virtual COMPV_ERROR_CODE open(const char* path) override;
	virtual bool isOpen()const override;
	virtual COMPV_ERROR_CODE close() override;
	virtual COMPV_ERROR_CODE read(CompVMatPtrPtr frame) override;
	virtual int frameRate()const override;

	static COMPV_ERROR_CODE newObj(CompVVideoReaderFFmpegPtrPtr reader);

private:
	bool m_bOpened;
	struct AVFormatContext *m_pFmtCtx;
	struct AVStream *m_pStream;
	struct AVCodecContext *m_pDecCtx;
	struct AVFrame *m_pFrame;
	int m_nStreamIdx;
	COMPV_SUBTYPE m_eSubType;
	AVPixelFormat m_ePixFmt;
	int m_nFrameRate;
};

COMPV_NAMESPACE_END()

#endif /* HAVE_FFMPEG */

#endif /* _COMPV_CORE_VIDEO_READER_FFMPEG_H_ */
