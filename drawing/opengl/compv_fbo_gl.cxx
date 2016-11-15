/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/drawing/opengl/compv_fbo_gl.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/drawing/compv_drawing.h"
#include "compv/drawing/opengl/compv_utils_gl.h"

COMPV_NAMESPACE_BEGIN()

CompVFBOGL::CompVFBOGL(size_t width, size_t height)
	: CompVObj()
	, m_bInit(false)
	, m_nWidth(width)
	, m_nHeight(height)
	, m_uNameFrameBuffer(0)
	, m_uNameTexture(0)
	, m_uNameDepthStencil(0)
{

}

CompVFBOGL::~CompVFBOGL()
{
	COMPV_CHECK_CODE_ASSERT(deInit());
}

COMPV_ERROR_CODE CompVFBOGL::bind()
{
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::haveCurrentContext(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);
	COMPV_CHECK_EXP_RETURN(!m_bInit, COMPV_ERROR_CODE_E_INVALID_STATE);

	glBindFramebuffer(GL_FRAMEBUFFER, m_uNameFrameBuffer);

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVFBOGL::unbind()
{
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::haveCurrentContext(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVFBOGL::updateSize(size_t width, size_t height)
{
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::haveCurrentContext(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);

	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVFBOGL::init(size_t width, size_t height)
{
	if (m_bInit) {
		return COMPV_ERROR_CODE_S_OK;
	}
	COMPV_CHECK_EXP_RETURN(!width || !height, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::haveCurrentContext(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);
	m_bInit = true; // Set true here to make sure deInit() will work
	COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
	std::string errString_;
	GLenum fboStatus_;

	glGenTextures(1, &m_uNameTexture);
	if (!m_uNameTexture) {
		COMPV_CHECK_CODE_BAIL(err_ = CompVUtilsGL::checkLastError());
		COMPV_CHECK_CODE_BAIL(err_ = COMPV_ERROR_CODE_E_GL);
	}
	glBindTexture(GL_TEXTURE_2D, m_uNameTexture);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	// Generate a renderbuffer and use it for both for stencil and depth
	glGenRenderbuffers(1, &m_uNameDepthStencil);
	if (!m_uNameDepthStencil) {
		COMPV_CHECK_CODE_BAIL(err_ = CompVUtilsGL::checkLastError());
		COMPV_CHECK_CODE_BAIL(err_ = COMPV_ERROR_CODE_E_GL);
	}
	glBindRenderbuffer(GL_RENDERBUFFER, m_uNameDepthStencil);
#if defined(GL_DEPTH24_STENCIL8)
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
#elif defined(GL_DEPTH24_STENCIL8_OES)
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
#else
#	error "Not supported"
#endif

	// Generate our Framebuffer object
	glGenFramebuffers(1, &m_uNameFrameBuffer);
	if (!m_uNameFrameBuffer) {
		COMPV_CHECK_CODE_BAIL(err_ = CompVUtilsGL::checkLastError());
		COMPV_CHECK_CODE_BAIL(err_ = COMPV_ERROR_CODE_E_GL);
	}

	// Bind to the FBO for next function
	glBindFramebuffer(GL_FRAMEBUFFER, m_uNameFrameBuffer);
	// Attach texture to color
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_uNameTexture, 0);
	// Attach depth buffer
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_uNameDepthStencil);
	// Attach stencil buffer
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_uNameDepthStencil);

	// Check FBO status
	if ((fboStatus_ = glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)) {
		COMPV_CHECK_CODE_BAIL(err_ = CompVUtilsGL::checkLastError());
		COMPV_CHECK_CODE_BAIL(err_ = COMPV_ERROR_CODE_E_GL);
	}

	m_nWidth = width;
	m_nHeight = height;

	COMPV_DEBUG_INFO("OpenGL FBO successfully created: FBO_id=%u, TEXT_id=%u, DEPTH_id=%u", m_uNameFrameBuffer, m_uNameTexture, m_uNameDepthStencil);

bail:
	if (COMPV_ERROR_CODE_IS_NOK(err_)) {
		COMPV_CHECK_CODE_ASSERT(deInit());
		m_bInit = false;
	}
	return err_;
}

COMPV_ERROR_CODE CompVFBOGL::deInit()
{
	if (!m_bInit) {
		return COMPV_ERROR_CODE_S_OK;
	}
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::haveCurrentContext(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);

	if (m_uNameTexture) {
		glDeleteTextures(1, &m_uNameTexture);
		m_uNameTexture = 0;
	}
	if (m_uNameDepthStencil) {
		glDeleteRenderbuffers(1, &m_uNameDepthStencil);
		m_uNameFrameBuffer = 0;
	}
	if (m_uNameFrameBuffer) {
		glDeleteFramebuffers(1, &m_uNameFrameBuffer);
		m_uNameFrameBuffer = 0;
	}
	m_bInit = false;

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVFBOGL::newObj(CompVFBOGLPtrPtr fbo, size_t width, size_t height)
{
	COMPV_CHECK_CODE_RETURN(CompVDrawing::init());
	COMPV_CHECK_EXP_RETURN(!fbo || !width || !height, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	CompVFBOGLPtr fbo_ = new CompVFBOGL(width, height);
	COMPV_CHECK_EXP_RETURN(!fbo_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	COMPV_CHECK_CODE_RETURN(fbo_->init(width, height));

	*fbo = fbo_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */
