/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/drawing/opengl/compv_surface_gl.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/drawing/compv_drawing.h"
#include "compv/drawing/compv_window.h"
#include "compv/drawing/compv_program.h"
#include "compv/drawing/compv_canvas.h"
#include "compv/drawing/opengl/compv_consts_gl.h"
#include "compv/drawing/opengl/compv_utils_gl.h"

// FIXME: OpenGL error handling not ok, impossible to find which function cause the error (erros stacked)

// FIXME(dmi): remove
#if defined(main)
#error "main must not be defined"
#endif
static const char kShaderVertex[] = COMPV_STRING(
	attribute vec4 position;
	attribute vec2 texCoord;
	varying vec2 texCoordVarying;
	void main() {
		gl_Position = position;
		texCoordVarying = texCoord;
	}
);
static const char kShaderFragment[] = COMPV_STRING(
	//precision mediump float; // TODO(dmi): should be GLES only
	varying vec2 texCoordVarying;
	uniform sampler2D SamplerRGBA;
	void main() {
		gl_FragColor = texture2D(SamplerRGBA, texCoordVarying).rgba;
	}
);

COMPV_NAMESPACE_BEGIN()

CompVSurfaceGL::CompVSurfaceGL(int width, int height)
	: CompVSurface(width, height)
	, CompVSurfaceBlit()
	, m_uNameFrameBuffer(0)
	, m_uNameTexture(0)
	, m_uNameDepthStencil(0)
	, m_uNameVertexBuffer(0)
	, m_uNameIndiceBuffer(0)
	, m_uNameSlotPosition(0)
	, m_uNameSlotTexCoord(0)
#if defined(HAVE_OPENGL) // FIXME
	, m_uNameVAO(0)
#endif
{

}

CompVSurfaceGL::~CompVSurfaceGL()
{
	COMPV_CHECK_CODE_ASSERT(deInitProgram());
	COMPV_CHECK_CODE_ASSERT(deInitFrameBuffer());
}

