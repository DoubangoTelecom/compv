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

static const std::string& kProgramVertexData = ""
"	attribute vec4 position;"
"	attribute vec2 texCoord;"
"	varying vec2 texCoordVarying;"
"	void main() {"
"		gl_Position = position;"
"		texCoordVarying = texCoord;"
"	}";

static const std::string& kProgramShaderDataR8G8B8 = ""
"	varying vec2 texCoordVarying;"
"	uniform sampler2D mySampler;"
"	void main() {"
"		gl_FragColor = vec4(texture2D(mySampler, texCoordVarying).rgb, 1.0); /* RGB -> RGBA */"
"	}";

static const std::string& kProgramShaderDataR8G8B8A8 = ""
"	varying vec2 texCoordVarying;"
"	uniform sampler2D mySampler;"
"	void main() {"
"		gl_FragColor = texture2D(mySampler, texCoordVarying).rgba; /* RGBA -> RGBA */"
"	}";

COMPV_NAMESPACE_BEGIN()

CompVRendererGLRGB::CompVRendererGLRGB(COMPV_PIXEL_FORMAT ePixelFormat, GLuint uNameSurfaceTexture)
	: CompVRendererGL(ePixelFormat, uNameSurfaceTexture)
	, m_bInit(false)
	, m_iFormat(GL_RGB)
	, m_uNameTexture(0)
	, m_uNameSampler(0)
	, m_uWidth(0)
	, m_uHeight(0)
	, m_uStride(0)
	, m_strPrgVertexData(kProgramVertexData)
	, m_strPrgFragData("")
{

}

CompVRendererGLRGB::~CompVRendererGLRGB()
{
	if (m_bInit) {
		COMPV_CHECK_CODE_ASSERT(deInit());
	}
}

