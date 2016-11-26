/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_DRAWING_SURFACE_GL_H_)
#define _COMPV_DRAWING_SURFACE_GL_H_

#include "compv/drawing/compv_config.h"
#include "compv/gl/compv_gl_headers.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/drawing/compv_common.h"
#include "compv/base/compv_obj.h"
#include "compv/base/drawing/compv_mvp.h"
#include "compv/drawing/compv_surface.h"
#include "compv/drawing/gl/compv_renderer_gl.h"
#include "compv/gl/compv_gl_fbo.h"
#include "compv/gl/compv_gl_canvas.h"
#include "compv/gl/compv_gl_blitter.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class CompVSurfaceGL;
typedef CompVPtr<CompVSurfaceGL* > CompVSurfaceGLPtr;
typedef CompVSurfaceGLPtr* CompVSurfaceGLPtrPtr;

class CompVSurfaceGL : public CompVSurface, public CompVGLBlitter
{
protected:
	CompVSurfaceGL(size_t width, size_t height);
public:
	virtual ~CompVSurfaceGL();
	COMPV_GET_OBJECT_ID(CompVSurfaceGL);
	COMPV_INLINE CompVGLFboPtr canvasFBO() { return m_ptrCanvasFBO; }
	CompVGLCanvasPtr canvasGL();

	COMPV_OVERRIDE_DECL1("CompVSurface", bool, isGLEnabled)()const override { return true; };
	COMPV_OVERRIDE_DECL0("CompVSurface", setMVP)(CompVMVPPtr mvp) override;
	COMPV_OVERRIDE_DECL1("CompVSurface", CompVRendererPtr, renderer)() override;
	COMPV_OVERRIDE_DECL1("CompVSurface", CompVCanvasPtr, canvas)() override;
	COMPV_OVERRIDE_DECL0("CompVSurface", drawImage)(CompVMatPtr mat) override;

	COMPV_ERROR_CODE blit(const CompVGLFboPtr ptrFboSrc, const CompVGLFboPtr ptrFboDst);
	COMPV_ERROR_CODE blitRenderer(const CompVGLFboPtr ptrFboDst);

	COMPV_ERROR_CODE updateSize(size_t newWidth, size_t newHeight);
	COMPV_ERROR_CODE setCanvasFBO(CompVGLFboPtr fbo);

	static COMPV_ERROR_CODE newObj(CompVSurfaceGLPtrPtr glSurface, size_t width, size_t height);

private:
	COMPV_ERROR_CODE init();
	COMPV_ERROR_CODE deInit();

private:
	bool m_bInit;
	CompVRendererGLPtr m_ptrRenderer;
	CompVGLProgramPtr m_ptrProgram;
	CompVGLFboPtr m_ptrCanvasFBO;
	CompVGLCanvasPtr m_ptrCanvas;
};

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

#endif /* _COMPV_DRAWING_SURFACE_GL_H_ */
