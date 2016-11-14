/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/drawing/opengl/compv_renderer_gl.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/drawing/compv_drawing.h"
#include "compv/drawing/opengl/compv_utils_gl.h"
#include "compv/drawing/opengl/compv_renderer_gl_yuv.h"
#include "compv/drawing/opengl/compv_renderer_gl_rgb.h"

COMPV_NAMESPACE_BEGIN()

CompVRendererGL::CompVRendererGL(COMPV_PIXEL_FORMAT ePixelFormat, GLuint uNameSurfaceTexture)
	: CompVRenderer(ePixelFormat)
	, m_bInit(false)
	, m_uNameVertexBuffer(0)
	, m_uNameIndiceBuffer(0)
	, m_uNameSurfaceTexture(uNameSurfaceTexture)
	, m_uNamePrgAttPosition(0)
	, m_uNamePrgAttTexCoord(0)
{
}

CompVRendererGL::~CompVRendererGL()
{
	if (m_bInit) {
		COMPV_CHECK_CODE_ASSERT(deInit());
	}
}

// Private function: do not check imput parameters
COMPV_ERROR_CODE CompVRendererGL::init(CompVMatPtr mat)
{
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::haveCurrentContext(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	GLfloat uMax, vMax;
	std::string prgVertexData, prgFragData;

	// Vertex buffer
	// TODO(dmi): use GLUtils
	if (!m_uNameVertexBuffer) {
		glGenBuffers(1, &m_uNameVertexBuffer);
		if (!m_uNameVertexBuffer) {
			std::string errString;
			COMPV_CHECK_CODE_BAIL(err = CompVUtilsGL::getLastError(&errString));
			if (!errString.empty()) {
				COMPV_DEBUG_ERROR("Failed to create vertex buffer: %s", errString.c_str());
				COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_E_GL);
			}
		}
	}
	glBindBuffer(GL_ARRAY_BUFFER, m_uNameVertexBuffer);
	uMax = static_cast<GLfloat>(mat->cols()) / static_cast<GLfloat>(mat->stride());
	vMax = 1.f;
	m_Vertices[0].TexCoord[0] = uMax, m_Vertices[0].TexCoord[1] = vMax;
	m_Vertices[1].TexCoord[0] = uMax, m_Vertices[0].TexCoord[1] = 0.f;
	m_Vertices[2].TexCoord[0] = 0.f, m_Vertices[0].TexCoord[1] = 0.f;
	m_Vertices[3].TexCoord[0] = 0.f, m_Vertices[0].TexCoord[1] = vMax;
	glBufferData(GL_ARRAY_BUFFER, sizeof(m_Vertices), m_Vertices, GL_STATIC_DRAW);

	// Indice buffer
	// TODO(dmi): use GLUtils
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

	COMPV_CHECK_CODE_BAIL(CompVProgram::newObj(&m_ptrProgram));
	prgVertexData = programVertexData();
	COMPV_CHECK_EXP_BAIL(prgVertexData.empty(), err = COMPV_ERROR_CODE_E_GL);
	prgFragData = programFragData();
	COMPV_CHECK_EXP_BAIL(prgFragData.empty(), err = COMPV_ERROR_CODE_E_GL);
	COMPV_CHECK_CODE_RETURN(err = m_ptrProgram->shadAttachVertexData(prgVertexData.c_str(), prgVertexData.length()));
	COMPV_CHECK_CODE_RETURN(err = m_ptrProgram->shadAttachFragmentData(prgFragData.c_str(), prgFragData.length()));
	COMPV_CHECK_CODE_RETURN(err = m_ptrProgram->link());
	COMPV_CHECK_CODE_ASSERT(err = m_ptrProgram->useBegin());

	m_uNamePrgAttPosition = glGetAttribLocation(m_ptrProgram->id(), "position");
	m_uNamePrgAttTexCoord = glGetAttribLocation(m_ptrProgram->id(), "texCoord");
	glBindBuffer(GL_ARRAY_BUFFER, m_uNameVertexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uNameIndiceBuffer);
	glEnableVertexAttribArray(m_uNamePrgAttPosition);
	glEnableVertexAttribArray(m_uNamePrgAttTexCoord);
	glVertexAttribPointer(m_uNamePrgAttPosition, 3, GL_FLOAT, GL_FALSE, sizeof(CompVGLVertex), 0);
	glVertexAttribPointer(m_uNamePrgAttTexCoord, 2, GL_FLOAT, GL_FALSE, sizeof(CompVGLVertex), (GLvoid*)(sizeof(GLfloat) * 3));

	m_bInit = true;

bail:
	if (m_ptrProgram) {
		COMPV_CHECK_CODE_ASSERT(m_ptrProgram->useEnd());
	}
	if (COMPV_ERROR_CODE_IS_NOK(err)) {
		COMPV_CHECK_CODE_ASSERT(deInit());
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return err;
}

COMPV_ERROR_CODE CompVRendererGL::bindBuffers()
{
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::haveCurrentContext(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);
	COMPV_CHECK_EXP_RETURN(!m_bInit, COMPV_ERROR_CODE_E_INVALID_STATE);
	
	// FIXME(dmi): use VAO
	glBindBuffer(GL_ARRAY_BUFFER, nameVertexBuffer());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, nameIndiceBuffer());
	glEnableVertexAttribArray(namePrgAttPosition());
	glEnableVertexAttribArray(namePrgAttTexCoord());
	glVertexAttribPointer(namePrgAttPosition(), 3, GL_FLOAT, GL_FALSE, sizeof(CompVGLVertex), 0);
	glVertexAttribPointer(namePrgAttTexCoord(), 2, GL_FLOAT, GL_FALSE, sizeof(CompVGLVertex), (GLvoid*)(sizeof(GLfloat) * 3));

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVRendererGL::unbindBuffers()
{
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::haveCurrentContext(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);

	// FIXME(dmi): use VAO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVRendererGL::deInit()
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
	m_ptrProgram = NULL;

	m_bInit = false;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVRendererGL::newObj(CompVRendererGLPtrPtr glRenderer, COMPV_PIXEL_FORMAT ePixelFormat, GLuint uNameSurfaceTexture)
{
	COMPV_CHECK_CODE_RETURN(CompVDrawing::init());
	COMPV_CHECK_EXP_RETURN(!glRenderer, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	CompVRendererGLPtr glRenderer_;
	switch (ePixelFormat) {
	case COMPV_PIXEL_FORMAT_R8G8B8:
	case COMPV_PIXEL_FORMAT_B8G8R8:
	case COMPV_PIXEL_FORMAT_R8G8B8A8:
	case COMPV_PIXEL_FORMAT_B8G8R8A8:
	case COMPV_PIXEL_FORMAT_A8B8G8R8:
	case COMPV_PIXEL_FORMAT_A8R8G8B8: {
		CompVRendererGLRGBPtr rgbGLRenderer_;
		COMPV_CHECK_CODE_RETURN(CompVRendererGLRGB::newObj(&rgbGLRenderer_, ePixelFormat, uNameSurfaceTexture));
		glRenderer_ = *rgbGLRenderer_;
		break;
	}
	case COMPV_PIXEL_FORMAT_GRAYSCALE:
	case COMPV_PIXEL_FORMAT_I420: {
		CompVRendererGLYUVPtr yuvGLRenderer_;
		COMPV_CHECK_CODE_RETURN(CompVRendererGLYUV::newObj(&yuvGLRenderer_, ePixelFormat, uNameSurfaceTexture));
		glRenderer_ = *yuvGLRenderer_;
		break;
	}
	default:
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
		break;
	}
	COMPV_CHECK_EXP_RETURN(!glRenderer_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	COMPV_CHECK_EXP_RETURN(sizeof(glRenderer_->m_Vertices) != sizeof(CompVGLTexture2DVertices), COMPV_ERROR_CODE_E_SYSTEM);
	memcpy(&glRenderer_->m_Vertices, &CompVGLTexture2DVertices, sizeof(CompVGLTexture2DVertices));

	COMPV_CHECK_EXP_RETURN(!(*glRenderer = glRenderer_), COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */
