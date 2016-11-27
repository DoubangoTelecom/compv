/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_GL_RENDERER_RGB_H_)
#define _COMPV_GL_RENDERER_RGB_H_

#include "compv/gl/compv_config.h"
#include "compv/gl/compv_gl_headers.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/gl/compv_gl_renderer.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class CompVGLRendererRGB;
typedef CompVPtr<CompVGLRendererRGB* > CompVGLRendererRGBPtr;
typedef CompVGLRendererRGBPtr* CompVGLRendererRGBPtrPtr;

class CompVGLRendererRGB : public CompVGLRenderer
{
protected:
	CompVGLRendererRGB(COMPV_PIXEL_FORMAT eRGBPixelFormat);
public:
	virtual ~CompVGLRendererRGB();
	COMPV_GET_OBJECT_ID(CompVGLRendererRGB);
	
	COMPV_OVERRIDE_DECL0("CompVGLRenderer", drawImage)(CompVMatPtr mat) override;

	static COMPV_ERROR_CODE newObj(CompVGLRendererRGBPtrPtr glRgbRenderer, COMPV_PIXEL_FORMAT eRGBPixelFormat);

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

#endif /* _COMPV_GL_RENDERER_RGB_H_ */
