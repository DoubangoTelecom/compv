/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/mf/compv_mf_camera.h"
#include "compv/mf/compv_mf_utils.h"
#include "compv/base/image/compv_image.h"

#include <initguid.h>

#define COMPV_THIS_CLASSNAME "CompVMFCamera"

DEFINE_GUID(PLUGIN_MF_LOW_LATENCY,
	0x9c27891a, 0xed7a, 0x40e1, 0x88, 0xe8, 0xb2, 0x27, 0x27, 0xa0, 0x24, 0xee);

COMPV_NAMESPACE_BEGIN()

CompVMFCamera::CompVMFCamera()
	: CompVCamera()
	, CompVLock()
	, m_bInitialized(false)
	, m_bStarted(false)
	, m_eSubTypeNeg(COMPV_SUBTYPE_NONE)
	, m_hWndPreview(NULL)
	, m_pSession(NULL)
	, m_pSource(NULL)
	, m_pCallback(NULL)
	, m_pSinkGrabber(NULL)
	, m_pSinkActivatePreview(NULL)
	, m_pTopology(NULL)
	, m_pGrabberInputType(NULL)
{
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "%s", __FUNCTION__);
}

CompVMFCamera::~CompVMFCamera()
{
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "%s", __FUNCTION__);
	COMPV_CHECK_CODE_NOP(deInit());
}

COMPV_ERROR_CODE CompVMFCamera::devices(CompVCameraDeviceInfoList& list) /* Overrides(CompVCamera) */
{
	CompVAutoLock<CompVMFCamera>(this);
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "%s", __FUNCTION__);
	CompVCameraDeviceInfoList list_;
	HRESULT hr = S_OK;
	UINT32 count;
	WCHAR *_pszName = NULL;
	WCHAR *_pszId = NULL;
	char pczString[MAX_PATH] = { 0 };
	int length;
	std::string name, id;

	if (!m_ptrDeviceListVideo) {
		COMPV_CHECK_CODE_RETURN(CompVMFDeviceListVideo::newObj(&m_ptrDeviceListVideo));
	}

	COMPV_CHECK_HRESULT_CODE_BAIL(hr = m_ptrDeviceListVideo->enumerateDevices());
	count = m_ptrDeviceListVideo->count();

	for (UINT32 i = 0; i < count; ++i) {
		if (SUCCEEDED(m_ptrDeviceListVideo->deviceName(i, &_pszName)) && SUCCEEDED(m_ptrDeviceListVideo->deviceId(i, &_pszId))) {
			// Unique Id
			if ((length = static_cast<int>(wcstombs(pczString, _pszId, MAX_PATH))) <= 0) {
				COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "wcstombs(%ls) failed: %d", _pszId, length);
				goto next;
			}
			id = std::string(pczString, length);
			// Friendly name
			if ((length = static_cast<int>(wcstombs(pczString, _pszName, MAX_PATH))) <= 0) {
				COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "wcstombs(%ls) failed: %d", _pszName, length);
				goto next;
			}
			name = std::string(pczString, length);
			// Add to the list
			list_.push_back(CompVCameraDeviceInfo(id, name, name));
		}
	next:
		if (_pszName) {
			CoTaskMemFree(_pszName), _pszName = NULL;
		}
		if (_pszId) {
			CoTaskMemFree(_pszId), _pszId = NULL;
		}
	}

	list = list_;

bail:
	if (_pszName) {
		CoTaskMemFree(_pszName);
	}
	if (_pszId) {
		CoTaskMemFree(_pszId);
	}
	COMPV_CHECK_EXP_RETURN(FAILED(hr), COMPV_ERROR_CODE_E_MFOUNDATION);
	return COMPV_ERROR_CODE_S_OK;
}
	
