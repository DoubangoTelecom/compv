/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_DRAWING_SURFACE_GL_H_)
#define _COMPV_DRAWING_SURFACE_GL_H_

#include "compv/base/compv_config.h"
#include "compv/drawing/opengl/compv_headers_gl.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/base/compv_common.h"
#include "compv/base/compv_obj.h"
#include "compv/drawing/compv_surface.h"
#include "compv/drawing/opengl/compv_mvp_glm.h"
#include "compv/drawing/opengl/compv_renderer_gl.h"
#include "compv/drawing/opengl/compv_fbo_gl.h"
#include "compv/drawing/opengl/compv_blitter_gl.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class CompVWindow;

class CompVSurfaceGL;
typedef CompVPtr<CompVSurfaceGL* > CompVSurfaceGLPtr;
typedef CompVSurfaceGLPtr* CompVSurfaceGLPtrPtr;

class CompVSurfaceGL : public CompVSurface, public CompVBlitterGL
{
protected:
	CompVSurfaceGL(size_t width, size_t height);
public:
	virtual ~CompVSurfaceGL();
	COMPV_GET_OBJECT_ID("CompVSurfaceGL");
	COMPV_INLINE CompVViewportPtr viewport() { return m_ptrViewport; }
	COMPV_INLINE CompVRendererGLPtr renderer() { return m_ptrRenderer; }
	COMPV_INLINE CompVFBOGLPtr canvasFBO() { return m_ptrCanvasFBO;}

	// Overrides(CompVSurface)
	virtual bool isGLEnabled()const  override { return true; };
	virtual COMPV_ERROR_CODE setMVP(CompVMVPPtr mvp) override;
	virtual COMPV_ERROR_CODE setViewport(CompVViewportPtr viewport) override;
	virtual COMPV_ERROR_CODE drawImage(CompVMatPtr mat, CompVRendererPtrPtr renderer = NULL) override;

	COMPV_ERROR_CODE blit(const CompVFBOGLPtr ptrFboSrc, const CompVFBOGLPtr ptrFboDst);
	COMPV_ERROR_CODE blitRenderer(const CompVFBOGLPtr ptrFboDst);

	COMPV_ERROR_CODE updateSize(size_t newWidth, size_t newHeight);
	COMPV_ERROR_CODE setCanvasFBO(CompVFBOGLPtr fbo);

	static COMPV_ERROR_CODE newObj(CompVSurfaceGLPtrPtr glSurface, size_t width, size_t height);

protected:
	// Overrides(CompVCanvas) 
	virtual COMPV_ERROR_CODE canvasBind() override;
	virtual COMPV_ERROR_CODE canvasUnbind()override;

private:
	COMPV_ERROR_CODE init();
	COMPV_ERROR_CODE deInit();

private:
	bool m_bInit;
	CompVRendererGLPtr m_ptrRenderer;
	CompVProgramPtr m_ptrProgram;
	CompVFBOGLPtr m_ptrCanvasFBO;
};

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

#endif /* _COMPV_DRAWING_SURFACE_GL_H_ */
