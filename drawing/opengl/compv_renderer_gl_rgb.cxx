/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/drawing/opengl/compv_renderer_gl_rgb.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/drawing/compv_drawing.h"
#include "compv/drawing/compv_program.h"

#include "compv/drawing/opengl/compv_utils_gl.h"

#define COMPV_SUPPORT_STRIDE 0

COMPV_NAMESPACE_BEGIN()

CompVRendererGLRgb::CompVRendererGLRgb(COMPV_PIXEL_FORMAT ePixelFormat)
	: CompVRendererGL(ePixelFormat)
	, m_uNameTexture(0)
	, m_uWidth(0)
	, m_uHeight(0)
	, m_uStride(0)
{

}

CompVRendererGLRgb::~CompVRendererGLRgb()
{
	// FIXME: check glcontext
	if (m_uNameTexture) {
		glDeleteTextures(1, &m_uNameTexture);
	}
}

COMPV_ERROR_CODE CompVRendererGLRgb::render(CompVMatPtr mat)
{
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	// Make sure we're bound to our frambuffer (FIXME: use name as parameter)
	GLint uNameFramebuffer;
	glGetIntegerv(GLenum(GL_FRAMEBUFFER_BINDING), &uNameFramebuffer);
	COMPV_CHECK_EXP_RETURN(!uNameFramebuffer, COMPV_ERROR_CODE_E_INVALID_STATE);

	
	if (m_uNameTexture && (m_uWidth != mat->cols() || m_uHeight != mat->rows() || m_uStride != mat->stride())) {
		glDeleteTextures(1, &m_uNameTexture);
		m_uNameTexture = 0;
	}

#if COMPV_SUPPORT_STRIDE
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint)mat->stride());
#endif

	// Create texture if not already done
	// FIXME(dmi): m_uNameTexture
	if (!m_uNameTexture) {
		glGenTextures(1, &m_uNameTexture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_uNameTexture);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#if COMPV_SUPPORT_STRIDE
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, static_cast<GLsizei>(mat->cols()), static_cast<GLsizei>(mat->rows()), 0, GL_RGB, GL_UNSIGNED_BYTE, mat->ptr());
#else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, static_cast<GLsizei>(mat->stride()), static_cast<GLsizei>(mat->rows()), 0, GL_RGB, GL_UNSIGNED_BYTE, mat->ptr());
#endif
	}
	else {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_uNameTexture);
		glTexSubImage2D(
			GL_TEXTURE_2D,
			0,
			0,
			0,
#if COMPV_SUPPORT_STRIDE
			static_cast<GLsizei>(mat->cols()),
#else
			static_cast<GLsizei>(mat->stride()),
#endif
			static_cast<GLsizei>(mat->rows()),
			GL_RGB,
			GL_UNSIGNED_BYTE,
			mat->ptr());
	}
	/*
	// Attach texture to FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_uNameTexture, 0); // FIXME: later, detach
	
	// Check FBO's status
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		COMPV_CHECK_CODE_BAIL(err = CompVUtilsGL::checkLastError());
		COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_E_GL);
	}
	*/

	// FIXME: move to blit?
	

	m_uWidth = mat->cols();
	m_uHeight = mat->rows();
	m_uStride = mat->stride();

	// Draw
	COMPV_CHECK_CODE_BAIL(err = blit());

bail:
#if COMPV_SUPPORT_STRIDE
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
	// dettach
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, 0, 0);

	return err;
}

COMPV_ERROR_CODE CompVRendererGLRgb::blit()
{
	// FIXME: use VAO
	// FIXME: check GLContext is available
	// FIXME: create program once
	// FIXME: use 'precision mediump float;' on OpenGLES
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
		//uniform sampler2D tex0;
		//uniform sampler2D tex1;
		varying vec2 texCoordVarying;
		uniform sampler2D SamplerRGB;
		void main() {
			gl_FragColor = vec4(texture2D(SamplerRGB, texCoordVarying).rgb, 1.0);
		}
	);

