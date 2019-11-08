/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#ifndef _COMPV_CAMERA_CONFIG_H_
#define _COMPV_CAMERA_CONFIG_H_

#include "compv/base/compv_config.h"

// Windows's symbols export
#if defined(COMPV_STATIC)
# 		define COMPV_CAMERA_API
#elif COMPV_OS_WINDOWS
#	if defined(COMPV_CAMERA_EXPORTS)
# 		define COMPV_CAMERA_API		__declspec(dllexport)
#	else
# 		define COMPV_CAMERA_API		__declspec(dllimport)
#	endif
#elif defined (COMPV_SYMBOLS_HIDDEN)
#	define COMPV_CAMERA_API			__attribute__((visibility("default")))
#else
#	define COMPV_CAMERA_API
#endif

#endif /* _COMPV_CAMERA_CONFIG_H_ */
