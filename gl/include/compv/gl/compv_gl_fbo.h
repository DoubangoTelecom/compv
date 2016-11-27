/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_GL_FBO_H_)
#define _COMPV_GL_FBO_H_

#include "compv/gl/compv_config.h"
#include "compv/gl/compv_gl_headers.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/base/compv_obj.h"
#include "compv/base/compv_bind.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

#define COMPV_GL_FBO_AUTOBIND(fbo) COMPV_AUTOBIND(CompVGLFbo, (fbo))

class CompVGLFbo;
typedef CompVPtr<CompVGLFbo* > CompVGLFboPtr;
typedef CompVGLFboPtr* CompVGLFboPtrPtr;

class COMPV_GL_API CompVGLFbo : public CompVObj, public CompVBind
{
protected:
	CompVGLFbo(size_t width, size_t height);
public:
	virtual ~CompVGLFbo();
	COMPV_GET_OBJECT_ID(CompVGLFbo);

	COMPV_INLINE size_t width() { return m_nWidth; }
	COMPV_INLINE size_t height() { return m_nHeight; }
	COMPV_INLINE GLuint nameFrameBuffer() { return m_uNameFrameBuffer; }
	COMPV_INLINE GLuint nameTexture() { return m_uNameTexture; }
	COMPV_INLINE GLuint nameDepthStencil() { return m_uNameDepthStencil; }

	COMPV_OVERRIDE_DECL0("CompVBind", bind)() override;
	COMPV_OVERRIDE_DECL0("CompVBind", unbind)() override;
	virtual COMPV_ERROR_CODE updateSize(size_t width, size_t height);

	COMPV_ERROR_CODE clear();

	COMPV_ERROR_CODE close();

	static COMPV_ERROR_CODE newObj(CompVGLFboPtrPtr fbo, size_t width, size_t height);

protected:
	virtual COMPV_ERROR_CODE init(size_t width, size_t height);
	virtual COMPV_ERROR_CODE deInit();

private:
	bool m_bInit;
	size_t m_nWidth;
	size_t m_nHeight;
	GLuint m_uNameFrameBuffer;
	GLuint m_uNameTexture;
	GLuint m_uNameDepthStencil;
};

static const CompVGLFboPtr kCompVGLPtrSystemFrameBuffer = NULL;

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

#endif /* _COMPV_GL_FBO_H_ */
