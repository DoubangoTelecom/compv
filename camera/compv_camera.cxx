/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/camera/compv_camera.h"
#include "compv/base/compv_fileutils.h"
#include "compv/base/compv_debug.h"
#include "compv/base/compv_base.h"

#include "compv/camera/android/compv_camera_android.h"
#include "compv/base/android/compv_android_dexclassloader.h"
#include "compv/base/android/compv_android_native_activity.h"

COMPV_NAMESPACE_BEGIN()

bool CompVCamera::s_bInitialized = false;
CompVCamera::CameraFactory CompVCamera::s_CameraFactory = { NULL, NULL };

CompVCamera::CompVCamera()
{

}

CompVCamera::~CompVCamera()
{

}

COMPV_ERROR_CODE CompVCamera::setListener(CompVCameraListenerPtr listener)
{
	m_ptrListener = listener;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVCamera::init()
{
    if (s_bInitialized) {
        return COMPV_ERROR_CODE_S_OK;
    }
    COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;

    COMPV_DEBUG_INFO("Initializing [camera] module (v %s)...", COMPV_VERSION_STRING);

    COMPV_CHECK_CODE_BAIL(err = CompVBase::init());

#if COMPV_OS_WINDOWS
    if (!s_CameraFactory.isValid() && CompVBase::isWin7OrLater()) {
		COMPV_CHECK_CODE_NOP(err = CompVSharedLib::newObj(&s_CameraFactory.lib, "CompVPluginMFoundation.dll"));
		if (s_CameraFactory.lib) {
			s_CameraFactory.funcNew = reinterpret_cast<CompVCameraNewFunc>(s_CameraFactory.lib->sym("newObjCamera"));
			COMPV_CHECK_EXP_NOP(!s_CameraFactory.funcNew, COMPV_ERROR_CODE_E_SYSTEM);
		}
    }
    if (!s_CameraFactory.isValid() && CompVBase::isWinXPOrLater()) {
        COMPV_CHECK_CODE_NOP(err = CompVSharedLib::newObj(&s_CameraFactory.lib, "CompVPluginDirectShow.dll"));
        if (s_CameraFactory.lib) {
            s_CameraFactory.funcNew = reinterpret_cast<CompVCameraNewFunc>(s_CameraFactory.lib->sym("newObjCamera"));
            COMPV_CHECK_EXP_NOP(!s_CameraFactory.funcNew, COMPV_ERROR_CODE_E_SYSTEM);
        }
    }
#endif

    s_bInitialized = true;

bail:
    if (COMPV_ERROR_CODE_IS_NOK(err)) {
        COMPV_CHECK_CODE_NOP(deInit());
    }
    return err;
}

COMPV_ERROR_CODE CompVCamera::deInit()
{
    COMPV_CHECK_CODE_NOP(CompVBase::deInit());

    s_CameraFactory.deinit();

    s_bInitialized = false;

    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVCamera::newObj(CompVCameraPtrPtr camera)
{
    COMPV_CHECK_EXP_RETURN(!camera, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    COMPV_CHECK_CODE_RETURN(init());
    CompVCameraPtr camera_;

    // Create camera using the factory (dynamic plugins)
    if (s_CameraFactory.isValid()) {
        COMPV_CHECK_CODE_RETURN(s_CameraFactory.funcNew(&camera_));
    }

    // Create camera using static implementations
    if (!camera_) {
#if COMPV_OS_ANDROID
		CompVCameraAndroidPtr cameraAndroid;
		COMPV_CHECK_CODE_RETURN(CompVCameraAndroid::newObj(&cameraAndroid));
		camera_ = *cameraAndroid;
#endif
    }

    COMPV_CHECK_EXP_RETURN(!camera_, COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);

    *camera = camera_;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
