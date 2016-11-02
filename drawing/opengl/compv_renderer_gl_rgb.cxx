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
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, static_cast<GLsizei>(mat->stride()), static_cast<GLsizei>(mat->rows()), 0, GL_RGB, GL_UNSIGNED_BYTE, mat->ptr());
		if ((mat->stride() & 3)) { // multiple of 4?
			glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint)mat->stride());
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		}
	}
	else {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_uNameTexture);
		glTexSubImage2D(
			GL_TEXTURE_2D,
			0,
			0,
			0,
			static_cast<GLsizei>(mat->stride()),
			static_cast<GLsizei>(mat->rows()),
			GL_RGB,
			GL_UNSIGNED_BYTE,
			mat->ptr());
	}
	// Attach texture to FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_uNameTexture, 0); // FIXME: later, detach
	
	// Check FBO's status
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		COMPV_CHECK_CODE_BAIL(err = CompVUtilsGL::checkLastError());
		COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_E_GL);
	}

	// FIXME: move to blit?
	

	m_uWidth = mat->cols();
	m_uHeight = mat->rows();
	m_uStride = mat->stride();

	// Draw
	COMPV_CHECK_CODE_BAIL(err = blit());

bail:
	// dettach
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, 0, 0);

	return err;
}

COMPV_ERROR_CODE CompVRendererGLRgb::blit()
{
	// FIXME: check GLContext is available
	// FIXME: create program once
#if defined(main)
#error "main must not be defined"
#endif
	static const char kShaderVertex[] = COMPV_STRING(
		void main() {
			gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
			gl_TexCoord[1] = gl_TextureMatrix[1] * gl_MultiTexCoord1;
			//gl_Position = ftransform();
			// gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
			gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex;
		}
	);
	static const char kShaderFragment[] = COMPV_STRING(
		uniform sampler2D tex0;
		uniform sampler2D tex1;
		void main() {
			//gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0); 
			//vec4 color = texture2D(tex, gl_TexCoord[0]);
			//gl_FragColor = texture2D(tex0, gl_TexCoord[0]);
			//gl_FragColor = texture2D(tex1, gl_TexCoord[0]);
			gl_FragColor = vec4(texture2D(tex1, gl_TexCoord[0]).rgb, 1.0);
		}
	);
	
	// FIXME: create program once and move to base class
	CompVProgramPtr ptrProgram;
	COMPV_CHECK_CODE_RETURN(CompVProgram::newObj(&ptrProgram));
	COMPV_CHECK_CODE_RETURN(ptrProgram->shadAttachVertexData(kShaderVertex, sizeof(kShaderVertex)));
	COMPV_CHECK_CODE_RETURN(ptrProgram->shadAttachFragmentData(kShaderFragment, sizeof(kShaderFragment)));
	COMPV_CHECK_CODE_RETURN(ptrProgram->link());
	COMPV_CHECK_CODE_ASSERT(ptrProgram->useBegin());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 1); // FIXME
	glUniform1i(glGetUniformLocation(ptrProgram->id(), "tex0"), 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_uNameTexture); // FIXME
	glUniform1i(glGetUniformLocation(ptrProgram->id(), "tex1"), 1);

	glBegin(GL_QUADS);
	glTexCoord2i(0, 0);
	glVertex2i(0, 0);

	glTexCoord2i(0, 1);
	glVertex2i(0, static_cast<GLint>(m_uStride));

	glTexCoord2i(1, 1);
	glVertex2i(static_cast<GLint>(m_uStride), static_cast<GLint>(m_uHeight));

	glTexCoord2i(1, 0);
	glVertex2i(static_cast<GLint>(m_uStride), 0);

	glEnd();

	COMPV_CHECK_CODE_ASSERT(ptrProgram->useEnd());

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);	

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

#endif /* defined(HAVE_OPENGL) ||defined(HAVE_OPENGLES) */
