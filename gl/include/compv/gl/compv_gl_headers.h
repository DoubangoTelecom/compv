/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_GL_HEADERS_H_)
#define _COMPV_GL_HEADERS_H_

#include "compv/gl/compv_gl_config.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

/* GLEW */
#if defined(HAVE_GL_GLEW_H)
#	include <GL/glew.h>
#endif

/* OpenGL-ES 2 */
#define GL_GLEXT_PROTOTYPES

#if COMPV_OS_ANDROID
#	define HAVE_OPENGLES	1
#	if __ANDROID_API__ >= 18 && 0 // TODO(dmi): for now use OpenGL-ES2 on Android
#		include <GLES3/gl3.h>
#		include <GLES3/gl3ext.h>
#	elif __ANDROID_API__ >= 8
#		include <GLES2/gl2.h>
#		include <GLES2/gl2ext.h>
#		define glGenVertexArraysOES     CompVGLVAO::genVertexArraysOES
#		define glBindVertexArrayOES		CompVGLVAO::bindVertexArrayOES
#		define glDeleteVertexArraysOES  CompVGLVAO::deleteVertexArraysOES
#	else
#		include <GLES/gl.h>
#		include <GLES/glext.h>
#	endif
#endif
#if !defined(HAVE_OPENGLES) && (defined(HAVE_GLSES2_GL2_H) && defined(HAVE_GLSES2_GL2EXT_H))
#	define HAVE_OPENGLES	1
#	include <GLES2/gl2.h>
#	include <GLES2/gl2ext.h>
#endif

#if COMPV_OS_IPHONE || COMPV_OS_IPHONE_SIMULATOR || (defined(HAVE_OPENGLES_ES2_GL_H) && defined(HAVE_OPENGLES_ES2_GLEXT_H))
#	define HAVE_OPENGLES	1
#	include <OpenGLES/ES2/gl.h>
#	include <OpenGLES/ES2/glext.h>
#endif

#if defined(GL_ES_VERSION_2_0) && !defined(GL_ES_VERSION_3_0)
#	define glGenVertexArrays     glGenVertexArraysOES
#	define glBindVertexArray	 glBindVertexArrayOES
#	define glDeleteVertexArrays  glDeleteVertexArraysOES
#endif

/* OpenGL */
#if COMPV_OS_WINDOWS || (defined(HAVE_GL_GL_H) && defined(HAVE_GL_GLU_H))
#	define HAVE_OPENGL	1
#	include <GL/GL.h>
#	include <GL/GLU.h>
#elif (COMPV_OS_OSX && !COMPV_OS_IPHONE) || (defined(HAVE_OPENGL_GL_H) && defined(HAVE_OPENGL_GLU_H)) // TARGET_OS_MAC is 1 on iOS
#	define HAVE_OPENGL	1
#	include <OpenGL/gl.h>
#	include <OpenGL/glu.h>
#elif (COMPV_OS_LINUX && !COMPV_OS_ANDROID) || (defined(HAVE_GL_GL_H) && defined(HAVE_GL_GLU_H))
#	define HAVE_OPENGL	1
#	include <GL/gl.h>
#	include <GL/glu.h>
#endif

/* EGL */
#if COMPV_OS_ANDROID || (defined(HAVE_EGL_EGL_H) && defined(HAVE_EGL_EGLEXT_H))
#	define HAVE_EGL		1
#	include <EGL/egl.h>
#	include <EGL/eglext.h>
#endif

/* EAGL (objc-code) */
//#if COMPV_OS_IPHONE || defined(HAVE_OPENGLES_EAGL_H)
//#  include <OpenGLES/EAGL.h>
//#endif

#endif /* _COMPV_GL_HEADERS_H_ */
