/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_DRAWING_RENDERER_GL_YUV_H_)
#define _COMPV_DRAWING_RENDERER_GL_YUV_H_

#include "compv/base/compv_config.h"
#include "compv/drawing/opengl/compv_headers_gl.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/drawing/opengl/compv_renderer_gl.h"
#include "compv/drawing/opengl/compv_consts_gl.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_obj.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class CompVRendererGLYUV;
typedef CompVPtr<CompVRendererGLYUV* > CompVRendererGLYUVPtr;
typedef CompVRendererGLYUVPtr* CompVRendererGLYUVPtrPtr;

class CompVRendererGLYUV : public CompVRendererGL
{
protected:
	CompVRendererGLYUV(COMPV_PIXEL_FORMAT eYUVPixelFormat);
public:
	virtual ~CompVRendererGLYUV();
	COMPV_GET_OBJECT_ID("CompVRendererGLYUV");

	// Overrides(CompVRenderer)
	virtual bool isGLEnabled()const { return true; };

	// Overrides(CompVRendererGL)
	virtual COMPV_ERROR_CODE drawImage(CompVMatPtr mat);

	static COMPV_ERROR_CODE newObj(CompVRendererGLYUVPtrPtr glRenderer, COMPV_PIXEL_FORMAT eYUVPixelFormat);

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

#endif /* _COMPV_DRAWING_RENDERER_GL_YUV_H_ */
