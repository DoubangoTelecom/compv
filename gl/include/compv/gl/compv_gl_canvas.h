/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_GL_CANVAS_H_)
#define _COMPV_GL_CANVAS_H_

#include "compv/gl/compv_gl_config.h"
#include "compv/gl/compv_gl_headers.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/base/drawing/compv_canvas.h"
#include "compv/gl/compv_gl_fbo.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

COMPV_OBJECT_DECLARE_PTRS(GLCanvas)

class COMPV_GL_API CompVGLCanvas : public CompVCanvas
{
protected:
    CompVGLCanvas(CompVGLFboPtr ptrFBO, CompVCanvasImplPtr ptrImpl);
public:
    virtual ~CompVGLCanvas();
    COMPV_OBJECT_GET_ID("CompVGLCanvas");
    COMPV_INLINE bool isEmpty()const {
        return m_bEmpty;
    }

    COMPV_OVERRIDE_DECL0("CompVCanvasInterface", drawText)(const void* textPtr, size_t textLengthInBytes, int x, int y) override;
    COMPV_OVERRIDE_DECL0("CompVCanvasInterface", drawLine)(int x0, int y0, int x1, int y1) override;

    COMPV_ERROR_CODE close();

    static COMPV_ERROR_CODE newObj(CompVGLCanvasPtrPtr canvas, CompVGLFboPtr ptrFBO, CompVCanvasImplPtr ptrImpl);

private:
    COMPV_INLINE void makeEmpty() {
        m_bEmpty = true;
    }
    COMPV_INLINE void unMakeEmpty() {
        m_bEmpty = false;
    }

private:
    COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
    bool m_bEmpty;
    CompVGLFboPtr m_ptrFBO;
    CompVCanvasImplPtr m_ptrImpl;
    COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

#endif /* _COMPV_GL_CANVAS_H_ */