COMPV_ERROR_CODE CompVSurfaceGL::drawImage(CompVMatPtr mat)
{
#if 1
	COMPV_CHECK_EXP_RETURN(!mat, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_CODE_RETURN(initFrameBuffer());
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::haveCurrentContext(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);

	// FIXME: remove if 'm_uNameFrameBuffer' is passed as parameter
	glBindFramebuffer(GL_FRAMEBUFFER, m_uNameFrameBuffer); // Draw to framebuffer
	
	// FIXME: Make sure Renderer has different format than mat and print perf issue message
	if (!m_ptrRenderer) { // FIXME: check chroma
		COMPV_CHECK_CODE_RETURN(CompVRenderer::newObj(&m_ptrRenderer, static_cast<COMPV_PIXEL_FORMAT>(mat->subType()), this));
	}
	COMPV_CHECK_CODE_RETURN(m_ptrRenderer->render(mat));

#if 0
	glBindFramebuffer(GL_FRAMEBUFFER, m_uNameFrameBuffer);
	uint8_t* data = (uint8_t*)malloc(640 * 480 * 4);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glReadPixels(0, 0, 640, 480, GL_RGBA, GL_UNSIGNED_BYTE, data);
	FILE* file = fopen("C:/Projects/image.rgba", "wb+");
	fwrite(data, 1, (640 * 480 * 4), file);
	fclose(file);
	free(data);
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // System's framebuffer
#endif

	// FIXME: remove if 'm_uNameFrameBuffer' is passed as parameter
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // Draw to system

#endif

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVSurfaceGL::drawText(const void* textPtr, size_t textLengthInBytes)
{
#if 1
	COMPV_CHECK_EXP_RETURN(!textPtr || !textLengthInBytes, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::haveCurrentContext(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);
	COMPV_CHECK_CODE_RETURN(initFrameBuffer());
	
	/*glPushAttrib(
		//GL_VIEWPORT_BIT
		//| GL_TRANSFORM_BIT
		// GL_TEXTURE_BIT
		//| GL_STENCIL_BUFFER_BIT
		//| GL_SCISSOR_BIT
		//| GL_POLYGON_STIPPLE_BIT
		//| GL_POLYGON_BIT
		//| GL_POINT_BIT
		//| GL_PIXEL_MODE_BIT
		//| GL_MULTISAMPLE_BIT
		//| GL_LIST_BIT
		//| GL_LINE_BIT
		//| GL_LIGHTING_BIT
		//| GL_HINT_BIT
		//| GL_FOG_BIT
		//| GL_EVAL_BIT
		//| GL_ENABLE_BIT
		 GL_DEPTH_BUFFER_BIT
		//| GL_CURRENT_BIT
		//| GL_COLOR_BUFFER_BIT
		//| GL_ACCUM_BUFFER_BIT
	);*/
	//glPushAttrib(GL_ALL_ATTRIB_BITS);
	//glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
	//glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT /*| GL_CLIENT_VERTEX_ARRAY_BIT*/);

	glBindFramebuffer(GL_FRAMEBUFFER, m_uNameFrameBuffer); // Draw to framebuffer

	// FIXME:
	static CompVCanvasPtr ptrCanvas;
	if (!ptrCanvas) {
		COMPV_CHECK_CODE_ASSERT(CompVCanvas::newObj(&ptrCanvas));
	}
	COMPV_CHECK_CODE_ASSERT(ptrCanvas->test());

	glBindFramebuffer(GL_FRAMEBUFFER, 0);  // Draw to system

	//glBindTexture(GL_TEXTURE_2D, 0);

	//glEnable(GL_DEPTH_TEST);
	
	//glDisable(GL_DEPTH_WRITEMASK);
	//glDepthFunc(GL_LESS);
	

	//glPopClientAttrib();
	//glPopAttrib();
#endif

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVSurfaceGL::clear()
{
	if (m_uNameFrameBuffer) {
		COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::haveCurrentContext(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);
		GLint fbo;
		glGetIntegerv(GLenum(GL_FRAMEBUFFER_BINDING), &fbo);

		glBindFramebuffer(GL_FRAMEBUFFER, m_uNameFrameBuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVSurfaceGL::blit()
{
#if 1
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::haveCurrentContext(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);
	COMPV_CHECK_CODE_RETURN(initProgram());
	COMPV_CHECK_CODE_RETURN(m_ptrProgram->useBegin());
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // Draw to system buffer
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_uNameTexture);
	

#if defined(HAVE_OPENGL) // FIXME
	glBindVertexArray(m_uNameVAO);
#else
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED_GPU();
	glBindBuffer(GL_ARRAY_BUFFER, m_uNameVertexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uNameIndiceBuffer);
	glEnableVertexAttribArray(m_uNameSlotPosition);
	glEnableVertexAttribArray(m_uNameSlotTexCoord);
	glVertexAttribPointer(m_uNameSlotPosition, 3, GL_FLOAT, GL_FALSE, sizeof(CompVGLVertex), 0);
	glVertexAttribPointer(m_uNameSlotTexCoord, 2, GL_FLOAT, GL_FALSE, sizeof(CompVGLVertex), (GLvoid*)(sizeof(GLfloat) * 3));
#endif

	glDrawElements(GL_TRIANGLES, CompVGLTexture2DIndicesCount, GL_UNSIGNED_BYTE, 0);

#if defined(HAVE_OPENGL) // FIXME
	glBindVertexArray(0);
#else
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
#endif
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	COMPV_CHECK_CODE_RETURN(m_ptrProgram->useEnd());
	
#elif 1
#if 0
	glBindFramebuffer(GL_FRAMEBUFFER, m_uNameFrameBuffer);
	uint8_t* data = (uint8_t*)malloc(640 * 480 * 4);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glReadPixels(0, 0, 640, 480, GL_RGBA, GL_UNSIGNED_BYTE, data);
	FILE* file = fopen("C:/Projects/image.rgba", "wb+");
	fwrite(data, 1, (640 * 480 * 4), file);
	fclose(file);
	free(data);
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // System's framebuffer
#endif

	// FIXME: check texture validity

	//drawToTexture();
	

	/* Text1 */
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_uNameTexture);

	glBegin(GL_QUADS);
	glTexCoord2i(0, 0);
	glVertex2i(0, 0);

	glTexCoord2i(0, 1);
	glVertex2i(0, static_cast<GLint>(getHeight()));

	glTexCoord2i(1, 1);
	glVertex2i(static_cast<GLint>(getWidth()), static_cast<GLint>(getHeight()));

	glTexCoord2i(1, 0);
	glVertex2i(static_cast<GLint>(getWidth()), 0);
	glEnd();
	
	glBindTexture(GL_TEXTURE_2D, 0);	
#endif

	return COMPV_ERROR_CODE_S_OK;
}

// Private funtion: up to the caller to check that the GL context is valid
COMPV_ERROR_CODE CompVSurfaceGL::initFrameBuffer()
{
	if (m_uNameFrameBuffer) {
		return COMPV_ERROR_CODE_S_OK;
	}
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::haveCurrentContext(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);
	COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
	std::string errString_;
	GLenum fboStatus_;

	//glEnable(GL_TEXTURE_2D);
	//glEnable(GL_DEPTH_TEST);

	// Generate exture
	glGenTextures(1, &m_uNameTexture);
	if (!m_uNameTexture) {
		COMPV_CHECK_CODE_BAIL(err_ = CompVUtilsGL::checkLastError());
		COMPV_CHECK_CODE_BAIL(err_ = COMPV_ERROR_CODE_E_GL);
	}
	glBindTexture(GL_TEXTURE_2D, m_uNameTexture);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);        
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getWidth(), getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	if ((getWidth() & 3)) { // multiple of 4?
#if defined(GL_UNPACK_ROW_LENGTH)
		glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint)getWidth());
#endif
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	}

	// Generate a renderbuffer and use it for both for stencil and depth
	glGenRenderbuffers(1, &m_uNameDepthStencil);
	if (!m_uNameDepthStencil) {
		COMPV_CHECK_CODE_BAIL(err_ = CompVUtilsGL::checkLastError());
		COMPV_CHECK_CODE_BAIL(err_ = COMPV_ERROR_CODE_E_GL);
	}
	glBindRenderbuffer(GL_RENDERBUFFER, m_uNameDepthStencil);
#if defined(GL_DEPTH24_STENCIL8)
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, getWidth(), getHeight()); // Should match CompVDrawind::init()
#elif defined(GL_DEPTH24_STENCIL8_OES)
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, getWidth(), getHeight()); // Should match CompVDrawind::init()
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
	// Clear buffers
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// Check FBO status
	if ((fboStatus_ = glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)) {
		COMPV_CHECK_CODE_BAIL(err_ = CompVUtilsGL::checkLastError());
		COMPV_CHECK_CODE_BAIL(err_ = COMPV_ERROR_CODE_E_GL);
	}

	COMPV_DEBUG_INFO("OpenGL FBO successfully created");

bail:
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	if (COMPV_ERROR_CODE_IS_NOK(err_)) {
		deInitFrameBuffer();
	}
	return err_;
}

