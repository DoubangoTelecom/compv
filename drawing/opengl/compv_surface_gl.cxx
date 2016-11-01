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

#include "compv/drawing/opengl/compv_surface_gl_rgb.h"
#include "compv/drawing/opengl/compv_surface_gl_grayscale.h"
#include "compv/drawing/opengl/compv_surface_gl_yuv.h"

// FIXME(dmi)
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
	, m_uNameFrameBuffer(0)
	, m_uNameTexture(0)
	, m_uNameDepthStencil(0)
	
{

}

CompVSurfaceGL::~CompVSurfaceGL()
{
	COMPV_CHECK_CODE_ASSERT(deInitFrameBuffer());
}

static void drawToTexture()
{
	GLuint text;
	glGenTextures(1, &text);
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
		0,
		0,
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
	COMPV_CHECK_EXP_RETURN(!mat, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_CODE_RETURN(initFrameBuffer());
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::haveCurrentContext(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);

	glBindFramebuffer(GL_FRAMEBUFFER, m_uNameFrameBuffer); // Draw to framebuffer

#if 0
	// Get current texture
	GLint textureName;
	glGetIntegerv(GLenum(GL_TEXTURE_BINDING_2D), &textureName);
	COMPV_CHECK_EXP_RETURN(!textureName, COMPV_ERROR_CODE_E_GL);
#else
	glBindTexture(GL_TEXTURE_2D, m_uNameTexture);
#endif

	//CompVProgramPtr ptrProgram;
	//COMPV_CHECK_CODE_ASSERT(CompVProgram::newObj(&ptrProgram));

	//COMPV_CHECK_CODE_ASSERT(ptrProgram->shadAttachVertexData(kShaderVertex, sizeof(kShaderVertex)));
	//COMPV_CHECK_CODE_ASSERT(ptrProgram->shadAttachFragmentData(kShaderFragmentGreen, sizeof(kShaderFragment)));
	//COMPV_CHECK_CODE_ASSERT(ptrProgram->link());

	//COMPV_CHECK_CODE_ASSERT(ptrProgram->useBegin());

	// FIXME:
#if 1
	int size = getWidth()*getHeight() * 4;
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
	free(data);
#else
	int size = getWidth()*getHeight() * 4;
	uint8_t* data = (uint8_t*)malloc(size);
	for (int i = 0; i < size; ++i)data[i] = rand();
	glTexSubImage2D(
		GL_TEXTURE_2D,
		0,
		0,
		0,
		static_cast<GLsizei>(200),
		static_cast<GLsizei>(100),
		GL_RGB,
		GL_UNSIGNED_BYTE,
		mat->ptr());
	free(data);
#endif

	

	
	//glClearColor(0.f, 0.f, 0.f, 1.f);
	//glViewport(0, 0, static_cast<GLsizei>(getWidth()), static_cast<GLsizei>(getHeight()));
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glEnable(GL_DEPTH_TEST);
	//glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();
	//glOrtho((GLdouble)0, static_cast<GLdouble>(getWidth()), (GLdouble)0, static_cast<GLdouble>(getHeight()), (GLdouble)-1, (GLdouble)1);
	//glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();

	/*glBegin(GL_QUADS);
	glTexCoord2i(0, 0);
	glVertex2i(0, 0);
	glTexCoord2i(0, 1);
	glVertex2i(0, static_cast<GLint>(getHeight()));
	glTexCoord2i(1, 1);
	glVertex2i(static_cast<GLint>(getWidth()), static_cast<GLint>(getHeight()));
	glTexCoord2i(1, 0);
	glVertex2i(static_cast<GLint>(getWidth()), 0);
	glEnd();*/

	//glDrawArrays(GL_TRIANGLES, 0, 1);

	//COMPV_CHECK_CODE_ASSERT(ptrProgram->useEnd());

	glBindTexture(GL_TEXTURE_2D, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0); // Draw to system

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVSurfaceGL::drawText(const void* textPtr, size_t textLengthInBytes)
{
	COMPV_CHECK_EXP_RETURN(!textPtr || !textLengthInBytes, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_CODE_RETURN(initFrameBuffer());
	
	glBindFramebuffer(GL_FRAMEBUFFER, m_uNameFrameBuffer); // Draw to framebuffer

	CompVCanvasPtr ptrCanvas;
	COMPV_CHECK_CODE_ASSERT(CompVCanvas::newObj(&ptrCanvas));
	COMPV_CHECK_CODE_ASSERT(ptrCanvas->test());

	glBindFramebuffer(GL_FRAMEBUFFER, 0);  // Draw to system

#if 0
	glBindTexture(GL_TEXTURE_2D, m_uNameTexture);

	CompVProgramPtr ptrProgram;
	COMPV_CHECK_CODE_ASSERT(CompVProgram::newObj(&ptrProgram));

	COMPV_CHECK_CODE_ASSERT(ptrProgram->shadAttachVertexData(kShaderVertex, sizeof(kShaderVertex)));
	COMPV_CHECK_CODE_ASSERT(ptrProgram->shadAttachFragmentData(kShaderFragment, sizeof(kShaderFragment)));
	COMPV_CHECK_CODE_ASSERT(ptrProgram->link());

	COMPV_CHECK_CODE_ASSERT(ptrProgram->useBegin());

	glClearColor(0.f, 0.f, 0.f, 1.f);
	glViewport(0, 0, static_cast<GLsizei>(getWidth()), static_cast<GLsizei>(getHeight()));
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho((GLdouble)0, static_cast<GLdouble>(getWidth()), (GLdouble)0, static_cast<GLdouble>(getHeight()), (GLdouble)-1, (GLdouble)1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

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

	COMPV_CHECK_CODE_ASSERT(ptrProgram->useEnd());
#endif

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