COMPV_ERROR_CODE CompVMFCamera::start(const std::string& deviceId COMPV_DEFAULT("")) /* Overrides(CompVCamera) */
{
	CompVAutoLock<CompVMFCamera>(this);
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "%s(%s)", __FUNCTION__, deviceId.c_str());
	if (m_bStarted) {
		return COMPV_ERROR_CODE_S_OK;
	}
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	HRESULT hr = S_OK;
	
	COMPV_CHECK_CODE_BAIL(err = init(deviceId));

	// Run the media session.
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = CompVMFUtils::runSession(m_pSession, m_pTopology));

	// Start asynchronous watcher thread
	m_bStarted = true;
	COMPV_CHECK_CODE_BAIL(err = CompVThread::newObj(&m_ptrThread, CompVMFCamera::RunSessionThread, this));

bail:
	if (FAILED(hr) || COMPV_ERROR_CODE_IS_NOK(err)) {
		COMPV_CHECK_CODE_NOP(stop());
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_MFOUNDATION);
	}
	return err;
}
	
COMPV_ERROR_CODE CompVMFCamera::stop() /* Overrides(CompVCamera) */
{
	CompVAutoLock<CompVMFCamera>(this);
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "%s", __FUNCTION__);
	
#if 0 // TODO(dmi): MF_E_MULTIPLE_SUBSCRIBERS
	// for the thread
	m_bStarted = false;
	if (m_pSession) {
		COMPV_CHECK_HRESULT_CODE_NOP(CompVMFUtils::stopSession(m_pSession, NULL)); // stop session to wakeup the asynchronous thread
	}
	if (m_ptrThread) {
		COMPV_CHECK_CODE_NOP(m_ptrThread->join());
		m_ptrThread = NULL;
	}
	if (m_pSource) {
		COMPV_CHECK_HRESULT_CODE_NOP(CompVMFUtils::stopSession(NULL, m_pSource)); // stop source to release the camera
	}
	return COMPV_ERROR_CODE_S_OK;
#else
	COMPV_CHECK_CODE_NOP(deInit());
	return COMPV_ERROR_CODE_S_OK;
#endif
}
	
