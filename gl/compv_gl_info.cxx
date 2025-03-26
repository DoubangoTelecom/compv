/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/gl/compv_gl_info.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/base/compv_debug.h"
#include "compv/gl/compv_gl_vao.h"
#include "compv/gl/compv_gl_utils.h"

COMPV_NAMESPACE_BEGIN()

bool CompVGLInfo::s_bGathered = false;
GLint CompVGLInfo::s_iVersionMajor = 0;
GLint CompVGLInfo::s_iVersionMinor = 0;
GLint CompVGLInfo::s_iMaxColorAttachments = 0;
GLint CompVGLInfo::s_iMaxDrawBuffers = 0;
GLint CompVGLInfo::s_iMaxtextureSize = 1024;

bool CompVGLInfo::extensions::s_bvertex_array_object = false;
bool CompVGLInfo::extensions::s_btexture_float = false;

#define THIS_CLASSNAME "CompVGLInfo"

COMPV_ERROR_CODE CompVGLInfo::gather()
{
    if (CompVGLInfo::s_bGathered) {
        return COMPV_ERROR_CODE_S_OK;
    }
    COMPV_DEBUG_INFO_EX(THIS_CLASSNAME, "Gathering GL info...");
    COMPV_CHECK_EXP_RETURN(!CompVGLUtils::isGLContextSet(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);

    const char* extensions = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
	const GLubyte* version = glGetString(GL_VERSION);
    COMPV_DEBUG_INFO_EX(THIS_CLASSNAME, "OpenGL version string: %s", version);
    COMPV_DEBUG_INFO_EX(THIS_CLASSNAME, "OpenGL shading version string: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
    COMPV_DEBUG_INFO_EX(THIS_CLASSNAME, "OpenGL renderer string: %s", glGetString(GL_RENDERER));
    COMPV_DEBUG_INFO_EX(THIS_CLASSNAME, "OpenGL vendor string: %s", glGetString(GL_VENDOR));
    COMPV_DEBUG_VERBOSE_EX(THIS_CLASSNAME, "OpenGL extensions string: %s", extensions);

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &s_iMaxtextureSize);
	COMPV_DEBUG_INFO_EX(THIS_CLASSNAME, "GL_MAX_TEXTURE_SIZE: %d", s_iMaxtextureSize);

#if defined(HAVE_OPENGL)
    glGetIntegerv(GL_MAJOR_VERSION, &s_iVersionMajor);
    glGetIntegerv(GL_MINOR_VERSION, &s_iVersionMinor);
    COMPV_DEBUG_INFO_EX(THIS_CLASSNAME, "OpenGL parsed major and minor versions: %d.%d", s_iVersionMajor, s_iVersionMinor);
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &s_iMaxColorAttachments);
    COMPV_DEBUG_INFO_EX(THIS_CLASSNAME, "GL_MAX_DRAW_BUFFERS: %d", s_iMaxColorAttachments);
    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &s_iMaxDrawBuffers);
    COMPV_DEBUG_INFO_EX(THIS_CLASSNAME, "GL_MAX_DRAW_BUFFERS: %d", s_iMaxDrawBuffers);
	COMPV_CHECK_EXP_NOP(s_iVersionMajor < 3, COMPV_ERROR_CODE_E_GL); // Do not exit but alert the user that OpenGL 3.0+ is required
#else
	COMPV_CHECK_EXP_NOP(sscanf(reinterpret_cast<const char*>(version), "OpenGL ES %d.%d ", &s_iVersionMajor, &s_iVersionMinor) == EOF, COMPV_ERROR_CODE_E_GL);
#endif
    if (extensions) {
        CompVGLInfo::extensions::s_bvertex_array_object = (strstr(extensions, "ARB_vertex_array_object") != NULL || strstr(extensions, "OES_vertex_array_object") != NULL);
#		if COMPV_OS_ANDROID && defined(GL_ES_VERSION_2_0) && !defined(GL_ES_VERSION_3_0)
        CompVGLInfo::extensions::s_bvertex_array_object &= CompVGLVAO::bindVertexArrayOES && CompVGLVAO::deleteVertexArraysOES && CompVGLVAO::genVertexArraysOES && CompVGLVAO::isVertexArrayOES;
#		endif
        CompVGLInfo::extensions::s_btexture_float = (strstr(extensions, "ARB_texture_float") != NULL || strstr(extensions, "OES_texture_float") != NULL);
    }
    CompVGLInfo::s_bGathered = true;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */
