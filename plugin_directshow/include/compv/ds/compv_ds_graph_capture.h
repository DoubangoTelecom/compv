/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_PLUGIN_DIRECTSHOW_GRAPH_CAPTURE_H_)
#define _COMPV_PLUGIN_DIRECTSHOW_GRAPH_CAPTURE_H_

#include "compv/ds/compv_ds_config.h"
#include "compv/base/compv_obj.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

COMPV_OBJECT_DECLARE_PTRS(DSGraphCapture)

class CompVDSGraphCapture : public CompVObj
{
protected:
    CompVDSGraphCapture(const ISampleGrabberCB* pcSampleGrabberCB);
public:
    virtual ~CompVDSGraphCapture();
    COMPV_OBJECT_GET_ID(CompVDSGraphCapture);
    COMPV_INLINE bool isConnected()const {
        return m_bConnect;
    }
    COMPV_INLINE bool isStarted()const {
        return m_bStart;
    }
    COMPV_INLINE const AM_MEDIA_TYPE& connectedMediaType()const {
        return m_ConnectedMediaType;
    }

    COMPV_ERROR_CODE start(const std::string& deviceId = "");
    COMPV_ERROR_CODE stop();

    static COMPV_ERROR_CODE newObj(CompVDSGraphCapturePtrPtr graph, const ISampleGrabberCB* pcSampleGrabberCB);

private:
    COMPV_ERROR_CODE init(const std::string& deviceId = "");
    COMPV_ERROR_CODE deInit();
    COMPV_ERROR_CODE connect(const std::string& deviceId = "");
    COMPV_ERROR_CODE disconnect();

private:
    bool m_bInit;
    bool m_bConnect;
    bool m_bStart;
    ICaptureGraphBuilder2* m_pCaptureGraphBuilder;
    IGraphBuilder* m_pGraphBuilder;

    IMediaControl* m_pMediaController;
    IMediaEventEx* m_pMediaEvent;

    ISampleGrabber* m_pGrabber;

    IBaseFilter* m_pFilterSampleGrabber;
    IBaseFilter* m_pFilterNullRenderer;
    IBaseFilter* m_pFilterSource;

    const ISampleGrabberCB* m_pcSampleGrabberCB;

    AM_MEDIA_TYPE m_ConnectedMediaType;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_PLUGIN_DIRECTSHOW_GRAPH_CAPTURE_H_ */
