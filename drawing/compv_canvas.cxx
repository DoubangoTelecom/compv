/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/drawing/compv_canvas.h"
#include "compv/base/compv_base.h"

#include "compv/drawing/skia/compv_canvas_skia.h"

COMPV_NAMESPACE_BEGIN()

//
//	CompVCanvasImpl
//

CompVCanvasImpl::CompVCanvasImpl()
{

}

CompVCanvasImpl::~CompVCanvasImpl()
{

}

COMPV_ERROR_CODE CompVCanvasImpl::newObj(CompVCanvasImplPtrPtr canvasImpl)
{
	COMPV_CHECK_CODE_RETURN(CompVBase::init());
	COMPV_CHECK_EXP_RETURN(!canvasImpl, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVCanvasImplPtr canvasImpl_;

#if defined(HAVE_SKIA)
	CompVCanvasImplSkiaPtr skiaCanvasImpl;
	COMPV_CHECK_CODE_RETURN(CompVCanvasImplSkia::newObj(&skiaCanvasImpl));
	canvasImpl_ = *skiaCanvasImpl;
#endif /* HAVE_GLFW_GLFW3_H */

	COMPV_CHECK_EXP_RETURN(!(*canvasImpl = canvasImpl_), COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);

	return COMPV_ERROR_CODE_S_OK;
}


//
//	CompVCanvas
//
CompVCanvas::CompVCanvas()
{

}

CompVCanvas::~CompVCanvas()
{
	COMPV_CHECK_CODE_ASSERT(deInit());
}

COMPV_ERROR_CODE CompVCanvas::drawText(const void* textPtr, size_t textLengthInBytes, size_t x, size_t y)
{
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	COMPV_CHECK_CODE_BAIL(err = init());
	COMPV_CHECK_CODE_BAIL(err = canvasBind());
	COMPV_CHECK_CODE_BAIL(err = m_ptrImpl->drawText(textPtr, textLengthInBytes, x, y));
bail:
	COMPV_CHECK_CODE_ASSERT(canvasUnbind());
	return err;
}

COMPV_ERROR_CODE CompVCanvas::init()
{
	if (!m_ptrImpl) {
		COMPV_CHECK_CODE_RETURN(CompVCanvasImpl::newObj(&m_ptrImpl));
	}
	return COMPV_ERROR_CODE_S_OK;
}
	
COMPV_ERROR_CODE CompVCanvas::deInit()
{
	m_ptrImpl = NULL;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

