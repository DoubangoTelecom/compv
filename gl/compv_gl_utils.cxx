/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/gl/compv_gl_utils.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)

COMPV_NAMESPACE_BEGIN()

CompVGLContext COMPVGLUtils::currentContext()
{
#if defined(HAVE_EGL)
	EGLContext ctx = eglGetCurrentContext();
	if (ctx == EGL_NO_CONTEXT) {
		return NULL;
	}
	return static_cast<CompVGLContext>(ctx);
#else
#	if COMPV_OS_WINDOWS
	return static_cast<CompVGLContext>(wglGetCurrentContext());
#	elif COMPV_OS_APPLE
	return static_cast<CompVGLContext>(aglGetCurrentContext());
#	else
	return static_cast<CompVGLContext>(glXGetCurrentContext());
#	endif
#endif
}

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */
