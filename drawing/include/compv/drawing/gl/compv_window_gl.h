/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_DRAWING_OPENGL_WINDOW_GL_H_)
#define _COMPV_DRAWING_OPENGL_WINDOW_GL_H_

#include "compv/drawing/compv_config.h"
#include "compv/gl/compv_gl_headers.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/drawing/gl/compv_surface_gl.h"
#include "compv/drawing/gl/compv_surfacelayer_gl_matching.h"
#include "compv/drawing/gl/compv_surfacelayer_gl_multi.h"
#include "compv/drawing/gl/compv_surfacelayer_gl_single.h"
#include "compv/drawing/compv_window.h"
#include "compv/base/compv_obj.h"
#include "compv/drawing/compv_common.h"
#include "compv/base/parallel/compv_mutex.h"
#include "compv/base/compv_lock.h"
#include "compv/gl/compv_gl_context.h"

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

	bool isInitialized() const;

	// Overrides(CompVWindow)
	COMPV_OVERRIDE_DECL1("CompVWindow", bool, isGLEnabled)() const override;
	COMPV_OVERRIDE_DECL0("CompVWindow", beginDraw)() override;
	COMPV_OVERRIDE_DECL0("CompVWindow", endDraw)() override;
	COMPV_OVERRIDE_DECL0("CompVWindow", addSingleLayerSurface)(CompVSingleSurfaceLayerPtrPtr layer) override;
	COMPV_OVERRIDE_DECL0("CompVWindow", removeSingleLayerSurface)(const CompVSingleSurfaceLayerPtr& layer) override;
	COMPV_OVERRIDE_DECL0("CompVWindow", addMatchingLayerSurface)(CompVMatchingSurfaceLayerPtrPtr layer) override;
	COMPV_OVERRIDE_DECL0("CompVWindow", removeMatchingLayerSurface)(const CompVMatchingSurfaceLayerPtr& layer) override;
	COMPV_OVERRIDE_DECL0("CompVWindow", addMultiLayerSurface)(CompVMultiSurfaceLayerPtrPtr layer) override;
	COMPV_OVERRIDE_DECL0("CompVWindow", removeMultiLayerSurface)(const CompVMultiSurfaceLayerPtr& layer) override;

	COMPV_OVERRIDE_DECL0("CompVWindowPriv", priv_updateSize)(size_t newWidth, size_t newHeight)override;

protected:
	virtual CompVGLContextPtr context() = 0;

private:
	bool m_bDrawing;
	std::map<compv_surfacelayer_id_t, CompVSingleSurfaceLayerGLPtr> m_mapSingleSurfaceLayers;
	std::map<compv_surfacelayer_id_t, CompVMatchingSurfaceLayerGLPtr> m_mapMatchingSurfaceLayers;
	std::map<compv_surfacelayer_id_t, CompVMultiSurfaceLayerGLPtr> m_mapMultiSurfaceLayers;
};

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

#endif /* _COMPV_DRAWING_OPENGL_WINDOW_GL_H_ */
