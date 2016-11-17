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
	CompVSurfaceGL(int width, int height);
public:
	virtual ~CompVSurfaceGL();
	COMPV_GET_OBJECT_ID("CompVSurfaceGL");
	

	// Overrides(CompVSurface)
	virtual bool isGLEnabled()const { return true; };
	virtual COMPV_ERROR_CODE setMVP(CompVMVPPtr mvp);
	virtual COMPV_ERROR_CODE setViewport(CompVViewportPtr viewport);
	virtual COMPV_ERROR_CODE drawImage(CompVMatPtr mat, CompVRendererPtrPtr renderer = NULL);

	COMPV_ERROR_CODE beginDraw();
	COMPV_ERROR_CODE endDraw();

	static COMPV_ERROR_CODE newObj(CompVSurfaceGLPtrPtr glSurface, const CompVWindow* window);

protected:
	// Overrides(CompVCanvas) 
	virtual COMPV_ERROR_CODE canvasBind();
	virtual COMPV_ERROR_CODE canvasUnbind();

private:
	COMPV_INLINE void makeDirty() { m_bDirty = true; }
	COMPV_INLINE void unmakeDirty() { m_bDirty = false; }
	COMPV_INLINE bool isDirty() { return m_bDirty; }
	COMPV_ERROR_CODE init();
	COMPV_ERROR_CODE deInit();

private:
	bool m_bInit;
	bool m_bDirty;
	bool m_bBeginDraw;
	CompVRendererGLPtr m_ptrRenderer;
	CompVProgramPtr m_ptrProgram;
	CompVViewportPtr m_ptrViewport;
};

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

#endif /* _COMPV_DRAWING_SURFACE_GL_H_ */
