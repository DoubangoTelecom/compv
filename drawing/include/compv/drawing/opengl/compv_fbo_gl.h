/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_DRAWING_OPENGL_FBO_H_)
#define _COMPV_DRAWING_OPENGL_FBO_H_

#include "compv/drawing/compv_config.h"
#include "compv/gl/compv_gl_headers.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/base/compv_obj.h"
#include "compv/drawing/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class CompVFBOGL;
typedef CompVPtr<CompVFBOGL* > CompVFBOGLPtr;
typedef CompVFBOGLPtr* CompVFBOGLPtrPtr;

class CompVFBOGL : public CompVObj
{
protected:
	CompVFBOGL(size_t width, size_t height);
public:
	virtual ~CompVFBOGL();
	COMPV_GET_OBJECT_ID(CompVFBOGL);

	COMPV_INLINE size_t width() { return m_nWidth; }
	COMPV_INLINE size_t height() { return m_nHeight; }
	COMPV_INLINE GLuint nameFrameBuffer() { return m_uNameFrameBuffer; }
	COMPV_INLINE GLuint nameTexture() { return m_uNameTexture; }
	COMPV_INLINE GLuint nameDepthStencil() { return m_uNameDepthStencil; }
	
	virtual COMPV_ERROR_CODE bind()const;
	virtual COMPV_ERROR_CODE unbind()const;
	virtual COMPV_ERROR_CODE updateSize(size_t width, size_t height);

	COMPV_ERROR_CODE clear();

	static COMPV_ERROR_CODE newObj(CompVFBOGLPtrPtr fbo, size_t width, size_t height);

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

static const CompVFBOGLPtr kCompVGLPtrSystemFrameBuffer = NULL;

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

#endif /* _COMPV_DRAWING_OPENGL_FBO_H_ */
