/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_DRAWING_CANVAS_SKIA_H_)
#define _COMPV_DRAWING_CANVAS_SKIA_H_

#include "compv/drawing/compv_drawing_config.h"
#if HAVE_SKIA
#include "compv/drawing/compv_drawing_common.h"
#include "compv/base/drawing/compv_canvas.h"
#include "compv/base/compv_obj.h"
#include "compv/base/compv_mat.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

#include <string>

#include <GrContext.h>
#include <SkCanvas.h>
#include <SkGraphics.h>
#include <SkSurface.h>
#include <gl/GrGLInterface.h>

COMPV_NAMESPACE_BEGIN()

extern const CompVCanvasFactory CompVCanvasFactorySkia;

COMPV_OBJECT_DECLARE_PTRS(CanvasImplSkia)

class CompVCanvasImplSkia : public CompVCanvasImpl
{
protected:
    CompVCanvasImplSkia();
public:
    virtual ~CompVCanvasImplSkia();
    COMPV_OBJECT_GET_ID(CompVCanvasImplSkia);

    virtual COMPV_ERROR_CODE drawText(const void* textPtr, size_t textLengthInBytes, int x, int y) override /*Overrides(CompVCanvasInterface)*/;
    virtual COMPV_ERROR_CODE drawLines(const compv_float32_t* x0, const compv_float32_t* y0, const compv_float32_t* x1, const compv_float32_t* y1, size_t count) override /*Overrides(CompVCanvasInterface)*/;
	virtual COMPV_ERROR_CODE drawInterestPoints(const std::vector<CompVInterestPoint >& interestPoints) override /*Overrides(CompVCanvasInterface)*/;

	virtual COMPV_ERROR_CODE close() override /*Overrides(CompVCanvasImpl)*/;

    static COMPV_ERROR_CODE newObj(CompVCanvasImplSkiaPtrPtr skiaCanvas);

private:
	bool isContextGLChanged()const;
	COMPV_ERROR_CODE init();
	COMPV_ERROR_CODE deInit();

private:
	GrContext* m_pContextSkia;
	SkSurface* m_pSurfaceSkia;
	const void* m_pcContextGL;
	bool m_bInitialized;
};

COMPV_NAMESPACE_END()

#endif /* HAVE_SKIA */

#endif /* _COMPV_DRAWING_CANVAS_SKIA_H_ */
