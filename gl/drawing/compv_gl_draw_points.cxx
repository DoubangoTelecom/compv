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
"	void main() {"
"		gl_PointSize = 10.0;"
"		gl_Position = position;"
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
	GLuint position_attribute;
	
	// Bind to VAO, VBO, Program
	COMPV_CHECK_CODE_BAIL(err = CompVGLDraw::bind());

	// Submit vertices data
	COMPV_glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2 * count, xy, GL_STATIC_DRAW);

	// Set position attribute (FIXME: do onces if VAO supported)
	position_attribute = COMPV_glGetAttribLocation(program()->name(), "position");
	COMPV_glEnableVertexAttribArray(position_attribute);
	COMPV_glVertexAttribPointer(position_attribute, 2, GL_FLOAT, GL_FALSE, 0, 0);

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

