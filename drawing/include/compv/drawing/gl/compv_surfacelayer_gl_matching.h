/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_DRAWING_SURFACE_LAYER_GL_MATCHING_H_)
#define _COMPV_DRAWING_SURFACE_LAYER_GL_MATCHING_H_

#include "compv/drawing/compv_config.h"
#include "compv/gl/compv_gl_headers.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/drawing/compv_surfacelayer_matching.h"
#include "compv/drawing/gl/compv_surface_gl.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class CompVMatchingSurfaceLayerGL;
typedef CompVPtr<CompVMatchingSurfaceLayerGL* > CompVMatchingSurfaceLayerGLPtr;
typedef CompVMatchingSurfaceLayerGLPtr* CompVMatchingSurfaceLayerGLPtrPtr;

class CompVMatchingSurfaceLayerGL : public CompVMatchingSurfaceLayer
{
protected:
	CompVMatchingSurfaceLayerGL();
public:
	virtual ~CompVMatchingSurfaceLayerGL();
	COMPV_GET_OBJECT_ID(CompVMatchingSurfaceLayerGL);
	
	COMPV_OVERRIDE_DECL0("CompVMatchingSurfaceLayer", drawMatches)(CompVMatPtr trainImage, CompVMatPtr queryImage) override;
	COMPV_OVERRIDE_DECL0("CompSurfaceLayer", blit)() override;

	COMPV_ERROR_CODE updateSize(size_t newWidth, size_t newHeight);

	static COMPV_ERROR_CODE newObj(CompVMatchingSurfaceLayerGLPtrPtr layer, size_t width, size_t height);

private:
	CompVSurfaceGLPtr m_ptrCoverSurfaceGL;
	CompVSurfaceGLPtr m_ptrTrainSurfaceGL;
	CompVSurfaceGLPtr m_ptrQuerySurfaceGL;
};

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

#endif /* _COMPV_DRAWING_SURFACE_LAYER_GL_MATCHING_H_ */