COMPV_ERROR_CODE CompVRendererGLRGB::drawImage(CompVMatPtr mat)
{
	COMPV_CHECK_EXP_RETURN(!mat || mat->isEmpty(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::haveCurrentContext(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);

	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;

	// Get pixel format and make sure it's supported
	COMPV_PIXEL_FORMAT pixelFormat = static_cast<COMPV_PIXEL_FORMAT>(mat->subType());
	COMPV_CHECK_EXP_RETURN(CompVRenderer::pixelFormat() != pixelFormat, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	// Check if format changed
	if (mat->cols(0) != m_uWidth || mat->rows() != m_uHeight || mat->stride() != m_uStride) {
		COMPV_DEBUG_INFO("GL renderer format changed: %d -> %d", CompVRenderer::pixelFormat(), pixelFormat);
		COMPV_CHECK_CODE_RETURN(deInit());
	}

	// Init if not already done
	if (!m_bInit) {
		COMPV_CHECK_CODE_RETURN(init(mat));
	}
	
	COMPV_CHECK_CODE_BAIL(err = CompVRendererGL::program()->useBegin());
	COMPV_CHECK_CODE_BAIL(err = CompVRendererGL::bindBuffers());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, CompVRendererGL::nameSurfaceTexture());

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_uNameTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, m_iFormat, static_cast<GLsizei>(mat->stride()), static_cast<GLsizei>(mat->rows()), 0, m_iFormat, GL_UNSIGNED_BYTE, mat->ptr());

	glDrawElements(GL_TRIANGLES, CompVRendererGL::indicesCount(), GL_UNSIGNED_BYTE, 0);

	m_uWidth = mat->cols(0);
	m_uHeight = mat->rows();
	m_uStride = mat->stride();

bail:
	COMPV_CHECK_CODE_ASSERT(CompVRendererGL::unbindBuffers());
	COMPV_CHECK_CODE_ASSERT(program()->useEnd());
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	return err;
}

COMPV_ERROR_CODE CompVRendererGLRGB::deInit()
{
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::haveCurrentContext(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);
	COMPV_CHECK_CODE_RETURN(CompVRendererGL::deInit()); // Base class implementation
	if (m_uNameTexture) {
		glDeleteTextures(1, &m_uNameTexture);
	}

	m_bInit = false;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVRendererGLRGB::init(CompVMatPtr mat)
{
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	CompVProgramPtr ptrProgram;
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::haveCurrentContext(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);
	COMPV_CHECK_EXP_RETURN(m_bInit, COMPV_ERROR_CODE_E_INVALID_STATE);
	COMPV_CHECK_CODE_BAIL(err = CompVRendererGL::init(mat)); // Base class implementation
	COMPV_CHECK_EXP_BAIL(!(ptrProgram = program()), (err = COMPV_ERROR_CODE_E_GL));
	COMPV_CHECK_CODE_BAIL(err = ptrProgram->useBegin());

	glGenTextures(1, &m_uNameTexture);
	glActiveTexture(GL_TEXTURE1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glBindTexture(GL_TEXTURE_2D, m_uNameTexture);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, m_iFormat, static_cast<GLsizei>(mat->stride()), static_cast<GLsizei>(mat->rows()), 0, m_iFormat, GL_UNSIGNED_BYTE, NULL);
	m_uNameSampler = glGetUniformLocation(ptrProgram->id(), "mySampler");
	glUniform1i(m_uNameSampler, 1);

bail:
	if (ptrProgram) {
		COMPV_CHECK_CODE_ASSERT(ptrProgram->useEnd());
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glBindTexture(GL_TEXTURE_2D, 0);
	if (COMPV_ERROR_CODE_IS_NOK(err)) {
		COMPV_CHECK_CODE_ASSERT(deInit());
	}
	else {
		m_bInit = true;
		return err;
	}
	return err;
}

COMPV_ERROR_CODE CompVRendererGLRGB::newObj(CompVRendererGLRGBPtrPtr glRenderer, COMPV_PIXEL_FORMAT eRGBPixelFormat, GLuint uNameSurfaceTexture)
{
	COMPV_CHECK_CODE_RETURN(CompVDrawing::init());
	COMPV_CHECK_EXP_RETURN(!glRenderer, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_EXP_RETURN(
		eRGBPixelFormat != COMPV_PIXEL_FORMAT_R8G8B8
		&& eRGBPixelFormat != COMPV_PIXEL_FORMAT_B8G8R8
		&& eRGBPixelFormat != COMPV_PIXEL_FORMAT_R8G8B8A8
		&& eRGBPixelFormat != COMPV_PIXEL_FORMAT_B8G8R8A8
		&& eRGBPixelFormat != COMPV_PIXEL_FORMAT_A8B8G8R8
		&& eRGBPixelFormat != COMPV_PIXEL_FORMAT_A8R8G8B8,
		COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	CompVRendererGLRGBPtr glRenderer_ = new CompVRendererGLRGB(eRGBPixelFormat, uNameSurfaceTexture);
	COMPV_CHECK_EXP_RETURN(!glRenderer_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	glRenderer_->m_iFormat = (eRGBPixelFormat == COMPV_PIXEL_FORMAT_R8G8B8A8
		|| eRGBPixelFormat == COMPV_PIXEL_FORMAT_B8G8R8A8
		|| eRGBPixelFormat == COMPV_PIXEL_FORMAT_A8B8G8R8
		|| eRGBPixelFormat == COMPV_PIXEL_FORMAT_A8R8G8B8) 
		? GL_RGBA : GL_RGB;

	switch (eRGBPixelFormat) {
	case COMPV_PIXEL_FORMAT_R8G8B8:
		glRenderer_->m_strPrgFragData = kProgramShaderDataR8G8B8;
		break;
	case COMPV_PIXEL_FORMAT_R8G8B8A8:
		glRenderer_->m_strPrgFragData = kProgramShaderDataR8G8B8A8;
		break;
	default:
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
		break;
	}

	COMPV_CHECK_EXP_RETURN(!(*glRenderer = glRenderer_), COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */
