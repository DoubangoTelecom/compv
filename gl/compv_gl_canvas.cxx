/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/gl/compv_gl_canvas.h"
#include "compv/gl/compv_gl_headers.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)

COMPV_NAMESPACE_BEGIN()

CompVGLCanvas::CompVGLCanvas(CompVGLFboPtr ptrFBO, CompVCanvasImplPtr ptrImpl)
    : CompVCanvas()
    , m_bEmpty(true)
    , m_ptrFBO(ptrFBO)
    , m_ptrImpl(ptrImpl)
{

}

CompVGLCanvas::~CompVGLCanvas()
{

}

COMPV_ERROR_CODE CompVGLCanvas::drawText(const void* textPtr, size_t textLengthInBytes, int x, int y) /*Overrides(CompVCanvasInterface)*/
{
    COMPV_CHECK_EXP_RETURN(!textPtr || !textLengthInBytes, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    COMPV_GL_FBO_AUTOBIND(*m_ptrFBO);
    COMPV_CHECK_CODE_RETURN(m_ptrImpl->drawText(textPtr, textLengthInBytes, x, y));
    unMakeEmpty();
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLCanvas::drawLine(int x0, int y0, int x1, int y1) /*Overrides(CompVCanvasInterface)*/
{
    COMPV_GL_FBO_AUTOBIND(*m_ptrFBO);
    COMPV_CHECK_CODE_RETURN(m_ptrImpl->drawLine(x0, y0, x1, y1));
    unMakeEmpty();
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLCanvas::drawInterestPoints(const std::vector<CompVInterestPoint >& interestPoints) /*Overrides(CompVCanvasInterface)*/
{
	COMPV_GL_FBO_AUTOBIND(*m_ptrFBO);
	COMPV_CHECK_CODE_RETURN(m_ptrImpl->drawInterestPoints(interestPoints));
	unMakeEmpty();
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLCanvas::close()
{
	if (m_ptrImpl) {
		COMPV_CHECK_CODE_NOP(m_ptrImpl->close());
	}
    if (m_ptrFBO) {
		COMPV_CHECK_CODE_NOP(m_ptrFBO->close());
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLCanvas::newObj(CompVGLCanvasPtrPtr canvas, CompVGLFboPtr ptrFBO, CompVCanvasImplPtr ptrImpl)
{
    COMPV_CHECK_EXP_RETURN(!canvas || !ptrFBO || !ptrImpl, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVGLCanvasPtr canvas_ = new CompVGLCanvas(ptrFBO, ptrImpl);
    COMPV_CHECK_EXP_RETURN(!canvas_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    *canvas = canvas_;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */
