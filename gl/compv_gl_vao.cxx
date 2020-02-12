/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/gl/compv_gl_vao.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)

#define COMPV_THIS_CLASSNAME	"CompVGLVAO"

COMPV_NAMESPACE_BEGIN()

bool CompVGLVAO::s_bInitialized = false;

#if COMPV_OS_ANDROID && (defined(GL_ES_VERSION_2_0) && GL_ES_VERSION_2_0) && !defined(GL_ES_VERSION_3_0)
CompVSharedLibPtr CompVGLVAO::s_ptrOpenGLES = NULL;
PFNGLBINDVERTEXARRAYOESPROC CompVGLVAO::bindVertexArrayOES = NULL;
PFNGLDELETEVERTEXARRAYSOESPROC CompVGLVAO::deleteVertexArraysOES = NULL;
PFNGLGENVERTEXARRAYSOESPROC CompVGLVAO::genVertexArraysOES = NULL;
PFNGLISVERTEXARRAYOESPROC CompVGLVAO::isVertexArrayOES = NULL;
static const struct libGLES{
	const char* name_lib;
	const char* name_glBindVertexArray;
	const char* name_glDeleteVertexArrays;
	const char* name_glGenVertexArrays;
	const char* name_glIsVertexArray;
}
libGLES_values[] = {
#if 0 // Hum, mixing GLESv3 / GLESv2 ?
	{ "libGLESv3.so", "glBindVertexArray", "glDeleteVertexArrays", "glGenVertexArrays", "glIsVertexArray" },
#endif
	{ "libGLESv2.so", "glBindVertexArrayOES", "glDeleteVertexArraysOES", "glGenVertexArraysOES", "glIsVertexArrayOES" },
};
#endif

COMPV_ERROR_CODE CompVGLVAO::init()
{
    if (CompVGLVAO::s_bInitialized) {
        return COMPV_ERROR_CODE_S_OK;
    }

    COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;

    // Android with OpenGL-ES 2.0 API
#if COMPV_OS_ANDROID && defined(GL_ES_VERSION_2_0) && !defined(GL_ES_VERSION_3_0)
	for (size_t i = 0; i < sizeof(libGLES_values) / sizeof(libGLES_values[0]); ++i) {
		if (COMPV_ERROR_CODE_IS_OK(CompVSharedLib::newObj(&CompVGLVAO::s_ptrOpenGLES, libGLES_values[i].name_lib))) {
			CompVGLVAO::bindVertexArrayOES = reinterpret_cast<PFNGLBINDVERTEXARRAYOESPROC>(s_ptrOpenGLES->sym(libGLES_values[i].name_glBindVertexArray));
			CompVGLVAO::deleteVertexArraysOES = reinterpret_cast<PFNGLDELETEVERTEXARRAYSOESPROC>(s_ptrOpenGLES->sym(libGLES_values[i].name_glDeleteVertexArrays));
			CompVGLVAO::genVertexArraysOES = reinterpret_cast<PFNGLGENVERTEXARRAYSOESPROC>(s_ptrOpenGLES->sym(libGLES_values[i].name_glGenVertexArrays));
			CompVGLVAO::isVertexArrayOES = reinterpret_cast<PFNGLISVERTEXARRAYOESPROC>(s_ptrOpenGLES->sym(libGLES_values[i].name_glIsVertexArray));
			COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "'%s' file loaded: bindVertexArrayOES=%s, deleteVertexArraysOES=%s, genVertexArraysOES=%s, isVertexArrayOES=%s",
				libGLES_values[i].name_lib,
				CompVGLVAO::bindVertexArrayOES ? "YES" : "NO", 
				CompVGLVAO::deleteVertexArraysOES ? "YES" : "NO", 
				CompVGLVAO::genVertexArraysOES ? "YES" : "NO", 
				CompVGLVAO::isVertexArrayOES ? "YES" : "NO");
			if (CompVGLVAO::bindVertexArrayOES && CompVGLVAO::deleteVertexArraysOES && CompVGLVAO::genVertexArraysOES && CompVGLVAO::isVertexArrayOES) {
				break;
			}
		}
		else {
			COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "'%s' file *not* loaded", libGLES_values[i].name_lib);
		}
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
    CompVGLVAO::s_ptrOpenGLES = NULL;
    CompVGLVAO::bindVertexArrayOES = NULL;
    CompVGLVAO::deleteVertexArraysOES = NULL;
    CompVGLVAO::genVertexArraysOES = NULL;
    CompVGLVAO::isVertexArrayOES = NULL;
#endif

    CompVGLVAO::s_bInitialized = false;

    return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

