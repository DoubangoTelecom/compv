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

static const std::string& kProgramVertexData =
#	if defined(HAVE_OPENGLES)
"	precision mediump float;"
#	endif
"	attribute vec2 position;"
"	attribute vec4 color;"
"	varying vec4 colorVarying;"
"	uniform mat4 MVP;"
"	uniform float pointSize;"
"	void main() {"
"		gl_PointSize = pointSize;"
"		gl_Position = MVP * vec4(position, 1.0, 1.0);"
"		colorVarying = color;"
"	}";

static const std::string& kProgramFragmentData =
#	if defined(HAVE_OPENGLES)
"	precision mediump float;"
#	endif
"	varying vec4 colorVarying;"
"	void main() {"
"		gl_FragColor = colorVarying;"
"	}";

#define kVertexDataWithMVP_Yes	true
#define kVertexDataWithMVP_No	false

COMPV_NAMESPACE_BEGIN()

CompVGLDrawPoints::CompVGLDrawPoints()
	: CompVGLDraw(kProgramVertexData, kProgramFragmentData, kVertexDataWithMVP_Yes)
	, m_fboWidth(0)
	, m_fboHeight(0)
{

}

CompVGLDrawPoints::~CompVGLDrawPoints()
{

}

COMPV_ERROR_CODE CompVGLDrawPoints::points(const CompVGLPoint2D* points, GLsizei count, const CompVDrawingOptions* options COMPV_DEFAULT(nullptr))
{
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	GLint fboWidth = 0, fboHeight = 0;
	bool bFirstTimeOrChanged;
	const GLfloat pointSize = options ? static_cast<GLfloat>(options->pointSize) : 7.f;
	GLuint uName;

	// Bind to VAO, VBO, Program
	COMPV_CHECK_CODE_BAIL(err = CompVGLDraw::bind());

	// Get FBO width
	COMPV_glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &fboWidth);
	COMPV_glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &fboHeight);
	COMPV_CHECK_EXP_BAIL(!fboWidth || !fboHeight, (err = COMPV_ERROR_CODE_E_GL), "fboWidth or fboHeight is equal to zero");
	bFirstTimeOrChanged = (m_fboWidth != fboWidth || m_fboHeight != fboHeight);
	
	if (!CompVGLInfo::extensions::vertex_array_object() || bFirstTimeOrChanged) {
		// Set position attribute
		GLuint uNamePosition = COMPV_glGetAttribLocation(program()->name(), "position");
		COMPV_glEnableVertexAttribArray(uNamePosition);
		COMPV_glVertexAttribPointer(uNamePosition, 2, GL_FLOAT, GL_FALSE, sizeof(CompVGLPoint2D), reinterpret_cast<const GLvoid *>(offsetof(CompVGLPoint2D, position)));

		// Set color attribute
		GLuint uNameColor = COMPV_glGetAttribLocation(program()->name(), "color");
		COMPV_glEnableVertexAttribArray(uNameColor);
		COMPV_glVertexAttribPointer(uNameColor, 4, GL_FLOAT, GL_FALSE, sizeof(CompVGLPoint2D), reinterpret_cast<const GLvoid *>(offsetof(CompVGLPoint2D, color)));

		// Set projection
		COMPV_CHECK_CODE_BAIL(err = CompVGLDraw::setOrtho(0, static_cast<GLfloat>(fboWidth), static_cast<GLfloat>(fboHeight), 0, -1, 1));
	}

	// Set PointSize
	uName = COMPV_glGetUniformLocation(program()->name(), "pointSize");
	COMPV_glUniform1f(uName, pointSize);

	// Submit vertices data
	COMPV_glBufferData(GL_ARRAY_BUFFER, sizeof(CompVGLPoint2D) * count, points, GL_STATIC_DRAW);

	// Draw points
	COMPV_glViewport(0, 0, static_cast<GLsizei>(fboWidth), static_cast<GLsizei>(fboHeight));
	COMPV_glDrawArrays(GL_POINTS, 0, count);

	// Update size
	m_fboWidth = fboWidth;
	m_fboHeight = fboHeight;

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

