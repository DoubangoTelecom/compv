/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_GL_H_)
#define _COMPV_GL_H_

#include "compv/gl/compv_gl_config.h"
#include "compv/base/compv_sharedlib.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_GL_API CompVGL
{
public:
    static COMPV_ERROR_CODE init();
    static COMPV_ERROR_CODE deInit();
#	if defined(HAVE_GL_GLEW_H)
    static COMPV_ERROR_CODE glewInit();
#endif
	static COMPV_INLINE bool isInitialized() {
		return s_bInitialized;
	}

private:
    static bool s_bInitialized;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_GL_H_ */
