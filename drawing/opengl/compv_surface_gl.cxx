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

#include "compv/drawing/opengl/compv_utils_gl.h"

// FIXME: OpenGL error handling not ok, impossible to find which function cause the error (erros stacked)

// FIXME(dmi): remove
#if defined(main)
#error "main must not be defined"
#endif
static const char kShaderVertex[] = COMPV_STRING(
	void main(void) {
		gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
		// gl_Position = ftransform();
		// gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
		gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex;
	}
);
static const char kShaderFragment[] = COMPV_STRING(
	uniform sampler2D tex;
	void main(void) {
		//gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0); 
		//vec4 color = texture2D(tex, gl_TexCoord[0]);
		gl_FragColor = texture2D(tex, gl_TexCoord[0]);
	}
);

static const char kShaderFragmentGreen[] = COMPV_STRING(
	uniform sampler2D tex;
	void main(void) {
		gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0); 
		//vec4 color = texture2D(tex, gl_TexCoord[0]);
		//gl_FragColor = texture2D(tex, gl_TexCoord[0]);
	}
);

COMPV_NAMESPACE_BEGIN()

CompVSurfaceGL::CompVSurfaceGL(int width, int height)
	: CompVSurface(width, height)
	, CompVSurfaceBlit()
	, m_uNameFrameBuffer(0)
	, m_uNameTexture(0)
	, m_uNameDepthStencil(0)
	
{

}

CompVSurfaceGL::~CompVSurfaceGL()
{
	COMPV_CHECK_CODE_ASSERT(deInitFrameBuffer());
}

// FIXME: remove
static void drawToTexture(GLint xoff = 0, GLint yoff = 0)
{
	GLuint text;
	glGenTextures(1, &text);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, text);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 256, 256, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);
	if ((256 & 3)) { // multiple of 4?
		glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint)256);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	}

	int size = 256 * 256 * 1;
	uint8_t* data = (uint8_t*)malloc(size);
	for (int i = 0; i < size; ++i)data[i] = i % 255;
	glTexSubImage2D(
		GL_TEXTURE_2D,
		0,
		xoff,
		yoff,
		static_cast<GLsizei>(256),
		static_cast<GLsizei>(256),
		GL_LUMINANCE,
		GL_UNSIGNED_BYTE,
		data);
	free(data);

	glBegin(GL_QUADS);
	glTexCoord2i(0, 0);
	glVertex2i(0, 0);
	
	glTexCoord2i(0, 1);
	glVertex2i(0, static_cast<GLint>(256));

	glTexCoord2i(1, 1);
	glVertex2i(static_cast<GLint>(256), static_cast<GLint>(256));

	glTexCoord2i(1, 0);
	glVertex2i(static_cast<GLint>(256), 0);
	glEnd();

	glDeleteTextures(1, &text);
	glBindTexture(GL_TEXTURE_2D, 0);
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
	//--glPushAttrib(GL_ALL_ATTRIB_BITS);
	//glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

	glBindFramebuffer(GL_FRAMEBUFFER, m_uNameFrameBuffer); // Draw to framebuffer

	// FIXME:
	static CompVCanvasPtr ptrCanvas;
	if (!ptrCanvas) {
		COMPV_CHECK_CODE_ASSERT(CompVCanvas::newObj(&ptrCanvas));
	}
	COMPV_CHECK_CODE_ASSERT(ptrCanvas->test());

	glBindFramebuffer(GL_FRAMEBUFFER, 0);  // Draw to system

	glBindTexture(GL_TEXTURE_2D, 0);

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
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::haveCurrentContext(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);

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

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);

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
		glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint)getWidth());
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	}

	// FIXME: not needed
	/*int size = getWidth() * getHeight() * 4;
	uint8_t* data = (uint8_t*)malloc(size);
	for (int i = 0; i < size; ++i)data[i] = rand();
	glTexSubImage2D(
		GL_TEXTURE_2D,
		0,
		0,
		0,
		static_cast<GLsizei>(getWidth()),
		static_cast<GLsizei>(getHeight()),
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		data);
	free(data);*/

	// Generate a renderbuffer and use it for both for stencil and depth
	glGenRenderbuffers(1, &m_uNameDepthStencil);
	if (!m_uNameDepthStencil) {
		COMPV_CHECK_CODE_BAIL(err_ = CompVUtilsGL::checkLastError());
		COMPV_CHECK_CODE_BAIL(err_ = COMPV_ERROR_CODE_E_GL);
	}
	glBindRenderbuffer(GL_RENDERBUFFER, m_uNameDepthStencil);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, getWidth(), getHeight()); // Should match CompVDrawind::init()

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
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

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