COMPV_ERROR_CODE CompVSurfaceGL::deInitFrameBuffer()
{
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::haveCurrentContext(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
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
	return err;
}

COMPV_ERROR_CODE CompVSurfaceGL::initProgram()
{
	if (m_ptrProgram) {
		return COMPV_ERROR_CODE_S_OK;
	}
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::haveCurrentContext(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);
	COMPV_CHECK_CODE_RETURN(initFrameBuffer());
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;

	// TODO(dmi): use GLUtils
#if defined(HAVE_OPENGL) // FIXME
	glGenVertexArrays(1, &m_uNameVAO);
	glBindVertexArray(m_uNameVAO);
#endif

	// Create vertex buffer
	glGenBuffers(1, &m_uNameVertexBuffer);
	if (!m_uNameVertexBuffer) {
		std::string errString;
		COMPV_CHECK_CODE_BAIL(err = CompVUtilsGL::getLastError(&errString));
		if (!errString.empty()) {
			COMPV_DEBUG_ERROR("Failed to create vertex buffer: %s", errString.c_str());
			COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_E_GL);
		}
	}
	glBindBuffer(GL_ARRAY_BUFFER, m_uNameVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(CompVGLScreenVertices), CompVGLScreenVertices, GL_STATIC_DRAW);
	
	// Create indice buffer
	if (!m_uNameIndiceBuffer) {
		glGenBuffers(1, &m_uNameIndiceBuffer);
		if (!m_uNameIndiceBuffer) {
			std::string errString;
			COMPV_CHECK_CODE_BAIL(err = CompVUtilsGL::getLastError(&errString));
			if (!errString.empty()) {
				COMPV_DEBUG_ERROR("Failed to create index buffer: %s", errString.c_str());
				COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_E_GL);
			}
		}
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uNameIndiceBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(CompVGLTexture2DIndices), CompVGLTexture2DIndices, GL_STATIC_DRAW);
	}

	// Create program, compile and link
	COMPV_CHECK_CODE_BAIL(err = CompVProgram::newObj(&m_ptrProgram));
	COMPV_CHECK_CODE_BAIL(err = m_ptrProgram->shadAttachVertexData(kShaderVertex, sizeof(kShaderVertex)));
	COMPV_CHECK_CODE_BAIL(err = m_ptrProgram->shadAttachFragmentData(kShaderFragment, sizeof(kShaderFragment)));
	COMPV_CHECK_CODE_BAIL(err = m_ptrProgram->link());
	COMPV_CHECK_CODE_BAIL(err = m_ptrProgram->useBegin()); // TODO(dmi): needed?

	m_uNameSlotPosition = glGetAttribLocation(m_ptrProgram->id(), "position");
	m_uNameSlotTexCoord = glGetAttribLocation(m_ptrProgram->id(), "texCoord");
	glBindBuffer(GL_ARRAY_BUFFER, m_uNameVertexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uNameIndiceBuffer);
	glEnableVertexAttribArray(m_uNameSlotPosition);
	glEnableVertexAttribArray(m_uNameSlotTexCoord);
	glVertexAttribPointer(m_uNameSlotPosition, 3, GL_FLOAT, GL_FALSE, sizeof(CompVGLVertex), 0);
	glVertexAttribPointer(m_uNameSlotTexCoord, 2, GL_FLOAT, GL_FALSE, sizeof(CompVGLVertex), (GLvoid*)(sizeof(GLfloat) * 3));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_uNameTexture);
	glUniform1i(glGetUniformLocation(m_ptrProgram->id(), "SamplerRGBA"), 0);

