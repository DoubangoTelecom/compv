/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_GL_RENDERER_GL_H_)
#define _COMPV_GL_RENDERER_GL_H_

#include "compv/gl/compv_gl_config.h"
#include "compv/gl/compv_gl_headers.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/base/drawing/compv_renderer.h"
#include "compv/gl/compv_gl_common.h"
#include "compv/gl/compv_gl_fbo.h"
#include "compv/gl/compv_gl_canvas.h"
#include "compv/gl/compv_gl_blitter.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

COMPV_OBJECT_DECLARE_PTRS(GLRenderer)

class CompVGLRenderer : public CompVRenderer
{
protected:
    CompVGLRenderer(COMPV_SUBTYPE ePixelFormat);
public:
    virtual ~CompVGLRenderer();
    COMPV_OBJECT_GET_ID(CompVGLRenderer);

    COMPV_INLINE CompVGLBlitterPtr blitter() {
        return m_ptrBlitter;
    }

    COMPV_OVERRIDE_DECL1("CompVRenderer", bool, isGLEnabled)()const override {
        return true;
    };
    COMPV_OVERRIDE_DECL1("CompVRenderer", CompVCanvasPtr, canvas)() override;

    COMPV_ERROR_CODE close();

    static COMPV_ERROR_CODE newObj(CompVGLRendererPtrPtr glRenderer, COMPV_SUBTYPE ePixelFormat);

protected:
    virtual COMPV_ERROR_CODE deInit();
    virtual COMPV_ERROR_CODE init(CompVMatPtr mat, const std::string& prgVertexData, const std::string& prgFragData, bool bMVP = false, bool bToScreen = false);
    virtual COMPV_ERROR_CODE bind();
    virtual COMPV_ERROR_CODE unbind();

private:
    COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
    bool m_bInit;
    CompVGLCanvasPtr m_ptrCanvas;
    CompVGLBlitterPtr m_ptrBlitter;
    COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

#endif /* _COMPV_GL_RENDERER_GL_H_ */