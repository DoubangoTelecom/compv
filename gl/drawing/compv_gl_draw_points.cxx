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
"	attribute vec3 color;"
"	varying vec4 colorVarying;"
"	uniform mat4 MVP;"
"	void main() {"
"		gl_PointSize = 7.0;"
"		gl_Position = MVP * vec4(position, 1.0, 1.0);"
"		colorVarying = vec4(color, 1.0);"
"	}";

static const std::string& kProgramFragmentData =
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

COMPV_ERROR_CODE CompVGLDrawPoints::process(const CompVGLPoints* points, GLsizei count)
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
	COMPV_glBufferData(GL_ARRAY_BUFFER, sizeof(CompVGLPoints) * count, points, GL_STATIC_DRAW);
	
	if (!CompVGLInfo::extensions::vertex_array_object() || bFirstTimeOrChanged) {
		// Set position attribute
		GLuint uNamePosition = COMPV_glGetAttribLocation(program()->name(), "position");
		COMPV_glEnableVertexAttribArray(uNamePosition);
		COMPV_glVertexAttribPointer(uNamePosition, 2, GL_FLOAT, GL_FALSE, sizeof(CompVGLPoints), 0);

		// Set color attribute
		GLuint uNameColor = COMPV_glGetAttribLocation(program()->name(), "color");
		COMPV_glEnableVertexAttribArray(uNameColor);
		COMPV_glVertexAttribPointer(uNameColor, 3, GL_FLOAT, GL_FALSE, sizeof(CompVGLPoints), (GLvoid*)(sizeof(GLfloat) * 2));

		// Set projection
		COMPV_CHECK_CODE_BAIL(err = CompVGLDraw::setOrtho(0, static_cast<GLfloat>(fboWidth), static_cast<GLfloat>(fboHeight), 0, -1, 1));
	}

	// Draw points
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

