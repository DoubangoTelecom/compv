/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/gl/compv_gl_vao.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)

COMPV_NAMESPACE_BEGIN()

bool CompVGLVAO::s_bInitialized = false;

#if COMPV_OS_ANDROID && (defined(GL_ES_VERSION_2_0) && GL_ES_VERSION_2_0) && !defined(GL_ES_VERSION_3_0)
CompVSharedLibPtr CompVGLVAO::s_ptrGLESv2 = NULL;
PFNGLBINDVERTEXARRAYOESPROC CompVGLVAO::bindVertexArrayOES = NULL;
PFNGLDELETEVERTEXARRAYSOESPROC CompVGLVAO::deleteVertexArraysOES = NULL;
PFNGLGENVERTEXARRAYSOESPROC CompVGLVAO::genVertexArraysOES = NULL;
PFNGLISVERTEXARRAYOESPROC CompVGLVAO::isVertexArrayOES = NULL;
#endif

COMPV_ERROR_CODE CompVGLVAO::init()
{
	if (CompVGLVAO::s_bInitialized) {
		return COMPV_ERROR_CODE_S_OK;
	}

	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;

	// Android with OpenGL-ES 2.0 API
#if COMPV_OS_ANDROID && defined(GL_ES_VERSION_2_0) && !defined(GL_ES_VERSION_3_0)
	if (COMPV_ERROR_CODE_IS_OK(CompVSharedLib::newObj(&CompVGLVAO::s_ptrGLESv2, "libGLESv2.so"))) {
		CompVGLVAO::bindVertexArrayOES = reinterpret_cast<PFNGLBINDVERTEXARRAYOESPROC>(s_ptrGLESv2->sym("glBindVertexArrayOES"));
		CompVGLVAO::deleteVertexArraysOES = reinterpret_cast<PFNGLDELETEVERTEXARRAYSOESPROC>(s_ptrGLESv2->sym("glDeleteVertexArraysOES"));
		CompVGLVAO::genVertexArraysOES = reinterpret_cast<PFNGLGENVERTEXARRAYSOESPROC>(s_ptrGLESv2->sym("glGenVertexArraysOES"));
		CompVGLVAO::isVertexArrayOES = reinterpret_cast<PFNGLISVERTEXARRAYOESPROC>(s_ptrGLESv2->sym("glIsVertexArrayOES"));
		COMPV_DEBUG_INFO("Android with OpenGL-ES 2.0 API and 'libGLESv2.so' file loaded: bindVertexArrayOES=%s, deleteVertexArraysOES=%s, genVertexArraysOES=%s, isVertexArrayOES=%s",
			CompVGLVAO::bindVertexArrayOES ? "YES" : "NO", CompVGLVAO::deleteVertexArraysOES ? "YES" : "NO", CompVGLVAO::genVertexArraysOES ? "YES" : "NO", CompVGLVAO::isVertexArrayOES ? "YES" : "NO");
	}
	else {
		COMPV_DEBUG_WARN("Android with OpenGL-ES 2.0 API and 'libGLESv2.so' file *not* loaded");
	}
#endif

	COMPV_CHECK_CODE_BAIL(err);

	CompVGLVAO::s_bInitialized = true;

bail:
	if (COMPV_ERROR_CODE_IS_NOK(err)) {
		COMPV_CHECK_CODE_ASSERT(CompVGLVAO::deInit());
	}
	return err;
}

COMPV_ERROR_CODE CompVGLVAO::deInit()
{
#if COMPV_OS_ANDROID && defined(GL_ES_VERSION_2_0) && !defined(GL_ES_VERSION_3_0)
	CompVGLVAO::s_ptrGLESv2 = NULL;
	CompVGLVAO::bindVertexArrayOES = NULL;
	CompVGLVAO::deleteVertexArraysOES = NULL;
	CompVGLVAO::genVertexArraysOES = NULL;
	CompVGLVAO::isVertexArrayOES = NULL;
#endif

	CompVGLVAO::s_bInitialized = false;

	return COMPV_ERROR_CODE_S_OK;
}

bool CompVGLVAO::haveAPI()
{
#if COMPV_OS_ANDROID && defined(GL_ES_VERSION_2_0) && !defined(GL_ES_VERSION_3_0)
	return CompVGLVAO::bindVertexArrayOES &&
	CompVGLVAO::deleteVertexArraysOES &&
	CompVGLVAO::genVertexArraysOES &&
	CompVGLVAO::isVertexArrayOES;
#else
	return true; // iOS -> ok
#endif
}

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

