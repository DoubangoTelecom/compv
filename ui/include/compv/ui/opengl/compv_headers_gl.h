/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_UI_OPENGL_HEADERS_GL_H_)
#define _COMPV_UI_OPENGL_HEADERS_GL_H_

#include "compv/base/compv_config.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

#if defined(HAVE_GL_GLEW_H)
#	include <GL/glew.h>
#endif

#if COMPV_OS_WINDOWS || (defined(HAVE_GL_GL_H) && defined(HAVE_GL_GLU_H))
#	include <GL/GL.h>
#	include <GL/GLU.h>
#elif TARGET_OS_MAC || (defined(HAVE_OPENGL_GL_H) && defined(HAVE_OPENGL_GLU_H))
#	include <OpenGL/gl.h>
#	include <OpenGL/glu.h>
#elif COMPV_OS_LINUX || (defined(HAVE_GL_GL_H) && defined(HAVE_GL_GLU_H))
#	include <GL/gl.h>
#	include <GL/glu.h>
#endif

#endif /* _COMPV_UI_OPENGL_HEADERS_GL_H_ */
