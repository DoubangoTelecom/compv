/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/ds/compv_ds_graph_capture.h"
#include "compv/ds/compv_ds_utils.h"

#define COMPV_THIS_CLASSNAME "CompVDSGraphCapture"

COMPV_NAMESPACE_BEGIN()

CompVDSGraphCapture::CompVDSGraphCapture(const ISampleGrabberCB* pcSampleGrabberCB)
    : m_bInit(false)
    , m_bConnected(false)
    , m_bStarted(false)
    , m_pCaptureGraphBuilder(NULL)
    , m_pGraphBuilder(NULL)
    , m_pGrabber(NULL)
    , m_pMediaController(NULL)
    , m_pMediaEvent(NULL)
    , m_pFilterSampleGrabber(NULL)
    , m_pFilterNullRenderer(NULL)
    , m_pFilterSource(NULL)
	, m_pStreamConfig(NULL)
    , m_pcSampleGrabberCB(pcSampleGrabberCB)
{
    ZeroMemory(&m_ConnectedMediaType, sizeof(m_ConnectedMediaType));
}

CompVDSGraphCapture::~CompVDSGraphCapture()
{
    COMPV_CHECK_CODE_NOP(deInit());
}

COMPV_ERROR_CODE CompVDSGraphCapture::start(const std::string& deviceId, const CompVDSCameraCaps& caps)
{
	m_CapsPref = caps;
    if (m_bStarted) {
        return COMPV_ERROR_CODE_S_OK;
    }
    HRESULT hr = S_OK;
    COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
    COMPV_CHECK_CODE_BAIL(err = connect(deviceId));
	COMPV_CHECK_EXP_BAIL(!m_pMediaController, (err = COMPV_ERROR_CODE_E_INVALID_STATE));
	COMPV_CHECK_CODE_BAIL(err = applyCaps());
    COMPV_CHECK_HRESULT_CODE_BAIL(hr = m_pGrabber->SetCallback(const_cast<ISampleGrabberCB*>(m_pcSampleGrabberCB), 1)); // Inc(m_pcSampleGrabberCB.refCount)
    COMPV_CHECK_HRESULT_CODE_BAIL(hr = m_pMediaController->Run());
    m_bStarted = true;

bail:
    if (COMPV_ERROR_CODE_IS_NOK(err) || FAILED(hr)) {
        COMPV_CHECK_CODE_NOP(stop());
        COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_DIRECTSHOW);
    }
    return err;
}

COMPV_ERROR_CODE CompVDSGraphCapture::stop()
{
    if (m_pGrabber) {
        COMPV_CHECK_HRESULT_CODE_NOP(m_pGrabber->SetCallback(NULL, 1)); // Dec(m_pcSampleGrabberCB.refCount)
    }
    if (m_pMediaController) {
        COMPV_CHECK_HRESULT_CODE_NOP(m_pMediaController->Stop());
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVDSGraphCapture::init(const std::string& deviceId COMPV_DEFAULT(""))
{
    if (m_bInit) {
        return COMPV_ERROR_CODE_S_OK;
    }
    HRESULT hr = S_OK;
    COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;

    // Graph builder
    COMPV_CHECK_HRESULT_CODE_BAIL(hr = COMPV_DS_COCREATE(CLSID_CaptureGraphBuilder2, IID_ICaptureGraphBuilder2, m_pCaptureGraphBuilder));
    COMPV_CHECK_HRESULT_CODE_BAIL(hr = COMPV_DS_COCREATE(CLSID_FilterGraph, IID_IGraphBuilder, m_pGraphBuilder));
    COMPV_CHECK_HRESULT_CODE_BAIL(hr = m_pCaptureGraphBuilder->SetFiltergraph(m_pGraphBuilder));

    // Sample graber
    COMPV_CHECK_HRESULT_CODE_BAIL(hr = COMPV_DS_COCREATE(CLSID_SampleGrabber, IID_IBaseFilter, m_pFilterSampleGrabber));
    COMPV_CHECK_HRESULT_CODE_BAIL(hr = m_pGraphBuilder->AddFilter(m_pFilterSampleGrabber, TEXT("SampleGrabber")));
    COMPV_CHECK_HRESULT_CODE_BAIL(hr = COMPV_DS_QUERY(m_pFilterSampleGrabber, IID_ISampleGrabber, m_pGrabber));
    COMPV_CHECK_HRESULT_CODE_BAIL(hr = m_pGrabber->SetOneShot(FALSE));
    COMPV_CHECK_HRESULT_CODE_BAIL(hr = m_pGrabber->SetBufferSamples(FALSE));

    // NULL renderer
    COMPV_CHECK_HRESULT_CODE_BAIL(hr = COMPV_DS_COCREATE(CLSID_NullRenderer, IID_IBaseFilter, m_pFilterNullRenderer));
    COMPV_CHECK_HRESULT_CODE_BAIL(hr = m_pGraphBuilder->AddFilter(m_pFilterNullRenderer, TEXT("NullRenderer")));

    // Source / Camera
    COMPV_CHECK_CODE_BAIL(err = CompVDSUtils::createSourceFilter(&m_pFilterSource, deviceId));
    COMPV_CHECK_HRESULT_CODE_BAIL(hr = m_pGraphBuilder->AddFilter(m_pFilterSource, TEXT("Source")));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = m_pCaptureGraphBuilder->FindInterface(
		&PIN_CATEGORY_CAPTURE,
		&MEDIATYPE_Video,
		m_pFilterSource,
		IID_IAMStreamConfig,
		reinterpret_cast<void**>(&m_pStreamConfig)));

    // Media controller
    COMPV_CHECK_HRESULT_CODE_BAIL(hr = COMPV_DS_QUERY(m_pGraphBuilder, IID_IMediaControl, m_pMediaController));

    m_bInit = true;

bail:
    if (FAILED(hr) || COMPV_ERROR_CODE_IS_NOK(err)) {
        COMPV_CHECK_CODE_NOP(deInit());
        COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_DIRECTSHOW);
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVDSGraphCapture::deInit()
{
    COMPV_CHECK_CODE_NOP(stop());
    COMPV_CHECK_CODE_NOP(disconnect());

    COMPV_DS_SAFE_RELEASE(m_pCaptureGraphBuilder);
    COMPV_DS_SAFE_RELEASE(m_pGraphBuilder);

    COMPV_DS_SAFE_RELEASE(m_pMediaController);
    COMPV_DS_SAFE_RELEASE(m_pMediaEvent);

    COMPV_DS_SAFE_RELEASE(m_pGrabber);

    COMPV_DS_SAFE_RELEASE(m_pFilterSampleGrabber);
    COMPV_DS_SAFE_RELEASE(m_pFilterNullRenderer);
    COMPV_DS_SAFE_RELEASE(m_pFilterSource);
	COMPV_DS_SAFE_RELEASE(m_pStreamConfig);

    m_bInit = false;

    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVDSGraphCapture::connect(const std::string& deviceId COMPV_DEFAULT(""))
{
    if (m_bConnected) {
        return COMPV_ERROR_CODE_S_OK;
    }
    COMPV_CHECK_CODE_RETURN(init(deviceId));

    COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
    HRESULT hr = S_OK;

    COMPV_CHECK_CODE_BAIL(err = CompVDSUtils::connectFilters(m_pGraphBuilder, m_pFilterSource, m_pFilterSampleGrabber));
    COMPV_CHECK_CODE_BAIL(err = CompVDSUtils::connectFilters(m_pGraphBuilder, m_pFilterSampleGrabber, m_pFilterNullRenderer));
	m_bConnected = true; // required by queryCapNeg
	COMPV_CHECK_CODE_BAIL(err = queryCapNeg());	

bail:
    if (COMPV_ERROR_CODE_IS_NOK(err) || FAILED(hr)) {
        COMPV_CHECK_CODE_NOP(disconnect());
        return err;
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVDSGraphCapture::disconnect()
{
    if (m_pGraphBuilder && m_pFilterSource && m_pFilterSampleGrabber) {
        COMPV_CHECK_CODE_NOP(CompVDSUtils::disconnectFilters(m_pGraphBuilder, m_pFilterSource, m_pFilterSampleGrabber));
    }
    if (m_pGraphBuilder && m_pFilterSampleGrabber && m_pFilterNullRenderer) {
        COMPV_CHECK_CODE_NOP(CompVDSUtils::disconnectFilters(m_pGraphBuilder, m_pFilterSampleGrabber, m_pFilterNullRenderer));
    }
    m_bConnected = false;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVDSGraphCapture::addSource(const std::string& deviceId COMPV_DEFAULT(""))
{
	// We can only add source before the end of the initialization
	COMPV_CHECK_EXP_RETURN(m_bInit || m_bStarted || m_pFilterSource || m_pStreamConfig || !m_pGraphBuilder || !m_pCaptureGraphBuilder, COMPV_ERROR_CODE_E_INVALID_STATE);
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	HRESULT hr = S_OK;
	bool filterAdded = false;
	COMPV_CHECK_CODE_BAIL(err = CompVDSUtils::createSourceFilter(&m_pFilterSource, deviceId));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = m_pGraphBuilder->AddFilter(m_pFilterSource, TEXT("Source")));
	filterAdded = true;
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = m_pCaptureGraphBuilder->FindInterface(
		&PIN_CATEGORY_CAPTURE,
		&MEDIATYPE_Video,
		m_pFilterSource,
		IID_IAMStreamConfig,
		reinterpret_cast<void**>(&m_pStreamConfig)));
bail:
	if (FAILED(hr) || COMPV_ERROR_CODE_IS_NOK(err)) {
		if (filterAdded) {
			COMPV_CHECK_HRESULT_CODE_NOP(m_pGraphBuilder->RemoveFilter(m_pFilterSource));
		}
		COMPV_DS_SAFE_RELEASE(m_pFilterSource);
		COMPV_DS_SAFE_RELEASE(m_pStreamConfig);
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_DIRECTSHOW);
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVDSGraphCapture::queryCapNeg()
{
	COMPV_CHECK_EXP_RETURN(!m_bConnected, COMPV_ERROR_CODE_E_INVALID_STATE);
	HRESULT hr = S_OK;
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = m_pGrabber->GetConnectedMediaType(&m_ConnectedMediaType));
	COMPV_CHECK_HRESULT_EXP_BAIL(!InlineIsEqualGUID(m_ConnectedMediaType.formattype, FORMAT_VideoInfo), hr = E_FAIL);
	COMPV_CHECK_CODE_BAIL(err = CompVDSUtils::mediaTypeToCaps(&m_ConnectedMediaType, m_CapsNeg));
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Neg caps -> %s", m_CapsNeg.toString().c_str());
bail:
	if (FAILED(hr) || COMPV_ERROR_CODE_IS_NOK(err)) {
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_DIRECTSHOW);
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVDSGraphCapture::applyCaps()
{
	COMPV_CHECK_EXP_RETURN(m_bStarted || !m_bInit, COMPV_ERROR_CODE_E_INVALID_STATE);
	AM_MEDIA_TYPE* mediaType = NULL;
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	HRESULT hr = S_OK;

	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "applyCaps(%s)", m_CapsPref.toString().c_str());
	COMPV_CHECK_CODE_BAIL(err = CompVDSUtils::capsToMediaType(m_pStreamConfig, m_CapsPref, mediaType));
	hr = m_pStreamConfig->SetFormat(mediaType);
	if (FAILED(hr)) {
		std::vector<CompVDSCameraCaps> capsSupported;
		CompVDSCameraCaps best;
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Failed (hr=%08x) to apply caps (%s), trying to use best cap *without* fps", hr, m_CapsPref.toString().c_str());
		COMPV_CHECK_HRESULT_CODE_BAIL(hr = CompVDSUtils::supportedCaps(m_pFilterSource, capsSupported)); // get supported caps
		COMPV_CHECK_CODE_BAIL(err = CompVDSUtils::bestCap(capsSupported, m_CapsPref, best, true)); // ignore fps
		COMPV_CHECK_CODE_BAIL(err = CompVDSUtils::capsToMediaType(m_pStreamConfig, best, mediaType));
		hr = m_pStreamConfig->SetFormat(mediaType);
		if (FAILED(hr)) {
			COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Failed to apply caps (%s) without, trying to use best cap *with* fps", m_CapsPref.toString().c_str());
			COMPV_CHECK_CODE_BAIL(err = CompVDSUtils::bestCap(capsSupported, m_CapsPref, best, false)); // do not ignore fps
			COMPV_CHECK_CODE_BAIL(err = CompVDSUtils::capsToMediaType(m_pStreamConfig, best, mediaType));
			COMPV_CHECK_HRESULT_CODE_BAIL(hr = m_pStreamConfig->SetFormat(mediaType));
		}
	}
	COMPV_CHECK_CODE_BAIL(err = queryCapNeg());

bail:
	if (mediaType) {
		DeleteMediaType(mediaType);
		mediaType = NULL;
	}
	if (FAILED(hr) || COMPV_ERROR_CODE_IS_NOK(err)) {
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_DIRECTSHOW);
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVDSGraphCapture::newObj(CompVDSGraphCapturePtrPtr graph, const ISampleGrabberCB* pcSampleGrabberCB)
{
    COMPV_CHECK_EXP_RETURN(!graph || !pcSampleGrabberCB, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVDSGraphCapturePtr graph_ = new CompVDSGraphCapture(pcSampleGrabberCB);
    COMPV_CHECK_EXP_RETURN(!graph_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    *graph = graph_;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()