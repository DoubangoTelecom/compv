/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#ifndef _COMPV_GL_CONFIG_H_
#define _COMPV_GL_CONFIG_H_

#include "compv/base/compv_config.h"

// Windows's symbols export
#if COMPV_OS_WINDOWS
#	if defined(COMPV_GL_EXPORTS)
# 		define COMPV_GL_API		__declspec(dllexport)
#	else
# 		define COMPV_GL_API		__declspec(dllimport)
#	endif
#else
#	define COMPV_GL_API
#endif

#endif /* _COMPV_GL_CONFIG_H_ */
