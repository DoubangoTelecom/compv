/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_GL_SURFACE_LAYER_MATCHING_H_)
#define _COMPV_GL_SURFACE_LAYER_MATCHING_H_

#include "compv/gl/compv_gl_config.h"
#include "compv/gl/compv_gl_headers.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/base/drawing/compv_surfacelayer_matching.h"
#include "compv/gl/compv_gl_surface.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

COMPV_OBJECT_DECLARE_PTRS(GLMatchingSurfaceLayer)

class CompVGLMatchingSurfaceLayer : public CompVMatchingSurfaceLayer
{
protected:
    CompVGLMatchingSurfaceLayer();
public:
    virtual ~CompVGLMatchingSurfaceLayer();
    COMPV_OBJECT_GET_ID(CompVGLMatchingSurfaceLayer);

	virtual COMPV_ERROR_CODE drawMatches(const CompVMatPtr& trainImage, const CompVMatPtr& trainGoodMatches, const CompVMatPtr& queryImage, const CompVMatPtr& queryGoodMatches) override /*Overrides(CompVMatchingSurfaceLayer)*/;

	virtual COMPV_ERROR_CODE blit() override /*Overries(CompVSurfaceLayer)*/;

    COMPV_ERROR_CODE updateSize(size_t newWidth, size_t newHeight);
    COMPV_ERROR_CODE close();

    static COMPV_ERROR_CODE newObj(CompVGLMatchingSurfaceLayerPtrPtr layer, size_t width, size_t height);

private:
	virtual COMPV_ERROR_CODE drawMatches(const compv_float64_t* trainX, const compv_float64_t* trainY, const compv_float64_t* queryX, const compv_float64_t* queryY, size_t count);
	virtual COMPV_ERROR_CODE drawMatches(const compv_float32_t* trainX, const compv_float32_t* trainY, const compv_float32_t* queryX, const compv_float32_t* queryY, size_t count);

private:
    CompVGLSurfacePtr m_ptrCoverSurfaceGL;
    CompVGLSurfacePtr m_ptrTrainSurfaceGL;
    CompVGLSurfacePtr m_ptrQuerySurfaceGL;
};


COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

#endif /* _COMPV_GL_SURFACE_LAYER_MATCHING_H_ */
