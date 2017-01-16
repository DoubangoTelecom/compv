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
	GLfloat* glMemPints_ = NULL;
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	static const size_t numComps = 2; // x and y comps (later will add color comp)
	size_t i = 0;

	glMemPints_ = reinterpret_cast<GLfloat*>(CompVMem::malloc(interestPoints.size() * numComps * sizeof(GLfloat)));
	COMPV_CHECK_EXP_RETURN(!glMemPints_, (err = COMPV_ERROR_CODE_E_OUT_OF_MEMORY), "Failed to allocation GL points");

	for (std::vector<CompVInterestPoint >::const_iterator it = interestPoints.begin(); it != interestPoints.end(); ++it, i += numComps) {
		glMemPints_[i] = static_cast<GLfloat>((*it).x);
		glMemPints_[i + 1] = static_cast<GLfloat>((*it).y);
	}

	COMPV_CHECK_CODE_BAIL(err = m_ptrDrawPoints->process(glMemPints_, static_cast<GLsizei>(interestPoints.size())));

bail:
	CompVMem::free(reinterpret_cast<void**>(&glMemPints_));
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