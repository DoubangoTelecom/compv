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
    COMPV_OVERRIDE_DECL1("CompVWindow", bool, isGLEnabled)() const override;
    COMPV_OVERRIDE_DECL0("CompVWindow", close)() override;
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