COMPV_ERROR_CODE CompVMFCamera::set(int id, const void* valuePtr, size_t valueSize) /* Overrides(CompVCaps) */
{
	CompVAutoLock<CompVMFCamera>(this);
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "%s(%d, %p, %zu)", __FUNCTION__, id, valuePtr, valueSize);
	COMPV_CHECK_EXP_RETURN(!valuePtr || !valueSize, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	switch (id) {
		case COMPV_CAMERA_CAP_INT_WIDTH: {
			COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
			m_CapsPref.width = (*reinterpret_cast<const int*>(valuePtr) + 1) & ~1; // odd numbers not supported by media foundation
			return COMPV_ERROR_CODE_S_OK;
		}
		case COMPV_CAMERA_CAP_INT_HEIGHT: {
			COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
			m_CapsPref.height = (*reinterpret_cast<const int*>(valuePtr) + 1) & ~1; // odd numbers not supported by media foundation
			return COMPV_ERROR_CODE_S_OK;
		}
		case COMPV_CAMERA_CAP_INT_FPS: {
			COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
			m_CapsPref.numFps = *reinterpret_cast<const int*>(valuePtr);
			m_CapsPref.denFps = 1;
			return COMPV_ERROR_CODE_S_OK;
		}
		case COMPV_CAMERA_CAP_INT_SUBTYPE: {
			COMPV_CHECK_CODE_RETURN(CompVMFUtils::convertSubType(static_cast<COMPV_SUBTYPE>(*reinterpret_cast<const int*>(valuePtr)), m_CapsPref.subType));
			return COMPV_ERROR_CODE_S_OK;
		}
		case COMPV_CAMERA_CAP_BOOL_AUTOFOCUS: {
			COMPV_CHECK_EXP_RETURN(valueSize != sizeof(bool), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
			m_CapsPref.autofocus = *reinterpret_cast<const bool*>(valuePtr) ? TRUE : FALSE;
			return COMPV_ERROR_CODE_S_OK;
		}
		default: {
			COMPV_DEBUG_ERROR("Media Foundation camera implementation doesn't support capability id %d", id);
			return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
		}
	}
}
	
COMPV_ERROR_CODE CompVMFCamera::get(int id, const void** valuePtrPtr, size_t valueSize) /* Overrides(CompVCaps) */
{
	CompVAutoLock<CompVMFCamera>(this);
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "%s(%d, %p, %zu)", __FUNCTION__, id, valuePtrPtr, valueSize);
	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMFCamera::device(__in const char* pszId, __out IMFActivate **ppActivate)
{
	CompVAutoLock<CompVMFCamera>(this);
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "%s(%s)", __FUNCTION__, pszId);
	COMPV_CHECK_EXP_RETURN(!ppActivate, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_EXP_RETURN(!m_ptrDeviceListVideo, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	wchar_t pczwSrcUniqueId[MAX_PATH] = { 0 };
	if (pszId) {
		int length;
		length = static_cast<int>(mbstowcs(pczwSrcUniqueId, pszId, sizeof(pczwSrcUniqueId) / sizeof(pczwSrcUniqueId[0])));
		COMPV_CHECK_EXP_RETURN(length < 0, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	}
	COMPV_CHECK_EXP_RETURN(FAILED(m_ptrDeviceListVideo->deviceWithId(pczwSrcUniqueId, ppActivate)), COMPV_ERROR_CODE_E_MFOUNDATION);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMFCamera::shutdown()
{
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "%s", __FUNCTION__);
	// for the thread
	m_bStarted = false;
	if (m_pSession) {
		COMPV_CHECK_HRESULT_CODE_NOP(CompVMFUtils::shutdownSession(m_pSession, NULL)); // stop session to wakeup the asynchronous thread
	}
	if (m_ptrThread) {
		COMPV_CHECK_CODE_NOP(m_ptrThread->join());
		m_ptrThread = NULL;
	}
	if (m_pSource) {
		COMPV_CHECK_HRESULT_CODE_NOP(CompVMFUtils::shutdownSession(NULL, m_pSource)); // stop source to release the camera
	}
	if (m_ptrDeviceListVideo) {
		m_ptrDeviceListVideo->clear(); // Not allowed to reuse a device after shutdown
	}

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMFCamera::init(const std::string& deviceId)
{
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "%s(%s)", __FUNCTION__, deviceId.c_str());
	if (m_bInitialized) {
		return COMPV_ERROR_CODE_S_OK;
	}
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	HRESULT hr = S_OK;
	CompVMFCameraCaps capsGrabber = m_CapsPref;
	CompVMFCameraCaps capsSource;
	std::vector<CompVMFCameraCaps> supportedCapsList;
	BOOL bVideoProcessorSupported = FALSE;
	IMFMediaSource* pSource = NULL;
	IMFAttributes* pSessionAttributes = NULL;
	IMFTopology *pTopology = NULL;
	IMFMediaSink* pEvr = NULL;
	IMFMediaType* pEncoderInputType = NULL;
	IMFTopologyNode* pNodeGrabber = NULL;
	IMFMediaType* pGrabberNegotiatedInputMedia = NULL;
	IMFActivate* pActivate = NULL;
	IMFMediaType* pGrabberInputType = NULL;
	IMFMediaType* pSourceOutputType = NULL;
	IMFActivate* pSinkGrabber = NULL;
	IMFMediaSession* pSession = NULL;
	IMFActivate* pSinkActivatePreview = NULL;

	// Enumerate the devices
	if (!m_ptrDeviceListVideo) {
		COMPV_CHECK_HRESULT_CODE_BAIL(hr = CompVMFDeviceListVideo::newObj(&m_ptrDeviceListVideo));
	}
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = m_ptrDeviceListVideo->enumerateDevices());

	// Create the media source for the device.
	COMPV_CHECK_CODE_BAIL(err = device(deviceId.c_str(), &pActivate));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pActivate->ActivateObject(
		__uuidof(IMFMediaSource),
		(void**)&pSource
	));

#if COMPV_MF_USE_PROCESSOR_IN_TOPO
	// Check whether video processor (http://msdn.microsoft.com/en-us/library/windows/desktop/hh162913(v=vs.85).aspx) is supported
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = CompVMFUtils::isVideoProcessorSupported(&bVideoProcessorSupported));
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "bVideoProcessorSupported = %s", bVideoProcessorSupported ? "YES" : "NO");
#endif

	// Get the list of supported caps
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = CompVMFUtils::supportedCaps(pSource, supportedCapsList));

	// Get caps for the source. Must match all configs (width, height, fps, subtype...)
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = CompVMFUtils::bestCap(supportedCapsList, m_CapsPref, capsSource, FALSE, FALSE, FALSE));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = CompVMFUtils::capsToMediaType(capsSource, &pSourceOutputType));

	// When video processor not supported then, we have to choose best cap supported by the
	// resizer (https://msdn.microsoft.com/en-us/library/windows/desktop/ff819491(v=vs.85).aspx),
	// frame rate converter (https://msdn.microsoft.com/en-us/library/windows/desktop/ff819100(v=vs.85).aspx) and 
	// color converter (https://msdn.microsoft.com/en-us/library/windows/desktop/ff819079(v=vs.85).aspx)
	if (!bVideoProcessorSupported) {
		BOOL supportResizeDSP = FALSE;
		BOOL supportFpsDSP = FALSE;
		BOOL supportColorDSP = FALSE;
		CompVMFCameraCaps capsPref = m_CapsPref;
		// Check if we can ignore the subType (thanks to the color converter DSP)
		// The filters will be chained in this order: source -> [framerate -> resizer -> color] -> grabber.
		if (capsGrabber.subType != capsSource.subType) {
			COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "The requested subtype(%s) is different than the source subtype(%s). Trying to add a color convertor dsp if possible",
				CompVMFUtils::guidName(capsGrabber.subType), CompVMFUtils::guidName(capsSource.subType));
			COMPV_CHECK_HRESULT_CODE_BAIL(hr = CompVMFUtils::isSupportedByColorConverter(capsSource.subType, capsGrabber.subType, &supportColorDSP));
			if (supportColorDSP) {
				COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "/!\\Forcing the topology to add a color converter(%s -> %s) dsp: performance issue. You should not use 'COMPV_CAMERA_CAP_INT_SUBTYPE' to force the subType.",
					CompVMFUtils::guidName(capsGrabber.subType), CompVMFUtils::guidName(capsSource.subType));
			}
			else {
				COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Color convertor doesn't support requested conversion(%s -> %s), using %s as default and best subtype",
					CompVMFUtils::guidName(capsGrabber.subType), CompVMFUtils::guidName(capsSource.subType), CompVMFUtils::guidName(capsSource.subType));
			}
		}
		COMPV_CHECK_HRESULT_CODE_BAIL(hr = CompVMFUtils::isSupportedByFrameRateConvert(capsSource.subType, &supportFpsDSP));
		COMPV_CHECK_HRESULT_CODE_BAIL(hr = CompVMFUtils::isSupportedByVideoResizer(capsSource.subType, &supportResizeDSP));
		if (!supportColorDSP) {
			capsPref.subType = capsSource.subType;
		}
		if (!supportFpsDSP) {
			capsPref.numFps = capsSource.numFps, capsPref.denFps = capsSource.denFps;
		}
		if (!supportResizeDSP) {
			capsPref.width = capsSource.width, capsPref.height = capsSource.height;
		}
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "supportColorDSP=%d, supportFpsDSP=%d, supportResizeDSP=%d", supportColorDSP, supportFpsDSP, supportResizeDSP);
		COMPV_CHECK_HRESULT_CODE_BAIL(hr = CompVMFUtils::bestCap(supportedCapsList, capsPref, capsGrabber, supportColorDSP, supportResizeDSP, supportFpsDSP));
		if ((capsGrabber.numFps / capsGrabber.denFps) > (capsSource.numFps / capsSource.denFps)) {
			// Do not duplicate frames when 'requested fps' > 'supported fps'
			// ..but drop frames when 'requested fps' > 'supported fps'
			capsGrabber.numFps = capsSource.numFps;
			capsGrabber.denFps = capsSource.denFps;
		}
		capsGrabber.autofocus = capsSource.autofocus;
	}

	// Set session attributes
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = MFCreateAttributes(&pSessionAttributes, 1));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pSessionAttributes->SetUINT32(PLUGIN_MF_LOW_LATENCY, TRUE));

	// Configure the media type that the Sample Grabber will receive.
	// Setting the major and subtype is usually enough for the topology loader
	// to resolve the topology.

	COMPV_CHECK_HRESULT_CODE_BAIL(hr = CompVMFUtils::createVideoType(&capsGrabber.subType, &pGrabberInputType, capsGrabber.width, capsGrabber.height, capsGrabber.numFps, capsGrabber.denFps));

	// Create the sample grabber sink.
	if (!m_pCallback) {
		COMPV_CHECK_HRESULT_CODE_BAIL(hr = CompVMFGrabberCB::CreateInstance(CompVMFCamera::BufferCB, this, &m_pCallback));
	}
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = MFCreateSampleGrabberSinkActivate(pGrabberInputType, m_pCallback, &pSinkGrabber));

	// To run as fast as possible, set this attribute (requires Windows 7):
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pSinkGrabber->SetUINT32(MF_SAMPLEGRABBERSINK_IGNORE_CLOCK, TRUE));

	// Create the Media Session.
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = MFCreateMediaSession(pSessionAttributes, &pSession));

	// Create the EVR activation object for the preview.
	if (m_hWndPreview) {
		COMPV_CHECK_HRESULT_CODE_BAIL(hr = MFCreateVideoRendererActivate(m_hWndPreview, &pSinkActivatePreview));
	}

