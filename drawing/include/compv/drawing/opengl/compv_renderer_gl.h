/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_DRAWING_RENDERER_GL_H_)
#define _COMPV_DRAWING_RENDERER_GL_H_

#include "compv/drawing/compv_config.h"
#include "compv/gl/compv_gl_headers.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/drawing/compv_renderer.h"
#include "compv/drawing/opengl/compv_blitter_gl.h"
#include "compv/drawing/opengl/compv_consts_gl.h"
#include "compv/drawing/opengl/compv_fbo_gl.h"
#include "compv/drawing/compv_program.h"
#include "compv/drawing/compv_common.h"
#include "compv/base/compv_obj.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class CompVRendererGL;
typedef CompVPtr<CompVRendererGL* > CompVRendererGLPtr;
typedef CompVRendererGLPtr* CompVRendererGLPtrPtr;

class CompVRendererGL : public CompVRenderer, public CompVBlitterGL
{
protected:
	CompVRendererGL(COMPV_PIXEL_FORMAT ePixelFormat);
public:
	virtual ~CompVRendererGL();
	COMPV_GET_OBJECT_ID(CompVRendererGL);

	// Overrides(CompVRenderer)
	virtual bool isGLEnabled()const { return true; };

	virtual COMPV_ERROR_CODE drawImage(CompVMatPtr mat) = 0;

	CompVFBOGLPtr fbo() { return m_ptrFBO; }

	static COMPV_ERROR_CODE newObj(CompVRendererGLPtrPtr glRenderer, COMPV_PIXEL_FORMAT ePixelFormat);
	
protected:
	// Overrides(CompVCanvas) 
	virtual COMPV_ERROR_CODE canvasBind();
	virtual COMPV_ERROR_CODE canvasUnbind();

	virtual COMPV_ERROR_CODE deInit();
	virtual COMPV_ERROR_CODE init(CompVMatPtr mat, const std::string& prgVertexData, const std::string& prgFragData, bool bMVP = false, bool bToScreen = false);
	virtual COMPV_ERROR_CODE bind();
	virtual COMPV_ERROR_CODE unbind();

private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
	bool m_bInit;
	CompVFBOGLPtr m_ptrFBO;
	COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

#endif /* _COMPV_DRAWING_RENDERER_GL_H_ */