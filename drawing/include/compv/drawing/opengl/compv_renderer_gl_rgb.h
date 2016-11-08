/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_DRAWING_RENDERER_GL_RGB_H_)
#define _COMPV_DRAWING_RENDERER_GL_RGB_H_

#include "compv/base/compv_config.h"
#include "compv/drawing/opengl/compv_headers_gl.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/base/compv_common.h"
#include "compv/base/compv_obj.h"
#include "compv/drawing/opengl/compv_renderer_gl.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class CompVRendererGLRgb;
typedef CompVPtr<CompVRendererGLRgb* > CompVRendererGLRgbPtr;
typedef CompVRendererGLRgbPtr* CompVRendererGLRgbPtrPtr;

class CompVRendererGLRgb : public CompVRendererGL
{
protected:
	CompVRendererGLRgb(COMPV_PIXEL_FORMAT ePixelFormat);
public:
	virtual ~CompVRendererGLRgb();
	virtual COMPV_INLINE const char* getObjectId() {
		return "CompVRendererGLRgb";
	};

	virtual COMPV_ERROR_CODE render(CompVMatPtr mat);

	static COMPV_ERROR_CODE newObj(CompVRendererGLRgbPtrPtr glRgbRenderer, COMPV_PIXEL_FORMAT ePixelFormat, const CompVSurface* surface);

private:
	virtual COMPV_ERROR_CODE blit();

private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
	GLuint m_uNameTexture;
	// FIXME(dmi): move to base class
	size_t m_uWidth;
	size_t m_uHeight;
	size_t m_uStride;
	COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

#endif /* _COMPV_DRAWING_RENDERER_GL_RGB_H_ */
