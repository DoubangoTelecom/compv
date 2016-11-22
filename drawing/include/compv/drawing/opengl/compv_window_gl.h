/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_DRAWING_OPENGL_WINDOW_GL_H_)
#define _COMPV_DRAWING_OPENGL_WINDOW_GL_H_

#include "compv/base/compv_config.h"
#include "compv/drawing/opengl/compv_headers_gl.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/drawing/opengl/compv_surface_gl.h"
#include "compv/drawing/opengl/compv_context_gl.h"
#include "compv/drawing/opengl/compv_surfacelayer_gl_matching.h"
#include "compv/drawing/opengl/compv_surfacelayer_gl_multi.h"
#include "compv/drawing/opengl/compv_surfacelayer_gl_single.h"
#include "compv/drawing/compv_window.h"
#include "compv/base/compv_obj.h"
#include "compv/base/compv_common.h"
#include "compv/base/parallel/compv_mutex.h"
#include "compv/base/compv_autolock.h"

#include <vector>

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

//
//	CompVWindowGL
//
class CompVWindowGL;
typedef CompVPtr<CompVWindowGL* > CompVWindowGLPtr;
typedef CompVWindowGLPtr* CompVWindowGLPtrPtr;

class CompVWindowGL : public CompVWindowPriv, public CompVLock
{
	friend class CompVWindowGLListener;
protected:
	CompVWindowGL(size_t width, size_t height, const char* title);
public:
	virtual ~CompVWindowGL();

	bool isInitialized()const;

	// Overrides(CompVWindow)
	virtual bool isGLEnabled()const override { return true; }
	virtual COMPV_ERROR_CODE beginDraw()override;
	virtual COMPV_ERROR_CODE endDraw()override;
	virtual COMPV_ERROR_CODE addSingleLayerSurface(CompVSingleSurfaceLayerPtrPtr layer)override;
	virtual COMPV_ERROR_CODE removeSingleLayerSurface(const CompVSingleSurfaceLayerPtr& layer)override;
	virtual COMPV_ERROR_CODE addMatchingLayerSurface(CompVMatchingSurfaceLayerPtrPtr layer)override;
	virtual COMPV_ERROR_CODE removeMatchingLayerSurface(const CompVMatchingSurfaceLayerPtr& layer)override;
	virtual COMPV_ERROR_CODE addMultiLayerSurface(CompVMultiSurfaceLayerPtrPtr layer) override;
	virtual COMPV_ERROR_CODE removeMultiLayerSurface(const CompVMultiSurfaceLayerPtr& layer) override;

	// Overrides(CompVWindowPriv)
	virtual COMPV_ERROR_CODE priv_updateSize(size_t newWidth, size_t newHeight);

protected:
	virtual CompVContextGLPtr context() = 0;

private:
	bool m_bDrawing;
	std::map<compv_surfacelayer_id_t, CompVSingleSurfaceLayerGLPtr> m_mapSingleSurfaceLayers;
	std::map<compv_surfacelayer_id_t, CompVMatchingSurfaceLayerGLPtr> m_mapMatchingSurfaceLayers;
	std::map<compv_surfacelayer_id_t, CompVMultiSurfaceLayerGLPtr> m_mapMultiSurfaceLayers;
};

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

#endif /* _COMPV_DRAWING_OPENGL_WINDOW_GL_H_ */
