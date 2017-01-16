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
	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}
	
COMPV_ERROR_CODE CompVGLCanvasImpl::drawLine(int x0, int y0, int x1, int y1)  /*Overrides(CompVCanvasInterface)*/
{
	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLCanvasImpl::drawInterestPoints(const std::vector<CompVInterestPoint >& interestPoints) /*Overrides(CompVCanvasInterface)*/
{
	if (interestPoints.empty()) {
		return COMPV_ERROR_CODE_S_OK;
	}
	if (!m_ptrDrawPoints) {
		COMPV_CHECK_CODE_RETURN(CompVGLDrawPoints::newObj(&m_ptrDrawPoints));
	}
	CompVGLPoints* glMemPoints_ = NULL, *glMemPoint_;
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	const GLfloat(*color)[3];

	glMemPoints_ = reinterpret_cast<CompVGLPoints*>(CompVMem::malloc(interestPoints.size() * sizeof(CompVGLPoints)));
	COMPV_CHECK_EXP_RETURN(!glMemPoints_, (err = COMPV_ERROR_CODE_E_OUT_OF_MEMORY), "Failed to allocation GL points");

	glMemPoint_ = glMemPoints_;
	for (std::vector<CompVInterestPoint >::const_iterator it = interestPoints.begin(); it != interestPoints.end(); ++it, ++glMemPoint_) {
		// x, y
		glMemPoint_->position[0] = static_cast<GLfloat>((*it).x);
		glMemPoint_->position[1] = static_cast<GLfloat>((*it).y);
		// r, g, b
		color = &kCompVGLRandomColors[rand() % kCompVGLRandomColorsCount];
		glMemPoint_->color[0] = (*color)[0];
		glMemPoint_->color[1] = (*color)[1];
		glMemPoint_->color[2] = (*color)[2];
	}

	COMPV_CHECK_CODE_BAIL(err = m_ptrDrawPoints->process(glMemPoints_, static_cast<GLsizei>(interestPoints.size())));

bail:
	CompVMem::free(reinterpret_cast<void**>(&glMemPoints_));
	return err;
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