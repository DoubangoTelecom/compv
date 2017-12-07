/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
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
"	attribute vec4 color;"
"	varying vec4 colorVarying;"
"	uniform mat4 MVP;"
"	uniform float pointSize;"
"	void main() {"
"		gl_PointSize = pointSize;"
"		gl_Position = MVP * vec4(position, 1.0, 1.0);"
"		colorVarying = vec4(color);"
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

COMPV_ERROR_CODE CompVGLDrawLines::lines(const CompVGLPoint2D* lines, const GLsizei count, const CompVDrawingOptions* options COMPV_DEFAULT(nullptr), bool connected COMPV_DEFAULT(false))
{
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	GLint fboWidth = 0, fboHeight = 0;
	const GLfloat linewidth = options ? static_cast<GLfloat>(options->lineWidth) : 2.f;
	const GLfloat pointSize = options ? static_cast<GLfloat>(options->pointSize) : 7.f;
	const bool loop = options ? options->lineLoop : false; // GL_LINE_LOOP or GL_LINE_STRIP
	bool bFirstTimeOrChanged;
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
		uName = COMPV_glGetAttribLocation(program()->name(), "position");
		COMPV_glEnableVertexAttribArray(uName);
		COMPV_glVertexAttribPointer(uName, 2, GL_FLOAT, GL_FALSE, sizeof(CompVGLPoint2D), reinterpret_cast<const GLvoid *>(offsetof(CompVGLPoint2D, position)));

		// Set color attribute
		uName = COMPV_glGetAttribLocation(program()->name(), "color");
		COMPV_glEnableVertexAttribArray(uName);
		COMPV_glVertexAttribPointer(uName, 4, GL_FLOAT, GL_FALSE, sizeof(CompVGLPoint2D), reinterpret_cast<const GLvoid *>(offsetof(CompVGLPoint2D, color)));

		// Set projection
		COMPV_CHECK_CODE_BAIL(err = CompVGLDraw::setOrtho(0, static_cast<GLfloat>(fboWidth), static_cast<GLfloat>(fboHeight), 0, -1, 1));
	}

	// Set PointSize
	uName = COMPV_glGetUniformLocation(program()->name(), "pointSize");
	COMPV_glUniform1f(uName, pointSize);

	// Submit vertices data
	COMPV_glBufferData(GL_ARRAY_BUFFER, sizeof(CompVGLPoint2D) * count, lines, GL_STATIC_DRAW);

	// Draw points
	COMPV_glLineWidth(linewidth);
	COMPV_glViewport(0, 0, static_cast<GLsizei>(fboWidth), static_cast<GLsizei>(fboHeight)); 
	COMPV_glDrawArrays(connected ? (loop ? GL_LINE_LOOP : GL_LINE_STRIP) : GL_LINES, 0, count);
	if (options && options->lineType == COMPV_DRAWING_LINE_TYPE_MATCH) {
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

