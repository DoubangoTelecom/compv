/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_DRAWING_SURFACE_LAYER_GL_SINGLE_H_)
#define _COMPV_DRAWING_SURFACE_LAYER_GL_SINGLE_H_

#include "compv/base/compv_config.h"
#include "compv/drawing/opengl/compv_headers_gl.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/drawing/compv_surfacelayer_single.h"
#include "compv/drawing/opengl/compv_surface_gl.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class CompVWindowGL;

class CompVSingleSurfaceLayerGL;
typedef CompVPtr<CompVSingleSurfaceLayerGL* > CompVSingleSurfaceLayerGLPtr;
typedef CompVSingleSurfaceLayerGLPtr* CompVSingleSurfaceLayerGLPtrPtr;

class CompVSingleSurfaceLayerGL : public CompVSingleSurfaceLayer
{
protected:
	CompVSingleSurfaceLayerGL();
public:
	virtual ~CompVSingleSurfaceLayerGL();
	COMPV_GET_OBJECT_ID("CompVSingleSurfaceLayerGL");

	// Overrides(CompVSingleSurfaceLayer)
	virtual CompVSurfacePtr surface() override;

	// Overrides(CompSurfaceLayer)
	virtual COMPV_ERROR_CODE blit() override;

	COMPV_ERROR_CODE updateSize(size_t newWidth, size_t newHeight);

	static COMPV_ERROR_CODE newObj(CompVSingleSurfaceLayerGLPtrPtr layer, size_t width, size_t height);

private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
		CompVSurfaceGLPtr m_ptrSurfaceGL;
	COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

#endif /* _COMPV_DRAWING_SURFACE_LAYER_GL_SINGLE_H_ */