#if 0
	// Set auto-focus
	if (capsGrabber.autofocus) { // supported ?
		/*hr = */CompVMFUtils::setAutoFocus(pSource, m_CapsPref.autofocus);
	}
#endif

	// Create the topology.
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Create topology: source(%s) -> grabber(%s), pref = %s", capsGrabber.toString().c_str(), capsGrabber.toString().c_str(), m_CapsPref.toString().c_str());
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = CompVMFUtils::createTopology(
		pSource,
		NULL,
		pSinkGrabber,
		pSinkActivatePreview,
		pSourceOutputType,
		pGrabberInputType,
		&pTopology));
	// Resolve topology (adds video processors if needed).
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = CompVMFUtils::resolveTopology(pTopology, &pTopology));

	// Find EVR for the preview.
	if (pSinkActivatePreview) {
		COMPV_CHECK_HRESULT_CODE_BAIL(hr = CompVMFUtils::findNodeObject(pTopology, CompVMFUtils::s_ullTopoIdSinkPreview, (void**)&pEvr));
	}

	// Find negotiated media m_CapsNeg
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = queryCapNeg(pTopology));

	m_pSession = pSession, pSession = NULL;
	m_pSource = pSource, pSource = NULL;
	m_pSinkGrabber = pSinkGrabber, pSinkGrabber = NULL;
	m_pSinkActivatePreview = pSinkActivatePreview, pSinkActivatePreview = NULL;
	m_pTopology = pTopology, pTopology = NULL;
	m_pGrabberInputType = pGrabberInputType, pGrabberInputType = NULL;

