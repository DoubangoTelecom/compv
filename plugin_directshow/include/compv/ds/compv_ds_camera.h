/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_PLUGIN_DIRECTSHOW_CAMERA_H_)
#define _COMPV_PLUGIN_DIRECTSHOW_CAMERA_H_

#include "compv/ds/compv_ds_config.h"
#include "compv/camera/compv_camera.h"
#include "compv/base/compv_obj.h"

COMPV_NAMESPACE_BEGIN()

COMPV_OBJECT_DECLARE_PTRS(CameraDS)

class COMPV_PLUGIN_DIRECTSHOW_API CompVCameraDS : public CompVCamera
{
protected:
	CompVCameraDS();
public:
	virtual ~CompVCameraDS();
	COMPV_OBJECT_GET_ID(CompVCameraDS);

	virtual COMPV_ERROR_CODE open(const std::string& name = "") override /* Overrides(CompVCamera) */;
	virtual COMPV_ERROR_CODE close() override /* Overrides(CompVCamera) */;

	static COMPV_ERROR_CODE newObj(CompVCameraDSPtrPtr camera);

private:
	
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_PLUGIN_DIRECTSHOW_CAMERA_H_ */
