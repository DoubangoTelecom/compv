/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_WINDOW_REGISTRY_H_)
#define _COMPV_BASE_WINDOW_REGISTRY_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/parallel/compv_mutex.h"
#include "compv/base/drawing/compv_window.h"

#include <map>

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class COMPV_BASE_API CompVWindowRegistry
{
    friend class CompVWindow;
public:
    static COMPV_ERROR_CODE init();
    static COMPV_ERROR_CODE deInit();
    static size_t count();
    static COMPV_ERROR_CODE closeAll();
#if COMPV_OS_ANDROID
	static COMPV_ERROR_CODE android_handle_cmd(int cmd);
#endif

private:
    static COMPV_ERROR_CODE add(CompVWindowPrivPtr window);
    static COMPV_ERROR_CODE remove(CompVWindowPrivPtr window);
    static COMPV_ERROR_CODE remove(compv_window_id_t windowId);

private:
    COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
    static std::map<compv_window_id_t, CompVWindowPrivPtr > m_sWindows;
    static CompVMutexPtr s_WindowsMutex;
    static bool s_bInitialized;
    COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_WINDOW_REGISTRY_H_ */
