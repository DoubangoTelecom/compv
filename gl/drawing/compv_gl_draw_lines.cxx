/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/gl/drawing/compv_gl_draw_lines.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/gl/compv_gl_utils.h"
#include "compv/gl/compv_gl_info.h"
#include "compv/gl/compv_gl_func.h"

static const std::string& kProgramVertexData =
#	if defined(HAVE_OPENGLES)
"	precision mediump float;"
#	endif
"	attribute vec2 position;"
"	attribute vec3 color;"
"	varying vec4 colorVarying;"
"	uniform mat4 MVP;"
"	void main() {"
"		gl_PointSize = 7.0;"
"		gl_Position = MVP * vec4(position, 1.0, 1.0);"
"		colorVarying = vec4(color, 1.0);"
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

CompVGLDrawLines::CompVGLDrawLines()
	: CompVGLDraw(kProgramVertexData, kProgramFragmentData, kVertexDataWithMVP_Yes)
	, m_fboWidth(0)
	, m_fboHeight(0)
{

}

CompVGLDrawLines::~CompVGLDrawLines()
{

}

COMPV_ERROR_CODE CompVGLDrawLines::lines(const CompVGLPoint2D* lines, GLsizei count)
{
	COMPV_CHECK_CODE_RETURN(draw(lines, count));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLDrawLines::matches(const CompVGLPoint2D* lines, GLsizei count)
{
	static const GLsizei trainOffsetx = 0;
	COMPV_CHECK_CODE_RETURN(draw(lines, count, COMPV_GL_LINE_TYPE_MATCH));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLDrawLines::draw(const CompVGLPoint2D* lines, GLsizei count, COMPV_GL_LINE_TYPE type COMPV_DEFAULT(COMPV_GL_LINE_TYPE_SIMPLE))
{
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	GLint fboWidth = 0, fboHeight = 0;
	bool bFirstTimeOrChanged;

	// Bind to VAO, VBO, Program
	COMPV_CHECK_CODE_BAIL(err = CompVGLDraw::bind());

	// Get FBO width
	COMPV_glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &fboWidth);
	COMPV_glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &fboHeight);
	COMPV_CHECK_EXP_BAIL(!fboWidth || !fboHeight, (err = COMPV_ERROR_CODE_E_GL), "fboWidth or fboHeight is equal to zero");
	bFirstTimeOrChanged = (m_fboWidth != fboWidth || m_fboHeight != fboHeight);

	// Submit vertices data
	COMPV_glBufferData(GL_ARRAY_BUFFER, sizeof(CompVGLPoint2D) * count, lines, GL_STATIC_DRAW);

	if (!CompVGLInfo::extensions::vertex_array_object() || bFirstTimeOrChanged) {
		// Set position attribute
		GLuint uNamePosition = COMPV_glGetAttribLocation(program()->name(), "position");
		COMPV_glEnableVertexAttribArray(uNamePosition);
		COMPV_glVertexAttribPointer(uNamePosition, 2, GL_FLOAT, GL_FALSE, sizeof(CompVGLPoint2D), reinterpret_cast<const GLvoid *>(offsetof(CompVGLPoint2D, position)));

		// Set color attribute
		GLuint uNameColor = COMPV_glGetAttribLocation(program()->name(), "color");
		COMPV_glEnableVertexAttribArray(uNameColor);
		COMPV_glVertexAttribPointer(uNameColor, 3, GL_FLOAT, GL_FALSE, sizeof(CompVGLPoint2D), reinterpret_cast<const GLvoid *>(offsetof(CompVGLPoint2D, color)));

		// Set projection
		COMPV_CHECK_CODE_BAIL(err = CompVGLDraw::setOrtho(0, static_cast<GLfloat>(fboWidth), static_cast<GLfloat>(fboHeight), 0, -1, 1));
	}

	// Draw points
	COMPV_glLineWidth(2.f);
	COMPV_glViewport(0, 0, static_cast<GLsizei>(fboWidth), static_cast<GLsizei>(fboHeight));
	COMPV_glDrawArrays(GL_LINES, 0, count);
	if (type == COMPV_GL_LINE_TYPE_MATCH) {
		COMPV_glDrawArrays(GL_POINTS, 0, count);
	}

	// Update size
	m_fboWidth = fboWidth;
	m_fboHeight = fboHeight;

bail:
	COMPV_CHECK_CODE_NOP(CompVGLDraw::unbind());
	return err;
}

COMPV_ERROR_CODE CompVGLDrawLines::newObj(CompVGLDrawLinesPtrPtr drawPoints)
{
	COMPV_CHECK_EXP_RETURN(!drawPoints, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVGLDrawLinesPtr drawPoints_ = new CompVGLDrawLines();
	COMPV_CHECK_EXP_RETURN(!drawPoints_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

	*drawPoints = drawPoints_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

