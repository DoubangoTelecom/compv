/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/mf/compv_mf_camera.h"
#include "compv/mf/compv_mf_utils.h"

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
	COMPV_CHECK_CODE_RETURN(CompVMFUtils::devices(list));
	return COMPV_ERROR_CODE_S_OK;
}
	
COMPV_ERROR_CODE CompVMFCamera::start(const std::string& deviceId COMPV_DEFAULT("")) /* Overrides(CompVCamera) */
{
	CompVAutoLock<CompVMFCamera>(this);
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "%s(%s)", __FUNCTION__, deviceId.c_str());
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	
	COMPV_CHECK_CODE_BAIL(err = init(deviceId));

	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);

bail:
	return err;
}
	
COMPV_ERROR_CODE CompVMFCamera::stop() /* Overrides(CompVCamera) */
{
	CompVAutoLock<CompVMFCamera>(this);
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "%s", __FUNCTION__);
	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}
	
COMPV_ERROR_CODE CompVMFCamera::set(int id, const void* valuePtr, size_t valueSize) /* Overrides(CompVCaps) */
{
	CompVAutoLock<CompVMFCamera>(this);
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "%s(%d, %p, %zu)", __FUNCTION__, id, valuePtr, valueSize);
	COMPV_CHECK_EXP_RETURN(!valuePtr || !valueSize, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	switch (id) {
		case COMPV_CAMERA_CAP_INT_WIDTH: {
			COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
			m_CapsPref.width = *reinterpret_cast<const int*>(valuePtr);
			return COMPV_ERROR_CODE_S_OK;
		}
		case COMPV_CAMERA_CAP_INT_HEIGHT: {
			COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
			m_CapsPref.height = *reinterpret_cast<const int*>(valuePtr);
			return COMPV_ERROR_CODE_S_OK;
		}
		case COMPV_CAMERA_CAP_INT_FPS: {
			COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
			m_CapsPref.fps = *reinterpret_cast<const int*>(valuePtr);
			return COMPV_ERROR_CODE_S_OK;
		}
		case COMPV_CAMERA_CAP_INT_SUBTYPE: {
			COMPV_CHECK_CODE_RETURN(CompVMFUtils::convertSubType(static_cast<COMPV_SUBTYPE>(*reinterpret_cast<const int*>(valuePtr)), m_CapsPref.subType));
			return COMPV_ERROR_CODE_S_OK;
		}
		default: {
			COMPV_DEBUG_ERROR("Media Foundation camera implementation doesn't support capability id %d", id);
			return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
		}
	}
}
	
COMPV_ERROR_CODE CompVMFCamera::get(int id, const void*& valuePtr, size_t valueSize) /* Overrides(CompVCaps) */
{
	CompVAutoLock<CompVMFCamera>(this);
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "%s(%d, %p, %zu)", __FUNCTION__, id, valuePtr, valueSize);
	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
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
	IMFActivate* pSinkGrabber = NULL;
	IMFMediaSession* pSession = NULL;
	IMFActivate* pSinkActivatePreview = NULL;
	CompVMFCameraCaps capsToSet = m_CapsPref;
	
	COMPV_CHECK_CODE_BAIL(err = CompVMFUtils::device(deviceId.c_str(), &pActivate));

	// Create the media source for the device.
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pActivate->ActivateObject(
		__uuidof(IMFMediaSource),
		(void**)&pSource
	));

	// Check whether video processor (http://msdn.microsoft.com/en-us/library/windows/desktop/hh162913(v=vs.85).aspx) is supported
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = CompVMFUtils::isVideoProcessorSupported(&bVideoProcessorSupported));
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "bVideoProcessorSupported = %s", bVideoProcessorSupported ? "YES" : "NO");

	// Must not be set because not supported by Frame Rate Converter DSP (http://msdn.microsoft.com/en-us/library/windows/desktop/ff819100(v=vs.85).aspx).aspx) because of color (neither I420 nor NV12)
	// Video Processor (http://msdn.microsoft.com/en-us/library/windows/desktop/hh162913(v=vs.85).aspx) supports both NV12 and I420
	COMPV_DEBUG_INFO_CODE_FOR_TESTING(); // Implement '!bVideoProcessorSupported'
	if (!bVideoProcessorSupported) {
		// FIXME(dmi): implement and update 'capsToSet'
		COMPV_DEBUG_INFO_CODE_NOT_TESTED();
	}

	// Set session attributes
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = MFCreateAttributes(&pSessionAttributes, 1));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pSessionAttributes->SetUINT32(PLUGIN_MF_LOW_LATENCY, TRUE));

	// Configure the media type that the Sample Grabber will receive.
	// Setting the major and subtype is usually enough for the topology loader
	// to resolve the topology.

	COMPV_CHECK_HRESULT_CODE_BAIL(hr = MFCreateMediaType(&pGrabberInputType));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pGrabberInputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pGrabberInputType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive));

	COMPV_CHECK_HRESULT_CODE_BAIL(hr = MFSetAttributeSize(pGrabberInputType, MF_MT_FRAME_SIZE, static_cast<UINT32>(capsToSet.width), static_cast<UINT32>(capsToSet.height)));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = MFSetAttributeRatio(pGrabberInputType, MF_MT_FRAME_RATE, static_cast<UINT32>(capsToSet.fps), 1));

	COMPV_CHECK_HRESULT_CODE_BAIL(hr = MFSetAttributeRatio(pGrabberInputType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pGrabberInputType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pGrabberInputType->SetUINT32(MF_MT_FIXED_SIZE_SAMPLES, TRUE));

	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pGrabberInputType->SetGUID(MF_MT_SUBTYPE, capsToSet.subType));

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
	COMPV_CHECK_CODE_NOP(stop());
	if (m_pSource) {
		COMPV_CHECK_HRESULT_CODE_NOP(m_pSource->Shutdown());
	}
	if (m_pSession) {
		COMPV_CHECK_HRESULT_CODE_NOP(m_pSession->Shutdown());
	}
	
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
	COMPV_CHECK_HRESULT_CODE_RETURN(E_NOTIMPL);
	return S_OK;
}

COMPV_NAMESPACE_END()
