/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/ds/compv_ds_config.h"
#include "compv/ds/compv_ds_camera.h"

#if !COMPV_OS_WINDOWS_CE && COMPV_OS_WINDOWS
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

COMPV_EXTERNC COMPV_PLUGIN_DIRECTSHOW_API COMPV_ERROR_CODE newObjCamera(CompVCameraPtrPtr camera)
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    COMPV_CHECK_EXP_RETURN(FAILED(hr) && hr != RPC_E_CHANGED_MODE, COMPV_ERROR_CODE_E_DIRECTSHOW);
    COMPV_CHECK_EXP_RETURN(!camera, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVDSCameraPtr camera_;
    COMPV_CHECK_CODE_RETURN(CompVDSCamera::newObj(&camera_));

    *camera = *camera_;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
