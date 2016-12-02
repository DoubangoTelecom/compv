/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
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

bool CompVGLInfo::extensions::s_bvertex_array_object = false;
bool CompVGLInfo::extensions::s_btexture_float = false;

COMPV_ERROR_CODE CompVGLInfo::gather()
{
    if (CompVGLInfo::s_bGathered) {
        return COMPV_ERROR_CODE_S_OK;
    }
    COMPV_DEBUG_INFO("Gathering GL info...");
    COMPV_CHECK_EXP_RETURN(!CompVGLUtils::isGLContextSet(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);

    const char* extensions = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
    COMPV_DEBUG_INFO("OpenGL version string: %s", glGetString(GL_VERSION));
    COMPV_DEBUG_INFO("OpenGL shading version string: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
    COMPV_DEBUG_INFO("OpenGL renderer string: %s", glGetString(GL_RENDERER));
    COMPV_DEBUG_INFO("OpenGL vendor string: %s", glGetString(GL_VENDOR));
    COMPV_DEBUG_INFO("OpenGL extensions string: %s", extensions);
#if defined(HAVE_OPENGL)
    glGetIntegerv(GL_MAJOR_VERSION, &s_iVersionMajor);
    glGetIntegerv(GL_MINOR_VERSION, &s_iVersionMinor);
    COMPV_DEBUG_INFO("OpenGL parsed major and minor versions: %d.%d", s_iVersionMajor, s_iVersionMinor);
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &s_iMaxColorAttachments);
    COMPV_DEBUG_INFO("GL_MAX_DRAW_BUFFERS: %d", s_iMaxColorAttachments);
    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &s_iMaxDrawBuffers);
    COMPV_DEBUG_INFO("GL_MAX_DRAW_BUFFERS: %d", s_iMaxDrawBuffers);
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