bail:
	COMPV_MF_SAFE_RELEASE(&pSource);
	COMPV_MF_SAFE_RELEASE(&pSessionAttributes);
	COMPV_MF_SAFE_RELEASE(&pTopology);
	COMPV_MF_SAFE_RELEASE(&pEvr);
	COMPV_MF_SAFE_RELEASE(&pEncoderInputType);
	COMPV_MF_SAFE_RELEASE(&pNodeGrabber);
	COMPV_MF_SAFE_RELEASE(&pGrabberNegotiatedInputMedia);
	COMPV_MF_SAFE_RELEASE(&pActivate);
	COMPV_MF_SAFE_RELEASE(&pGrabberInputType);
	COMPV_MF_SAFE_RELEASE(&pSourceOutputType);
	COMPV_MF_SAFE_RELEASE(&pSinkGrabber);
	COMPV_MF_SAFE_RELEASE(&pSession);
	COMPV_MF_SAFE_RELEASE(&pSinkActivatePreview);

	if (FAILED(hr) || COMPV_ERROR_CODE_IS_NOK(err)) {
		COMPV_CHECK_CODE_NOP(deInit());
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_MFOUNDATION);
	}
	return err;
}

COMPV_ERROR_CODE CompVMFCamera::deInit()
{
	COMPV_CHECK_CODE_NOP(shutdown());
	
	COMPV_MF_SAFE_RELEASE(&m_pSession);
	COMPV_MF_SAFE_RELEASE(&m_pSource);
	COMPV_MF_SAFE_RELEASE(&m_pSinkActivatePreview);
	COMPV_MF_SAFE_RELEASE(&m_pCallback);
	COMPV_MF_SAFE_RELEASE(&m_pSinkGrabber);
	COMPV_MF_SAFE_RELEASE(&m_pTopology);
	COMPV_MF_SAFE_RELEASE(&m_pGrabberInputType);

	m_bInitialized = false;

	return COMPV_ERROR_CODE_S_OK;
}

HRESULT CompVMFCamera::queryCapNeg(IMFTopology *pTopology)
{
	COMPV_CHECK_HRESULT_EXP_RETURN(!pTopology, E_INVALIDARG);
	UINT32 nNegWidth = m_CapsNeg.width;
	UINT32 nNegHeight = m_CapsNeg.height;
	UINT32 nNegNumeratorFps = m_CapsNeg.numFps;
	UINT32 nNegDenominatorFps = m_CapsNeg.denFps;
	HRESULT hr = S_OK;
	CompVMFCameraCaps capsNeg = m_CapsNeg;
	COMPV_SUBTYPE subTypeNeg = m_eSubTypeNeg;
	IMFMediaType* pMediaTypeNeg = NULL;
	IMFTopologyNode* pNode = NULL;
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pTopology->GetNodeByID(CompVMFUtils::s_ullTopoIdSinkMain, &pNode));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pNode->GetInputPrefType(0, &pMediaTypeNeg));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pMediaTypeNeg->GetGUID(MF_MT_SUBTYPE, &capsNeg.subType));
	hr = MFGetAttributeSize(pMediaTypeNeg, MF_MT_FRAME_SIZE, &nNegWidth, &nNegHeight);
	if (SUCCEEDED(hr)) {
		capsNeg.width = nNegWidth;
		capsNeg.height = nNegHeight;
	}
	else {
		COMPV_DEBUG_WARN_EX(COMPV_THIS_CLASSNAME, "MFGetAttributeSize failed: %08x", hr);
	}
	hr = MFGetAttributeRatio(pMediaTypeNeg, MF_MT_FRAME_RATE, &nNegNumeratorFps, &nNegDenominatorFps);
	if (SUCCEEDED(hr) && nNegDenominatorFps) {
		capsNeg.numFps = nNegNumeratorFps;
		capsNeg.denFps = nNegDenominatorFps;
	}
	else {
		COMPV_DEBUG_WARN_EX(COMPV_THIS_CLASSNAME, "MFGetAttributeRatio failed(%08x) or nNegDenominatorFps(%u) is equal to zero", hr, nNegDenominatorFps);
	}

	COMPV_CHECK_HRESULT_EXP_BAIL(COMPV_ERROR_CODE_IS_NOK(CompVMFUtils::convertSubType(capsNeg.subType, subTypeNeg)), hr = MF_E_INVALIDMEDIATYPE);

	hr = S_OK;
	m_CapsNeg = capsNeg;
	m_eSubTypeNeg = subTypeNeg;
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Neg caps -> %s", m_CapsNeg.toString().c_str());

bail:
	COMPV_MF_SAFE_RELEASE(&pMediaTypeNeg);
	COMPV_MF_SAFE_RELEASE(&pNode);
	return hr;
}

