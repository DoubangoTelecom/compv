/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/gl/compv_gl_utils.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/base/compv_debug.h"

#define kModuleNameGLUtils "GLUtils"

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

COMPV_ERROR_CODE COMPVGLUtils::lastError(std::string *error)
{
	//!\ You must *not* call any "COMPV_gl*" function here as they recursively call this function. 
	// Doing so will lead to a stack overflow crash.
	// issue.
	GLenum err;
	COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
	char buff_[33] = { 0 };
	while ((err = glGetError()) != GL_NO_ERROR) {
		if (error) {
#if defined(HAVE_GL_GLU_H) || defined(HAVE_GLU_H) || defined(__glu_h__) || defined(__GLU_H__)
			const char* str = reinterpret_cast<const char*>(gluErrorString(err));
			if (str) {
				*error += "\n" + std::string(str);
			}
			else {
				snprintf(buff_, sizeof(buff_), "%d", static_cast<int>(err));
				*error += "\ncode:" + std::string(buff_);
				// err_ = COMPV_ERROR_CODE_E_GL;
			}
#else
			snprintf(buff_, sizeof(buff_), "%d", static_cast<int>(err));
			*error += "\n" + std::string(buff_);
#endif
		}
	}
	return err_;
}

COMPV_ERROR_CODE COMPVGLUtils::checkLastError()
{
	std::string errString_;
	COMPV_CHECK_CODE_RETURN(COMPVGLUtils::lastError(&errString_));
	if (!errString_.empty()) {
		COMPV_DEBUG_ERROR_EX(kModuleNameGLUtils, "OpenGL error: %s", errString_.c_str());
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_GL);
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */
