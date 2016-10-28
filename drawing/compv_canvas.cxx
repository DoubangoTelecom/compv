/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/drawing/compv_canvas.h"
#include "compv/base/compv_base.h"

#include "compv/drawing/compv_canvas_skia.h"

COMPV_NAMESPACE_BEGIN()

CompVCanvas::CompVCanvas()
{

}

CompVCanvas::~CompVCanvas()
{

}

COMPV_ERROR_CODE CompVCanvas::newObj(CompVCanvasPtrPtr canvas)
{
	COMPV_CHECK_CODE_RETURN(CompVBase::init());
	COMPV_CHECK_EXP_RETURN(canvas == NULL, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	*canvas = NULL;

#if defined(HAVE_SKIA)
	CompVCanvasSkiaPtr skiaCanvas;
	COMPV_CHECK_CODE_RETURN(CompVCanvasSkia::newObj(&skiaCanvas));
	*canvas = dynamic_cast<CompVCanvas*>(*skiaCanvas);
#endif /* HAVE_GLFW_GLFW3_H */

	COMPV_CHECK_EXP_RETURN(!*canvas, COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

