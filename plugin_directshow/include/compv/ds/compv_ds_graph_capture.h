/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
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
        return m_bConnected;
    }
    COMPV_INLINE bool isStarted()const {
        return m_bStarted;
    }
    COMPV_INLINE const AM_MEDIA_TYPE* connectedMediaType()const {
        return &m_ConnectedMediaType;
    }
	COMPV_INLINE const CompVDSCameraCaps* capsNeg()const {
		return &m_CapsNeg;
	}
	COMPV_INLINE const CompVDSCameraCaps* capsPref()const {
		return &m_CapsPref;
	}

    COMPV_ERROR_CODE start(const std::string& deviceId, const CompVDSCameraCaps& caps);
    COMPV_ERROR_CODE stop();

    static COMPV_ERROR_CODE newObj(CompVDSGraphCapturePtrPtr graph, const ISampleGrabberCB* pcSampleGrabberCB);

private:
    COMPV_ERROR_CODE init(const std::string& deviceId);
    COMPV_ERROR_CODE deInit();
    COMPV_ERROR_CODE connect(const std::string& deviceId);
    COMPV_ERROR_CODE disconnect();
	COMPV_ERROR_CODE addSource(const std::string& deviceId);
	COMPV_ERROR_CODE changeSource(const std::string& deviceId, bool connect);
	COMPV_ERROR_CODE queryCapNeg();
	COMPV_ERROR_CODE applyCaps();

private:
    bool m_bInit;
    bool m_bConnected;
    bool m_bStarted;
	std::string m_strDeviceId;
	CompVDSCameraCaps m_CapsPref;
	CompVDSCameraCaps m_CapsNeg;
    ICaptureGraphBuilder2* m_pCaptureGraphBuilder;
    IGraphBuilder* m_pGraphBuilder;

    IMediaControl* m_pMediaController;
    IMediaEventEx* m_pMediaEvent;

    ISampleGrabber* m_pGrabber;

    IBaseFilter* m_pFilterSampleGrabber;
    IBaseFilter* m_pFilterNullRenderer;
    IBaseFilter* m_pFilterSource;

	IAMStreamConfig* m_pStreamConfig;

    const ISampleGrabberCB* m_pcSampleGrabberCB;

    AM_MEDIA_TYPE m_ConnectedMediaType;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_PLUGIN_DIRECTSHOW_GRAPH_CAPTURE_H_ */
