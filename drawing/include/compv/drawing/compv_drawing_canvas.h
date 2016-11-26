/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_DRAWING_CANVAS_H_)
#define _COMPV_DRAWING_CANVAS_H_

#include "compv/drawing/compv_config.h"
#include "compv/drawing/compv_common.h"
#include "compv/drawing/skia/compv_canvas_skia.h"
#include "compv/base/drawing/compv_canvas.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class CompVDrawingCanvasImpl
{
public:
	static COMPV_ERROR_CODE newObj(CompVCanvasImplPtrPtr canvasImpl)
	{
		COMPV_CHECK_EXP_RETURN(!canvasImpl, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		CompVCanvasImplPtr canvasImpl_;
#if defined(HAVE_SKIA)
		CompVCanvasImplSkiaPtr canvasImplSkia;
		COMPV_CHECK_CODE_RETURN(CompVCanvasImplSkia::newObj(&canvasImplSkia));
		canvasImpl_ = *canvasImplSkia;
#endif /* HAVE_SKIA */
		COMPV_CHECK_EXP_RETURN(!(*canvasImpl = canvasImpl_), COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
		return COMPV_ERROR_CODE_S_OK;
	}
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_DRAWING_CANVAS_H_ */
