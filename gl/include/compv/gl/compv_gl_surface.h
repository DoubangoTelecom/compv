/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_GL_SURFACE_H_)
#define _COMPV_GL_SURFACE_H_

#include "compv/gl/compv_gl_config.h"
#include "compv/gl/compv_gl_headers.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/base/drawing/compv_mvp.h"
#include "compv/base/drawing/compv_surface.h"
#include "compv/gl/compv_gl_renderer.h"
#include "compv/gl/compv_gl_fbo.h"
#include "compv/gl/compv_gl_canvas.h"
#include "compv/gl/compv_gl_blitter.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

COMPV_OBJECT_DECLARE_PTRS(GLSurface)

class COMPV_GL_API CompVGLSurface : public CompVSurface
{
protected:
    CompVGLSurface(size_t width, size_t height);
public:
    virtual ~CompVGLSurface();
    COMPV_OBJECT_GET_ID(CompVGLSurface);
    COMPV_INLINE CompVGLBlitterPtr blitter() {
        return m_ptrBlitter;
    }
    CompVGLCanvasPtr canvasGL();
	CompVGLRendererPtr rendererGL();

	virtual bool isGLEnabled()const override /*Overrides(CompVSurface)*/ {
        return true;
    };
	virtual COMPV_ERROR_CODE setMVP(CompVMVPPtr mvp) override /*Overrides(CompVSurface)*/;
	virtual CompVRendererPtr renderer() override /*Overrides(CompVSurface)*/;
	virtual CompVCanvasPtr canvas() override /*Overrides(CompVSurface)*/;
	virtual CompVCanvasPtr requestCanvas(size_t width = 0, size_t height = 0) override /*Overrides(CompVSurface)*/;
	virtual COMPV_ERROR_CODE drawImage(const CompVMatPtr& mat, const CompVViewportPtr& viewport = nullptr) override /*Overrides(CompVSurface)*/;

    COMPV_ERROR_CODE blit(const CompVGLFboPtr ptrFboSrc, const CompVGLFboPtr ptrFboDst);
    COMPV_ERROR_CODE blitRenderer(const CompVGLFboPtr ptrFboDst);

    COMPV_ERROR_CODE updateSize(size_t newWidth, size_t newHeight);

    COMPV_ERROR_CODE close();

    static COMPV_ERROR_CODE newObj(CompVGLSurfacePtrPtr glSurface, size_t width, size_t height);

private:
    COMPV_ERROR_CODE init();
    COMPV_ERROR_CODE deInit();

private:
    COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
    bool m_bInit;
    CompVGLRendererPtr m_ptrRenderer;
    CompVGLProgramPtr m_ptrProgram;
    CompVGLCanvasPtr m_ptrCanvas;
    CompVGLBlitterPtr m_ptrBlitter;
    COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

#endif /* _COMPV_GL_SURFACE_H_ */
