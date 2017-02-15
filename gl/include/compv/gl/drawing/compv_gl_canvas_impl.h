/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_GL_DRAWING_CANVAS_IMPL_H_)
#define _COMPV_GL_DRAWING_CANVAS_IMPL_H_

#include "compv/gl/compv_gl_config.h"
#include "compv/gl/compv_gl_headers.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/gl/drawing/compv_gl_draw_points.h"
#include "compv/gl/drawing/compv_gl_draw_lines.h"
#include "compv/base/drawing/compv_canvas.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

extern COMPV_GL_API const CompVCanvasFactory CompVCanvasFactoryGL;

COMPV_OBJECT_DECLARE_PTRS(GLCanvasImpl)

class CompVGLCanvasImpl : public CompVCanvasImpl
{
protected:
	CompVGLCanvasImpl();
public:
	virtual ~CompVGLCanvasImpl();
	COMPV_OBJECT_GET_ID(CompVGLCanvasImpl);

	virtual COMPV_ERROR_CODE drawText(const void* textPtr, size_t textLengthInBytes, int x, int y) override /*Overrides(CompVCanvasInterface)*/;
	virtual COMPV_ERROR_CODE drawLines(const compv_float32_t* x0, const compv_float32_t* y0, const compv_float32_t* x1, const compv_float32_t* y1, size_t count) override /*Overrides(CompVCanvasInterface)*/;
	virtual COMPV_ERROR_CODE drawInterestPoints(const CompVInterestPointVector & interestPoints) override /*Overrides(CompVCanvasInterface)*/;

	virtual COMPV_ERROR_CODE close() override /*Overrides(CompVCanvasImpl)*/;

	static COMPV_ERROR_CODE newObj(CompVGLCanvasImplPtrPtr canvas);

private:

private:
	CompVGLDrawPointsPtr m_ptrDrawPoints;
	CompVGLDrawLinesPtr m_ptrDrawLines;
};

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

#endif /* _COMPV_GL_DRAWING_CANVAS_IMPL_H_ */
