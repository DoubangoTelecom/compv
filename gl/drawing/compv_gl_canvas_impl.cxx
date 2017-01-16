/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/gl/drawing/compv_gl_canvas_impl.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/gl/compv_gl_func.h"
#include "compv/gl/compv_gl_utils.h"
#include "compv/gl/compv_gl_info.h"
#include "compv/gl/compv_gl_program.h"

COMPV_NAMESPACE_BEGIN()

static COMPV_ERROR_CODE CompVGLCanvasImpl_newObj(CompVCanvasImplPtrPtr canvasImpl)
{
	CompVGLCanvasImplPtr canvasImplGL;
	COMPV_CHECK_CODE_RETURN(CompVGLCanvasImpl::newObj(&canvasImplGL));
	*canvasImpl = *canvasImplGL;
	return COMPV_ERROR_CODE_S_OK;
}
COMPV_GL_API const CompVCanvasFactory CompVCanvasFactoryGL = {
	"GL",
	CompVGLCanvasImpl_newObj
};

CompVGLCanvasImpl::CompVGLCanvasImpl()
{

}

CompVGLCanvasImpl::~CompVGLCanvasImpl()
{

}

COMPV_ERROR_CODE CompVGLCanvasImpl::drawText(const void* textPtr, size_t textLengthInBytes, int x, int y)  /*Overrides(CompVCanvasInterface)*/
{
	if (!m_ptrDrawPoints) {
		COMPV_CHECK_CODE_RETURN(CompVGLDrawPoints::newObj(&m_ptrDrawPoints));
	}
	static const size_t points = 100;
	GLfloat tests[points * 2];
	for (size_t i = 0; i < points; i+=2) {
		tests[i] = float(rand() % 1000) / 1000.f;
		tests[i+1] = float(rand() % 1000) / 1000.f;
	}

	COMPV_CHECK_CODE_RETURN(m_ptrDrawPoints->process(tests, points));
	return COMPV_ERROR_CODE_S_OK;

#if 0 // FIXME: remove
	GLuint vao = kCompVGLNameInvalid;
	GLuint vbo = kCompVGLNameInvalid;
	GLint position_attribute;
	CompVGLProgramPtr program;
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;

	static GLfloat vertices_position[24] = {
		 	0.0, 0.0,
		 	0.5, 0.0,
		 	0.5, 0.5,
		
		 	0.0, 0.0,
		 	0.0, 0.5,
			- 0.5, 0.5,
		
		 	0.0, 0.0,
			- 0.5, 0.0,
			- 0.5, -0.5,
		
		 	0.0, 0.0,
			0.0, -0.5,
			0.5, -0.5,
	};

	// VAO
	if (CompVGLInfo::extensions::vertex_array_object()) {
		COMPV_CHECK_CODE_BAIL(err = CompVGLUtils::vertexArraysGen(&vao));
		COMPV_glBindVertexArray(vao);
	}
	// FIXME: what else?

	// VBO
	COMPV_CHECK_CODE_BAIL(err = CompVGLUtils::bufferGen(&vbo));
	COMPV_glBindBuffer(GL_ARRAY_BUFFER, vbo);
	COMPV_glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_position), vertices_position, GL_STATIC_DRAW);

	// Program
	COMPV_CHECK_CODE_BAIL(CompVGLProgram::newObj(&program, kProgramVertexData.c_str(), kProgramVertexData.length(), kProgramFragmentData.c_str(), kProgramFragmentData.length()));
	COMPV_CHECK_CODE_BAIL(err = program->bind());
	position_attribute = COMPV_glGetAttribLocation(program->name(), "position");
	COMPV_glEnableVertexAttribArray(position_attribute);
	COMPV_glVertexAttribPointer(position_attribute, 2, GL_FLOAT, GL_FALSE, 0, 0);	
	
	// Draw
	//COMPV_glDrawArrays(GL_TRIANGLES, 0, 12);
	COMPV_glDrawArrays(GL_POINTS, 0, 12);

bail:
	if (program) {
		COMPV_CHECK_CODE_NOP(program->unbind());
	}
	COMPV_glBindBuffer(GL_ARRAY_BUFFER, kCompVGLNameInvalid);
	COMPV_glBindVertexArray(kCompVGLNameInvalid);
	CompVGLUtils::bufferDelete(&vbo);
	CompVGLUtils::vertexArraysDelete(&vao);
	return COMPV_ERROR_CODE_S_OK;
#endif
}
	
COMPV_ERROR_CODE CompVGLCanvasImpl::drawLine(int x0, int y0, int x1, int y1)  /*Overrides(CompVCanvasInterface)*/
{
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLCanvasImpl::close() /*Overrides(CompVCanvasImpl)*/
{
	m_ptrDrawPoints = NULL;

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLCanvasImpl::newObj(CompVGLCanvasImplPtrPtr canvas)
{
	COMPV_CHECK_EXP_RETURN(!canvas, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVGLCanvasImplPtr canvas_ = new CompVGLCanvasImpl();
	COMPV_CHECK_EXP_RETURN(!canvas_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

	*canvas = canvas_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */