/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#ifndef _COMPV_PLUGIN_MFOUNDATION_CONFIG_H_
#define _COMPV_PLUGIN_MFOUNDATION_CONFIG_H_

#include "compv/camera/compv_camera_config.h"

// Windows's symbols export
#if COMPV_OS_WINDOWS
#	if defined(COMPV_PLUGIN_MFOUNDATION_EXPORTS)
# 		define COMPV_PLUGIN_MFOUNDATION_API		__declspec(dllexport)
#	else
# 		define COMPV_PLUGIN_MFOUNDATION_API		__declspec(dllimport)
#	endif
#else
#	define COMPV_PLUGIN_MFOUNDATION_API
#endif

#endif /* _COMPV_PLUGIN_MFOUNDATION_CONFIG_H_ */
