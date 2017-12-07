/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_GL_SURFACE_LAYER_MULTI_H_)
#define _COMPV_GL_SURFACE_LAYER_MULTI_H_

#include "compv/gl/compv_gl_config.h"
#include "compv/gl/compv_gl_headers.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/base/drawing/compv_surfacelayer_multi.h"
#include "compv/gl/compv_gl_surface.h"

#include <map>

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class CompVGLWindow;

COMPV_OBJECT_DECLARE_PTRS(GLMultiSurfaceLayer)

class CompVGLMultiSurfaceLayer : public CompVMultiSurfaceLayer
{
protected:
    CompVGLMultiSurfaceLayer();
public:
    virtual ~CompVGLMultiSurfaceLayer();
    COMPV_OBJECT_GET_ID(CompVGLMultiSurfaceLayer);

    virtual COMPV_ERROR_CODE addSurface(CompVSurfacePtrPtr surface, size_t width, size_t height, bool activate = true) override /*Overrides(CompVMultiSurfaceLayer)*/;
	virtual COMPV_ERROR_CODE removeSurface(const CompVSurfacePtr surface) override /*Overrides(CompVMultiSurfaceLayer)*/;
	
	virtual CompVSurfacePtr cover() override /*Overrides(CompVSurfaceLayer)*/;
	virtual COMPV_ERROR_CODE blit() override /*Overrides(CompVSurfaceLayer)*/;

    COMPV_ERROR_CODE updateSize(size_t newWidth, size_t newHeight);
    COMPV_ERROR_CODE close();

    static COMPV_ERROR_CODE newObj(CompVGLMultiSurfaceLayerPtrPtr layer, size_t width, size_t height);

private:
    COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
    std::map<compv_surface_id_t, CompVGLSurfacePtr> m_mapSurfaces;
    CompVGLSurfacePtr m_ptrCoverSurfaceGL;
    COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

#endif /* _COMPV_GL_SURFACE_LAYER_MULTI_H_ */
