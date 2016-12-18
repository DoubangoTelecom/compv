/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/ds/compv_ds_camera.h"
#include "compv/ds/compv_ds_utils.h"

#define COMPV_THIS_CLASSNAME "CompVDSCamera"

COMPV_NAMESPACE_BEGIN()

CompVDSCamera::CompVDSCamera()
    : CompVCamera()
	, CompVLock()
    , m_pGrabber(NULL)
{

}

CompVDSCamera::~CompVDSCamera()
{
    COMPV_CHECK_CODE_NOP(stop());
    COMPV_DS_SAFE_RELEASE(m_pGrabber);
}

COMPV_ERROR_CODE CompVDSCamera::devices(CompVCameraDeviceInfoList& list) /* Overrides(CompVCamera) */
{
	CompVAutoLock<CompVDSCamera>(this);
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "devices");
    list.clear();
    CompVDSCameraDeviceInfoList list_;
    COMPV_CHECK_CODE_RETURN(CompVDSUtils:: enumerateCaptureDevices(list_));
    for (CompVDSCameraDeviceInfoList::iterator it = list_.begin(); it != list_.end(); ++it) {
        list.push_back(CompVCameraDeviceInfo(it->id, it->name, it->description));
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVDSCamera::start(const std::string& deviceId COMPV_DEFAULT("")) /* Overrides(CompVCamera) */
{
	CompVAutoLock<CompVDSCamera>(this);
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "start");
    COMPV_CHECK_CODE_RETURN(m_pGrabber->start(deviceId, m_Caps));
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVDSCamera::stop() /* Overrides(CompVCamera) */
{
	CompVAutoLock<CompVDSCamera>(this);
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "stop");
    COMPV_CHECK_CODE_RETURN(m_pGrabber->stop());
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVDSCamera::set(int id, const void* valuePtr, size_t valueSize) /* Overrides(CompVCaps) */
{
	CompVAutoLock<CompVDSCamera>(this);
	COMPV_CHECK_EXP_RETURN(!valuePtr || !valueSize, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "%s(%d, %p, %zu)", __FUNCTION__, id, valuePtr, valueSize);
	switch (id) {
		case COMPV_CAMERA_CAP_INT_WIDTH: {
			COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
			m_Caps.width = *reinterpret_cast<const int*>(valuePtr);
			return COMPV_ERROR_CODE_S_OK;
		}
		case COMPV_CAMERA_CAP_INT_HEIGHT: {
			COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
			m_Caps.height = *reinterpret_cast<const int*>(valuePtr);
			return COMPV_ERROR_CODE_S_OK;
		}
		case COMPV_CAMERA_CAP_INT_FPS: {
			COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
			m_Caps.fps = *reinterpret_cast<const int*>(valuePtr);
			return COMPV_ERROR_CODE_S_OK;
		}
		case COMPV_CAMERA_CAP_INT_SUBTYPE: {
			COMPV_CHECK_CODE_RETURN(CompVDSUtils::convertSubType(static_cast<COMPV_SUBTYPE>(*reinterpret_cast<const int*>(valuePtr)), m_Caps.subType));
			return COMPV_ERROR_CODE_S_OK;
		}
		default: {
			COMPV_DEBUG_ERROR("DirectShow camera implementation doesn't support capability id %d", id);
			return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
		}
	}
}

COMPV_ERROR_CODE CompVDSCamera::get(int id, const void*& valuePtr, size_t valueSize) /* Overrides(CompVCaps) */
{
	CompVAutoLock<CompVDSCamera>(this);
	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVDSCamera::newObj(CompVDSCameraPtrPtr camera)
{
    COMPV_CHECK_EXP_RETURN(!camera, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVDSCameraPtr camera_ = new CompVDSCamera();
    COMPV_CHECK_EXP_RETURN(!camera_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    camera_->m_pGrabber = new CompVDSGrabber(CompVDSCamera::DSBufferCB, *camera_);
    COMPV_CHECK_EXP_RETURN(!camera_->m_pGrabber, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    camera_->m_pGrabber->AddRef();

    *camera = *camera_;
    return COMPV_ERROR_CODE_S_OK;
}

HRESULT STDMETHODCALLTYPE CompVDSCamera::DSBufferCB(const CompVMatPtr image, const void *pcUserData)
{
	CompVDSCameraPtr camera = const_cast<CompVDSCamera*>(static_cast<const CompVDSCamera*>(pcUserData));
	CompVAutoLock<CompVDSCamera> autoLock(*camera);
	CompVCameraListenerPtr listener = camera->listener();
	if (!listener) {
		return S_OK;
	}
	COMPV_ERROR_CODE err;
	COMPV_CHECK_CODE_BAIL(err = listener->onNewFrame(image));
bail:
	return COMPV_ERROR_CODE_IS_NOK(err) ? E_FAIL : S_OK;
}

COMPV_NAMESPACE_END()