/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_DRAWING_RENDERER_GL_RGB_H_)
#define _COMPV_DRAWING_RENDERER_GL_RGB_H_

#include "compv/drawing/compv_config.h"
#include "compv/gl/compv_gl_headers.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/drawing/compv_common.h"
#include "compv/base/compv_obj.h"
#include "compv/drawing/gl/compv_renderer_gl.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class CompVRendererGLRGB;
typedef CompVPtr<CompVRendererGLRGB* > CompVRendererGLRGBPtr;
typedef CompVRendererGLRGBPtr* CompVRendererGLRGBPtrPtr;

class CompVRendererGLRGB : public CompVRendererGL
{
protected:
	CompVRendererGLRGB(COMPV_PIXEL_FORMAT eRGBPixelFormat);
public:
	virtual ~CompVRendererGLRGB();
	COMPV_GET_OBJECT_ID(CompVRendererGLRGB);

	// Overrides(CompVRendererGL)
	virtual COMPV_ERROR_CODE drawImage(CompVMatPtr mat);

	static COMPV_ERROR_CODE newObj(CompVRendererGLRGBPtrPtr glRgbRenderer, COMPV_PIXEL_FORMAT eRGBPixelFormat);

protected:
	virtual COMPV_ERROR_CODE deInit();
	virtual COMPV_ERROR_CODE init(CompVMatPtr mat);

private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
	bool m_bInit;
	GLint m_iFormat;
	GLuint m_uNameTexture;
	GLuint m_uNameSampler;
	size_t m_uWidth;
	size_t m_uHeight;
	size_t m_uStride;
	std::string m_strPrgVertexData;
	std::string m_strPrgFragData;
	COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

#endif /* _COMPV_DRAWING_RENDERER_GL_RGB_H_ */
