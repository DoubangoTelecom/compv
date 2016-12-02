/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_PLUGIN_DIRECTSHOW_CAMERA_H_)
#define _COMPV_PLUGIN_DIRECTSHOW_CAMERA_H_

#include "compv/ds/compv_ds_config.h"
#include "compv/ds/compv_ds_grabber.h"
#include "compv/camera/compv_camera.h"
#include "compv/base/compv_obj.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

COMPV_OBJECT_DECLARE_PTRS(DSCamera)

class CompVDSCamera : public CompVCamera
{
protected:
	CompVDSCamera();
public:
	virtual ~CompVDSCamera();
	COMPV_OBJECT_GET_ID(CompVDSCamera);

	virtual COMPV_ERROR_CODE devices(CompVCameraDeviceInfoList& list) override /* Overrides(CompVCamera) */;
	virtual COMPV_ERROR_CODE start(const std::string& deviceId = "") override /* Overrides(CompVCamera) */;
	virtual COMPV_ERROR_CODE stop() override /* Overrides(CompVCamera) */;

	static COMPV_ERROR_CODE newObj(CompVDSCameraPtrPtr camera);

private:
	CompVDSGrabber* m_pGrabber;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_PLUGIN_DIRECTSHOW_CAMERA_H_ */
