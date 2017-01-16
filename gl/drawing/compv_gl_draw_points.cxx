/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/gl/drawing/compv_gl_draw_points.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/gl/compv_gl_utils.h"
#include "compv/gl/compv_gl_info.h"
#include "compv/gl/compv_gl_func.h"

COMPV_NAMESPACE_BEGIN()

static const std::string& kProgramVertexData =
"	attribute vec4 position;"
"	uniform vec2 TextureSize;"
"	void main() {"
"		gl_PointSize = 5.0;"
"		gl_Position = vec4(position.x/TextureSize.x, position.y/TextureSize.y, 1.0, 1.0);"
"	}";

static const std::string& kProgramFragmentData =
"	void main() {"
"		gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);"
"	}";

CompVGLDrawPoints::CompVGLDrawPoints()
	: CompVGLDraw(kProgramVertexData, kProgramFragmentData)
{

}

CompVGLDrawPoints::~CompVGLDrawPoints()
{

}

COMPV_ERROR_CODE CompVGLDrawPoints::process(const GLfloat* xy, GLsizei count)
{
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	GLuint uNamePosition, uNameLocation;
	GLint fboWidth = 0, fboHeight = 0;
	
	// Bind to VAO, VBO, Program
	COMPV_CHECK_CODE_BAIL(err = CompVGLDraw::bind());

	// Get FBO width
	COMPV_glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &fboWidth);
	COMPV_glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &fboHeight);
	COMPV_CHECK_EXP_BAIL(!fboWidth || !fboHeight, (err = COMPV_ERROR_CODE_E_GL), "fboWidth or fboHeight is equal to zero");

	// Submit vertices data
	COMPV_glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2 * count, xy, GL_STATIC_DRAW);

	// Set TextureSize (FIXME: do only if textureSize change for VAO)
	const GLfloat TextureSize[2] = { static_cast<GLfloat>(fboWidth), static_cast<GLfloat>(fboHeight) };
	uNameLocation = COMPV_glGetUniformLocation(program()->name(), "TextureSize");
	COMPV_glUniform2fv(uNameLocation, 1, TextureSize);

	// Set position attribute (FIXME: do onces if VAO supported)
	uNamePosition = COMPV_glGetAttribLocation(program()->name(), "position");
	COMPV_glEnableVertexAttribArray(uNamePosition);
	COMPV_glVertexAttribPointer(uNamePosition, 2, GL_FLOAT, GL_FALSE, 0, 0);

	// Draw points
	COMPV_glDrawArrays(GL_POINTS, 0, count);

bail:
	COMPV_CHECK_CODE_NOP(CompVGLDraw::unbind());
	return err;
}

COMPV_ERROR_CODE CompVGLDrawPoints::newObj(CompVGLDrawPointsPtrPtr drawPoints)
{
	COMPV_CHECK_EXP_RETURN(!drawPoints, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVGLDrawPointsPtr drawPoints_ = new CompVGLDrawPoints();
	COMPV_CHECK_EXP_RETURN(!drawPoints_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

	*drawPoints = drawPoints_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

