/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_GL_RENDERER_YUV_H_)
#define _COMPV_GL_RENDERER_YUV_H_

#include "compv/gl/compv_gl_config.h"
#include "compv/gl/compv_gl_headers.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/gl/compv_gl_renderer.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

COMPV_OBJECT_DECLARE_PTRS(GLRendererYUV)

class CompVGLRendererYUV : public CompVGLRenderer
{
protected:
    CompVGLRendererYUV(COMPV_PIXEL_FORMAT eYUVPixelFormat);
public:
    virtual ~CompVGLRendererYUV();
    COMPV_OBJECT_GET_ID(CompVGLRendererYUV);

    virtual COMPV_ERROR_CODE drawImage(CompVMatPtr mat) COMPV_OVERRIDE_DECL("CompVGLRenderer");

    static COMPV_ERROR_CODE newObj(CompVGLRendererYUVPtrPtr glRenderer, COMPV_PIXEL_FORMAT eYUVPixelFormat);

protected:
    virtual COMPV_ERROR_CODE deInit();
    virtual COMPV_ERROR_CODE init(CompVMatPtr mat);

private:
    COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
    bool m_bInit;
    GLuint m_uNameTextures[4];
    size_t m_uWidths[4];
    size_t m_uHeights[4];
    size_t m_uStrides[4];
    size_t m_uTexturesCount;
    std::string m_strPrgVertexData;
    std::string m_strPrgFragData;
    COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

#endif /* _COMPV_GL_RENDERER_YUV_H_ */