bail:
	if (m_ptrProgram) {
		m_ptrProgram->useEnd();
	}
#if defined(HAVE_OPENGL) // FIXME
	glBindVertexArray(0);
#endif
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	if (COMPV_ERROR_CODE_IS_NOK(err)) {
		COMPV_CHECK_CODE_RETURN(deInitProgram());
	}
	return err;
}

COMPV_ERROR_CODE CompVSurfaceGL::deInitProgram()
{
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::haveCurrentContext(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);
	if (m_uNameVertexBuffer) {
		glDeleteBuffers(1, &m_uNameVertexBuffer);
		m_uNameVertexBuffer = 0;
	}
	if (m_uNameIndiceBuffer) {
		glDeleteBuffers(1, &m_uNameIndiceBuffer);
		m_uNameIndiceBuffer = 0;
	}
#if defined(HAVE_OPENGL) // FIXME
	if (m_uNameVAO) {
		glDeleteVertexArrays(1, &m_uNameVAO);
	}
#endif
	m_ptrProgram = NULL;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVSurfaceGL::newObj(CompVSurfaceGLPtrPtr glSurface, const CompVWindow* window)
{
	COMPV_CHECK_CODE_RETURN(CompVDrawing::init());
	COMPV_CHECK_EXP_RETURN(!glSurface || !window, COMPV_ERROR_CODE_E_INVALID_PARAMETER); // Check input pointers validity
	COMPV_CHECK_EXP_RETURN(!window->isGLEnabled(), COMPV_ERROR_CODE_E_INVALID_PARAMETER); // Make sure the window has an GL context attached

	CompVSurfaceGLPtr glSurface_ = new CompVSurfaceGL(window->getWidth(), window->getHeight());
	COMPV_CHECK_EXP_RETURN(!glSurface_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	
	*glSurface = glSurface_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) ||defined(HAVE_OPENGLES) */
