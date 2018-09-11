/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#ifndef _COMPV_GPU_CONFIG_H_
#define _COMPV_GPU_CONFIG_H_

#include "compv/base/compv_config.h"

// Windows's symbols export
#if COMPV_OS_WINDOWS && !defined(COMPV_GPU_STATIC)
#	if defined(COMPV_GPU_EXPORTS)
# 		define COMPV_GPU_API		__declspec(dllexport)
#	else
# 		define COMPV_GPU_API		__declspec(dllimport)
#	endif
#else
#	define COMPV_GPU_API
#endif

#endif /* _COMPV_GPU_CONFIG_H_ */
