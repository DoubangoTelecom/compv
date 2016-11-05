/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_DRAWING_OPENGL_CONSTS_GL_H_)
#define _COMPV_DRAWING_OPENGL_CONSTS_GL_H_

#include "compv/base/compv_config.h"
#include "compv/drawing/opengl/compv_headers_gl.h"
#if defined(HAVE_OPENGL) ||defined(HAVE_OPENGLES)
#include "compv/drawing/compv_program.h"
#include "compv/base/compv_obj.h"
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

typedef struct {
	GLfloat Position[3];
	GLfloat TexCoord[2];
} CompVGLVertex;

static const CompVGLVertex CompVGLTexture2DVertices[] = {
	{ { 1.f, -1.f, 0.f }, { 1.f, 1.f } },
	{ { 1.f, 1.f, 0.f }, { 1.f, 0.f } },
	{ { -1.f, 1.f, 0.f }, { 0.f, 0.f } },
	{ { -1.f, -1.f, 0.f }, { 0.f, 1.f } }
};

static const CompVGLVertex CompVGLScreenVertices[] = {
	{ { 1.f, -1.f, 0.f }, { 1.f,  0.f } },
	{ { 1.f, 1.f, 0.f }, { 1.f, 1.f } },
	{ { -1.f, 1.f, 0.f }, { 0.f, 1.f } },
	{ { -1.f, -1.f, 0.f }, { 0.f, 0.f } }
};

static const GLubyte CompVGLTexture2DIndices[] = {
	0, 1, 2, // 1st triangle
	2, 3, 0 // 2nd triangle
};

static const GLsizei CompVGLTexture2DIndicesCount = sizeof(CompVGLTexture2DIndices) / sizeof(CompVGLTexture2DIndices[0]);

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) ||defined(HAVE_OPENGLES) */

#endif /* _COMPV_DRAWING_OPENGL_CONSTS_GL_H_ */
