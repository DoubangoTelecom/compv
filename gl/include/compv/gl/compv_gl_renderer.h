/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_GL_RENDERER_PLANAR_H_)
#define _COMPV_GL_RENDERER_PLANAR_H_

#include "compv/gl/compv_gl_config.h"
#include "compv/gl/compv_gl_headers.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/base/drawing/compv_renderer.h"
#include "compv/base/drawing/compv_viewport.h"
#include "compv/gl/compv_gl_canvas.h"
#include "compv/gl/compv_gl_blitter.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

COMPV_OBJECT_DECLARE_PTRS(GLRenderer)

class CompVGLRenderer : public CompVRenderer
{
protected:
    CompVGLRenderer(COMPV_SUBTYPE ePixelFormat);
public:
    virtual ~CompVGLRenderer();
    COMPV_OBJECT_GET_ID(CompVGLRenderer);

	COMPV_INLINE CompVGLBlitterPtr blitter() {
		return m_ptrBlitter;
	}

	virtual bool isGLEnabled()const override /*Overrides(CompVGLRenderer)*/ {
		return true;
	};
	virtual CompVCanvasPtr canvas() override /*Overrides(CompVGLRenderer)*/;
    virtual COMPV_ERROR_CODE drawImage(const CompVMatPtr& mat, const CompVViewportPtr& viewport = nullptr) override /*Overrides(CompVGLRenderer)*/;

	COMPV_ERROR_CODE close();

    static COMPV_ERROR_CODE newObj(CompVGLRendererPtrPtr glRenderer, COMPV_SUBTYPE ePixelFormat);

private:
    COMPV_ERROR_CODE deInit();
    COMPV_ERROR_CODE init(const CompVMatPtr mat);

private:
    COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
    bool m_bInit;
	GLenum m_eFormats[COMPV_PLANE_MAX_COUNT];
	GLenum m_ePixelDataType;
	const char* m_pSamplerNames[COMPV_PLANE_MAX_COUNT];
    GLuint m_uNameTextures[COMPV_PLANE_MAX_COUNT];
    size_t m_uWidths[COMPV_PLANE_MAX_COUNT];
    size_t m_uHeights[COMPV_PLANE_MAX_COUNT];
    size_t m_uStrides[COMPV_PLANE_MAX_COUNT];
	GLsizei m_uWidthsTexture[COMPV_PLANE_MAX_COUNT];
	GLsizei m_uHeightsTexture[COMPV_PLANE_MAX_COUNT];
	GLsizei m_uStridesTexture[COMPV_PLANE_MAX_COUNT];
    size_t m_uTexturesCount;
    std::string m_strPrgVertexData;
    std::string m_strPrgFragData;
	CompVGLCanvasPtr m_ptrCanvas;
	CompVGLBlitterPtr m_ptrBlitter;
    COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

#endif /* _COMPV_GL_RENDERER_PLANAR_H_ */
