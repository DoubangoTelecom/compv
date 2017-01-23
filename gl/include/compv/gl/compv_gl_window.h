/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_GL_WINDOW_H_)
#define _COMPV_GL_WINDOW_H_

#include "compv/gl/compv_gl_config.h"
#include "compv/gl/compv_gl_headers.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/base/drawing/compv_window.h"
#include "compv/base/compv_lock.h"
#include "compv/gl/compv_gl_surface.h"
#include "compv/gl/compv_gl_surfacelayer_matching.h"
#include "compv/gl/compv_gl_surfacelayer_multi.h"
#include "compv/gl/compv_gl_surfacelayer_single.h"
#include "compv/gl/compv_gl_context.h"

#include <vector>

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

//
//	CompVGLWindow
//
COMPV_OBJECT_DECLARE_PTRS(GLWindow)

class COMPV_GL_API CompVGLWindow : public CompVWindowPriv, public CompVLock
{
    friend class CompVGLWindowListener;
protected:
    CompVGLWindow(size_t width, size_t height, const char* title);
public:
    virtual ~CompVGLWindow();

    bool isInitialized() const;

    // Overrides(CompVWindow)
	virtual bool isGLEnabled() const override /*Overrides(CompVWindow)*/;
	virtual COMPV_ERROR_CODE close() override /*Overrides(CompVWindow)*/;
	virtual COMPV_ERROR_CODE beginDraw() override /*Overrides(CompVWindow)*/;
	virtual COMPV_ERROR_CODE endDraw() override /*Overrides(CompVWindow)*/;
	virtual COMPV_ERROR_CODE addSingleLayerSurface(CompVSingleSurfaceLayerPtrPtr layer) override /*Overrides(CompVWindow)*/;
	virtual COMPV_ERROR_CODE removeSingleLayerSurface(const CompVSingleSurfaceLayerPtr& layer) override /*Overrides(CompVWindow)*/;
	virtual COMPV_ERROR_CODE addMatchingLayerSurface(CompVMatchingSurfaceLayerPtrPtr layer) override /*Overrides(CompVWindow)*/;
	virtual COMPV_ERROR_CODE removeMatchingLayerSurface(const CompVMatchingSurfaceLayerPtr& layer) override /*Overrides(CompVWindow)*/;
	virtual COMPV_ERROR_CODE addMultiLayerSurface(CompVMultiSurfaceLayerPtrPtr layer) override /*Overrides(CompVWindow)*/;
	virtual COMPV_ERROR_CODE removeMultiLayerSurface(const CompVMultiSurfaceLayerPtr& layer) override /*Overrides(CompVWindow)*/;

	virtual COMPV_ERROR_CODE priv_updateSize(size_t newWidth, size_t newHeight) override /*Overrides(CompVWindowPriv)*/;
	virtual COMPV_ERROR_CODE priv_updateState(COMPV_WINDOW_STATE newState) override /*Overrides(CompVWindowPriv)*/;

protected:
    virtual CompVGLContextPtr context() = 0;

private:
    COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
    bool m_bDrawing;
    std::map<compv_surfacelayer_id_t, CompVGLSingleSurfaceLayerPtr> m_mapSingleSurfaceLayers;
    std::map<compv_surfacelayer_id_t, CompVGLMatchingSurfaceLayerPtr> m_mapMatchingSurfaceLayers;
    std::map<compv_surfacelayer_id_t, CompVGLMultiSurfaceLayerPtr> m_mapMultiSurfaceLayers;
    COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

#endif /* _COMPV_GL_WINDOW_H_ */
