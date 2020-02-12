/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#ifndef _COMPV_GL_CONFIG_H_
#define _COMPV_GL_CONFIG_H_

#include "compv/base/compv_config.h"

// Windows's symbols export
#if defined(COMPV_STATIC)
# 		define COMPV_GL_API
#elif COMPV_OS_WINDOWS
#	if defined(COMPV_GL_EXPORTS)
# 		define COMPV_GL_API		__declspec(dllexport)
#	else
# 		define COMPV_GL_API		__declspec(dllimport)
#	endif
#else
#	define COMPV_GL_API			__attribute__((visibility("default")))
#endif

#if !defined(COMPV_GL_SWAP_INTERVAL)
#	define COMPV_GL_SWAP_INTERVAL 0
#endif

#if !defined(COMPV_GL_EGL_CONTEXT_CLIENT_VERSION)
#	define COMPV_GL_EGL_CONTEXT_CLIENT_VERSION 2
#endif

#if !defined(COMPV_GL_DEBUG) && (defined(DEBUG) || defined(_DEBUG))
#	define COMPV_GL_DEBUG 1
#endif

#endif /* _COMPV_GL_CONFIG_H_ */
