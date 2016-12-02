/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/ds/compv_ds_camera.h"
#include "compv/ds/compv_ds_utils.h"

COMPV_NAMESPACE_BEGIN()

CompVDSCamera::CompVDSCamera()
	: CompVCamera()
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
	COMPV_CHECK_CODE_RETURN(m_pGrabber->start(deviceId));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVDSCamera::stop() /* Overrides(CompVCamera) */
{
	COMPV_CHECK_CODE_RETURN(m_pGrabber->stop());
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVDSCamera::newObj(CompVDSCameraPtrPtr camera)
{
	COMPV_CHECK_EXP_RETURN(!camera, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVDSCameraPtr camera_ = new CompVDSCamera();
	COMPV_CHECK_EXP_RETURN(!camera_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	camera_->m_pGrabber = new CompVDSGrabber();
	COMPV_CHECK_EXP_RETURN(!camera_->m_pGrabber, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	camera_->m_pGrabber->AddRef();

	*camera = *camera_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()