COMPV_ERROR_CODE CompVMFCamera::newObj(CompVMFCameraPtrPtr camera)
{
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "%s", __FUNCTION__);
	COMPV_CHECK_EXP_RETURN(!camera, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVMFCameraPtr camera_ = new CompVMFCamera();
	COMPV_CHECK_EXP_RETURN(!camera_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

	*camera = *camera_;
	return COMPV_ERROR_CODE_S_OK;
}

HRESULT STDMETHODCALLTYPE CompVMFCamera::BufferCB(REFGUID guidMajorMediaType, DWORD dwSampleFlags,
	LONGLONG llSampleTime, LONGLONG llSampleDuration, const BYTE * pSampleBuffer,
	DWORD dwSampleSiz, const void *pcUserData)
{
	COMPV_CHECK_HRESULT_EXP_RETURN(!pSampleBuffer || !dwSampleSiz, E_POINTER);
	CompVMFCameraPtr camera = const_cast<CompVMFCamera*>(static_cast<const CompVMFCamera*>(pcUserData));
	CompVAutoLock<CompVMFCamera> autoLock(*camera);
	CompVCameraCallbackOnNewFrame& callback = camera->callbackOnNewFrame();
	if (!callback) {
		return S_OK;
	}
	
	COMPV_ERROR_CODE err;
	// Forward the image data to the listeners
	COMPV_CHECK_CODE_BAIL(err = CompVImage::wrap(camera->m_eSubTypeNeg, static_cast<const void*>(pSampleBuffer), static_cast<size_t>(camera->m_CapsNeg.width), static_cast<size_t>(camera->m_CapsNeg.height), static_cast<size_t>(camera->m_CapsNeg.width), &camera->m_ptrImageCB));
	COMPV_CHECK_CODE_BAIL(err = callback(camera->m_ptrImageCB));

bail:
	COMPV_CHECK_HRESULT_EXP_RETURN(COMPV_ERROR_CODE_IS_NOK(err), E_FAIL);
	return S_OK;
}

void *COMPV_STDCALL CompVMFCamera::RunSessionThread(void * arg)
{
	CompVMFCameraPtr ptrCamera = reinterpret_cast<CompVMFCamera *>(arg);
	HRESULT hrStatus = S_OK;
	HRESULT hr = S_OK;
	IMFMediaEvent *pEvent = NULL;
	MediaEventType met;

	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "%s (MF video producer) - ENTER", __FUNCTION__);

	while (ptrCamera->m_bStarted) {
		hr = ptrCamera->m_pSession->GetEvent(0, &pEvent);
		if (hr == MF_E_SHUTDOWN) {
			if (ptrCamera->m_bStarted) {
				COMPV_CHECK_HRESULT_CODE_BAIL(hr); // Shutdown called but "bStarted" not equal to false
			}
			break; // Shutdown called and "bStarted" is equal to false => break the loop
		}
		COMPV_CHECK_HRESULT_CODE_BAIL(hr = pEvent->GetStatus(&hrStatus));
		COMPV_CHECK_HRESULT_CODE_BAIL(hr = pEvent->GetType(&met));
#if 0
		COMPV_CHECK_HRESULT_EXP_BAIL(FAILED(hrStatus) && hrStatus != MF_E_MULTIPLE_SUBSCRIBERS, hrStatus);
#else
		COMPV_CHECK_HRESULT_CODE_BAIL(hr = hrStatus);
#endif
		if (met == MESessionEnded) {
			break;
		}
		COMPV_MF_SAFE_RELEASE(&pEvent);
	}

bail:
	if (FAILED(hr) && ptrCamera->m_bStarted) {
		CompVAutoLock<CompVMFCamera> autoLock(*ptrCamera);
		CompVCameraCallbackOnError& callback = ptrCamera->callbackOnError();
		if (callback) {
			callback(std::string("Media foundation error:") + std::to_string(hr)); // TODO(dmi): set the correct error message
		}
	}
	COMPV_MF_SAFE_RELEASE(&pEvent);
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "%s (MF video producer) - EXIT", __FUNCTION__);

	return NULL;
}

COMPV_NAMESPACE_END()
