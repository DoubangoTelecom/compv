/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_PLUGIN_MFOUNDATION_CAMERA_H_)
#define _COMPV_PLUGIN_MFOUNDATION_CAMERA_H_

#include "compv/mf/compv_mf_config.h"
#include "compv/mf/compv_mf_grabber.h"
#include "compv/mf/compv_mf_devices.h"
#include "compv/camera/compv_camera.h"
#include "compv/base/compv_obj.h"
#include "compv/base/compv_lock.h"
#include "compv/base/parallel/compv_thread.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

COMPV_OBJECT_DECLARE_PTRS(MFCamera)

class CompVMFCamera : public CompVCamera, public CompVLock
{
protected:
	CompVMFCamera();
public:
	virtual ~CompVMFCamera();
	COMPV_OBJECT_GET_ID(CompVMFCamera);

	virtual COMPV_ERROR_CODE devices(CompVCameraDeviceInfoList& list) override /* Overrides(CompVCamera) */;
	virtual COMPV_ERROR_CODE start(const std::string& deviceId = "") override /* Overrides(CompVCamera) */;
	virtual COMPV_ERROR_CODE stop() override /* Overrides(CompVCamera) */;

	virtual COMPV_ERROR_CODE set(int id, const void* valuePtr, size_t valueSize) override /* Overrides(CompVCaps) */;
	virtual COMPV_ERROR_CODE get(int id, const void** valuePtrPtr, size_t valueSize) override /* Overrides(CompVCaps) */;

	static COMPV_ERROR_CODE newObj(CompVMFCameraPtrPtr camera);

private:
	COMPV_ERROR_CODE device(__in const char* pszId, __out IMFActivate **ppActivate);
	COMPV_ERROR_CODE shutdown();
	COMPV_ERROR_CODE init(const std::string& deviceId);
	COMPV_ERROR_CODE deInit();
	HRESULT queryCapNeg(IMFTopology *pTopology);
	static HRESULT STDMETHODCALLTYPE BufferCB(REFGUID guidMajorMediaType, DWORD dwSampleFlags,
		LONGLONG llSampleTime, LONGLONG llSampleDuration, const BYTE * pSampleBuffer,
		DWORD dwSampleSiz, const void *pcUserData);
	static void *COMPV_STDCALL RunSessionThread(void * arg);
	

private:
	bool m_bInitialized;
	bool m_bStarted;
	CompVMFCameraCaps m_CapsPref;
	CompVMFCameraCaps m_CapsNeg;
	COMPV_SUBTYPE m_eSubTypeNeg;
	CompVThreadPtr m_ptrThread;
	CompVMatPtr m_ptrImageCB;
	CompVMFDeviceListVideoPtr m_ptrDeviceListVideo;

	HWND m_hWndPreview;
	IMFMediaSession* m_pSession;
	IMFMediaSource* m_pSource;
	CompVMFGrabberCB* m_pCallback;
	IMFActivate* m_pSinkGrabber;
	IMFActivate* m_pSinkActivatePreview;
	IMFTopology* m_pTopology;
	IMFMediaType* m_pGrabberInputType;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_PLUGIN_MFOUNDATION_CAMERA_H_ */
