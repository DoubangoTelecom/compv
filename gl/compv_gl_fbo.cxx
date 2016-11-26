/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/gl/compv_gl_fbo.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/gl/compv_gl.h"
#include "compv/gl/compv_gl_utils.h"
#include "compv/gl/compv_gl_common.h"
#include "compv/gl/compv_gl_func.h"

COMPV_NAMESPACE_BEGIN()

CompVGLFbo::CompVGLFbo(size_t width, size_t height)
	: CompVObj()
	, CompVBind()
	, m_bInit(false)
	, m_nWidth(width)
	, m_nHeight(height)
	, m_uNameFrameBuffer(0)
	, m_uNameTexture(0)
	, m_uNameDepthStencil(0)
{

}

CompVGLFbo::~CompVGLFbo()
{
	COMPV_CHECK_CODE_ASSERT(deInit());
}

COMPV_OVERRIDE_IMPL0("CompVBind", CompVGLFbo::bind)()
{
	COMPV_CHECK_EXP_RETURN(!CompVGLUtils::isGLContextSet(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);
	COMPV_CHECK_EXP_RETURN(!m_bInit, COMPV_ERROR_CODE_E_INVALID_STATE);

	COMPV_glBindFramebuffer(GL_FRAMEBUFFER, m_uNameFrameBuffer);
	COMPV_glBindRenderbuffer(GL_RENDERBUFFER, m_uNameDepthStencil);

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_OVERRIDE_IMPL0("CompVBind", CompVGLFbo::unbind)()
{
	COMPV_CHECK_EXP_RETURN(!CompVGLUtils::isGLContextSet(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);

	COMPV_glBindFramebuffer(GL_FRAMEBUFFER, kCompVGLNameSystemFrameBuffer);
	COMPV_glBindRenderbuffer(GL_RENDERBUFFER, kCompVGLNameSystemRenderBuffer);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLFbo::updateSize(size_t width, size_t height)
{
	COMPV_CHECK_EXP_RETURN(!CompVGLUtils::isGLContextSet(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);
	
	if (m_nWidth != width && m_nHeight != height) {
		if (m_bInit) {
			GLenum fboStatus_;
			COMPV_glActiveTexture(GL_TEXTURE0);
			COMPV_glBindTexture(GL_TEXTURE_2D, m_uNameTexture);
			COMPV_glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
			COMPV_glBindRenderbuffer(GL_RENDERBUFFER, m_uNameDepthStencil);
#if defined(GL_DEPTH24_STENCIL8)
			COMPV_glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
#elif defined(GL_DEPTH24_STENCIL8_OES)
			COMPV_glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
#else
#	error "Not supported"
#endif
			COMPV_glCheckFramebufferStatus(&fboStatus_, GL_FRAMEBUFFER);
			if (fboStatus_ != GL_FRAMEBUFFER_COMPLETE) {
				COMPV_CHECK_CODE_ASSERT(CompVGLUtils::checkLastError());
				COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_GL);
			}
			m_nWidth = width;
			m_nHeight = height;
		}
	}

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLFbo::init(size_t width, size_t height)
{
	if (m_bInit) {
		return COMPV_ERROR_CODE_S_OK;
	}
	COMPV_CHECK_EXP_RETURN(!width || !height, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_EXP_RETURN(!CompVGLUtils::isGLContextSet(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);
	m_bInit = true; // Set true here to make sure deInit() will work
	COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
	std::string errString_;
	GLenum fboStatus_;

	COMPV_glActiveTexture(GL_TEXTURE0);
	COMPV_CHECK_CODE_BAIL(err_ = CompVGLUtils::textureGen(&m_uNameTexture));
	COMPV_glBindTexture(GL_TEXTURE_2D, m_uNameTexture);
	COMPV_glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	COMPV_glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	COMPV_glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	COMPV_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	COMPV_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	COMPV_glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	// Generate a renderbuffer and use it for both for stencil and depth
	COMPV_CHECK_CODE_BAIL(err_ = CompVGLUtils::renderBufferGen(&m_uNameDepthStencil));
	COMPV_glBindRenderbuffer(GL_RENDERBUFFER, m_uNameDepthStencil);
#if defined(GL_DEPTH24_STENCIL8)
	COMPV_glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
#elif defined(GL_DEPTH24_STENCIL8_OES)
	COMPV_glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
#else
#	error "Not supported"
#endif

	// Generate our Framebuffer object
	COMPV_CHECK_CODE_BAIL(err_ = CompVGLUtils::frameBufferGen(&m_uNameFrameBuffer));
	// Bind to the FBO for next function
	COMPV_glBindFramebuffer(GL_FRAMEBUFFER, m_uNameFrameBuffer);
	// Attach texture to color
	COMPV_glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_uNameTexture, 0);
	// Attach depth buffer
	COMPV_glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_uNameDepthStencil);
	// Attach stencil buffer
	COMPV_glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_uNameDepthStencil);

	// Check FBO status
	COMPV_glCheckFramebufferStatus(&fboStatus_, GL_FRAMEBUFFER);
	if (fboStatus_ != GL_FRAMEBUFFER_COMPLETE) {
		COMPV_CHECK_CODE_BAIL(err_ = CompVGLUtils::checkLastError());
		COMPV_CHECK_CODE_BAIL(err_ = COMPV_ERROR_CODE_E_GL);
	}

	m_nWidth = width;
	m_nHeight = height;

	COMPV_DEBUG_INFO("OpenGL FBO successfully created: FBO_id=%u, TEXT_id=%u, DEPTH_id=%u", m_uNameFrameBuffer, m_uNameTexture, m_uNameDepthStencil);

bail:
	COMPV_glBindFramebuffer(GL_FRAMEBUFFER, 0);
	COMPV_glBindRenderbuffer(GL_RENDERBUFFER, 0);
	COMPV_glBindTexture(GL_TEXTURE_2D, 0);
	if (COMPV_ERROR_CODE_IS_NOK(err_)) {
		COMPV_CHECK_CODE_ASSERT(deInit());
		m_bInit = false;
	}
	return err_;
}

COMPV_ERROR_CODE CompVGLFbo::deInit()
{
	if (!m_bInit) {
		return COMPV_ERROR_CODE_S_OK;
	}
	COMPV_CHECK_EXP_RETURN(!CompVGLUtils::isGLContextSet(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);
	COMPV_CHECK_CODE_ASSERT(CompVGLUtils::textureDelete(&m_uNameTexture));
	COMPV_CHECK_CODE_ASSERT(CompVGLUtils::renderBufferDelete(&m_uNameDepthStencil));
	COMPV_CHECK_CODE_ASSERT(CompVGLUtils::frameBufferDelete(&m_uNameFrameBuffer));
	
	m_bInit = false;

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLFbo::clear()
{
	if (!m_bInit) {
		return COMPV_ERROR_CODE_S_OK;
	}
	COMPV_CHECK_EXP_RETURN(!CompVGLUtils::isGLContextSet(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);
	COMPV_ERROR_CODE err;
	COMPV_CHECK_CODE_BAIL(err = bind());
	COMPV_glClearColor(0.f, 0.f, 0.f, 1.f);
	COMPV_glClearStencil(0);
	COMPV_glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
bail:
	COMPV_CHECK_CODE_ASSERT(unbind());
	return err;
}

COMPV_ERROR_CODE CompVGLFbo::newObj(CompVGLFboPtrPtr fbo, size_t width, size_t height)
{
	COMPV_CHECK_CODE_RETURN(CompVGL::init());
	COMPV_CHECK_EXP_RETURN(!fbo || !width || !height, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	CompVGLFboPtr fbo_ = new CompVGLFbo(width, height);
	COMPV_CHECK_EXP_RETURN(!fbo_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	COMPV_CHECK_CODE_RETURN(fbo_->init(width, height));

	*fbo = fbo_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */
