/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/mf/compv_mf_config.h"
#include "compv/mf/compv_mf_camera.h"
#include "compv/mf/compv_mf_utils.h"

#if !COMPV_OS_WINDOWS_CE
BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
#endif

COMPV_NAMESPACE_BEGIN()

extern "C" COMPV_PLUGIN_MFOUNDATION_API COMPV_ERROR_CODE newObjCamera(CompVCameraPtrPtr camera)
{
	COMPV_CHECK_EXP_RETURN(!camera, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_EXP_RETURN(FAILED(CompVMFUtils::startup()), COMPV_ERROR_CODE_E_MFOUNDATION, "Failed to start Microsoft Media Foundation engine");

	CompVMFCameraPtr camera_;
	COMPV_CHECK_CODE_RETURN(CompVMFCamera::newObj(&camera_));

	*camera = *camera_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
