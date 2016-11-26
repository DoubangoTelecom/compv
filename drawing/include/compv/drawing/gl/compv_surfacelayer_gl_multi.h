/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_DRAWING_SURFACE_LAYER_GL_MULTI_H_)
#define _COMPV_DRAWING_SURFACE_LAYER_GL_MULTI_H_

#include "compv/drawing/compv_config.h"
#include "compv/gl/compv_gl_headers.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/drawing/compv_surfacelayer_multi.h"
#include "compv/drawing/gl/compv_surface_gl.h"

#include <map>

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class CompVWindowGL;

class CompVMultiSurfaceLayerGL;
typedef CompVPtr<CompVMultiSurfaceLayerGL* > CompVMultiSurfaceLayerGLPtr;
typedef CompVMultiSurfaceLayerGLPtr* CompVMultiSurfaceLayerGLPtrPtr;

class CompVMultiSurfaceLayerGL : public CompVMultiSurfaceLayer
{
protected:
	CompVMultiSurfaceLayerGL();
public:
	virtual ~CompVMultiSurfaceLayerGL();
	COMPV_GET_OBJECT_ID(CompVMultiSurfaceLayerGL);
	
	COMPV_OVERRIDE_DECL0("CompVMultiSurfaceLayer", addSurface)(CompVSurfacePtrPtr surface, size_t width, size_t height) override;
	COMPV_OVERRIDE_DECL0("CompVMultiSurfaceLayer", removeSurface)(const CompVSurfacePtr surface) override;
	
	COMPV_OVERRIDE_DECL0("CompSurfaceLayer", blit)() override;

	COMPV_ERROR_CODE updateSize(size_t newWidth, size_t newHeight);

	static COMPV_ERROR_CODE newObj(CompVMultiSurfaceLayerGLPtrPtr layer, size_t width, size_t height);

private:
	COMPV_ERROR_CODE initFBO();

private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
	std::map<compv_surface_id_t, CompVSurfaceGLPtr> m_mapSurfaces;
	CompVSurfaceGLPtr m_ptrCoverSurfaceGL;
	CompVGLFboPtr m_ptrFBO;
	COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

#endif /* _COMPV_DRAWING_SURFACE_LAYER_GL_MULTI_H_ */