#if !COMPV_SUPPORT_STRIDE
	GLfloat uMax = ((GLfloat)m_uWidth / (GLfloat)m_uStride);
	GLfloat vMax = 1.f;
	static const CompVGLVertex Vertices[] = {
		{ { 1.f, -1.f, 0.f }, { uMax, vMax } },
		{ { 1.f, 1.f, 0.f }, { uMax, 0.f } },
		{ { -1.f, 1.f, 0.f }, { 0.f, 0.f } },
		{ { -1.f, -1.f, 0.f }, { 0.f, vMax } }
	};
#endif
	
	// FIXME: create program once and move to base class
	CompVProgramPtr ptrProgram;
	COMPV_CHECK_CODE_RETURN(CompVProgram::newObj(&ptrProgram));
	COMPV_CHECK_CODE_RETURN(ptrProgram->shadAttachVertexData(kShaderVertex, sizeof(kShaderVertex)));
	COMPV_CHECK_CODE_RETURN(ptrProgram->shadAttachFragmentData(kShaderFragment, sizeof(kShaderFragment)));
	COMPV_CHECK_CODE_RETURN(ptrProgram->link());
	COMPV_CHECK_CODE_ASSERT(ptrProgram->useBegin());

	// TODO(dmi): retrieving positions must be done once
	GLuint slotPosition = glGetAttribLocation(ptrProgram->id(), "position");
	GLuint slotTexCoord = glGetAttribLocation(ptrProgram->id(), "texCoord");
#if COMPV_SUPPORT_STRIDE
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer());
#else
	GLuint nameVertexBuffer;
	glGenBuffers(1, &nameVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, nameVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
#endif
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indiceBuffer());
	glEnableVertexAttribArray(slotTexCoord);
	glEnableVertexAttribArray(slotPosition);
	glVertexAttribPointer(slotPosition, 3, GL_FLOAT, GL_FALSE, sizeof(CompVGLVertex), 0);
	glVertexAttribPointer(slotTexCoord, 2, GL_FLOAT, GL_FALSE, sizeof(CompVGLVertex), (GLvoid*)(sizeof(GLfloat) * 3));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 1); // FIXME
	//glUniform1i(glGetUniformLocation(ptrProgram->id(), "tex0"), 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_uNameTexture); // FIXME
	glUniform1i(glGetUniformLocation(ptrProgram->id(), "SamplerRGB"), 1); // TODO(dmi): must be done once

	glDrawElements(GL_TRIANGLES, CompVRendererGL::indicesCount(), GL_UNSIGNED_BYTE, 0);

	COMPV_CHECK_CODE_ASSERT(ptrProgram->useEnd());

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

#if COMPV_SUPPORT_STRIDE
#else
	glDeleteBuffers(1, &nameVertexBuffer);
#endif

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVRendererGLRgb::newObj(CompVRendererGLRgbPtrPtr glRgbRenderer, COMPV_PIXEL_FORMAT ePixelFormat, const CompVSurface* surface)
{
	COMPV_CHECK_CODE_RETURN(CompVDrawing::init());
	COMPV_CHECK_EXP_RETURN(!glRgbRenderer || !surface, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_EXP_RETURN(!surface->isGLEnabled(), COMPV_ERROR_CODE_E_INVALID_PARAMETER); // Make sure both the window and surface have an GL context attached
	CompVRendererGLRgbPtr glRgbRenderer_;
	switch (ePixelFormat) {
	case COMPV_PIXEL_FORMAT_R8G8B8:
		glRgbRenderer_ = new CompVRendererGLRgb(ePixelFormat);
		COMPV_CHECK_EXP_RETURN(!glRgbRenderer_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
		break;
	default:
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT);
		break;
	}	

	*glRgbRenderer = glRgbRenderer_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */
