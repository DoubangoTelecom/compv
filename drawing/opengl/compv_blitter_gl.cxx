/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/drawing/opengl/compv_blitter_gl.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/drawing/compv_drawing.h"
#include "compv/drawing/opengl/compv_utils_gl.h"

// FIXME(dmi): VAO - remove
#if defined(HAVE_OPENGL)
#	define COMPV_VAO 1
#else
#	define COMPV_VAO 0
#endif

COMPV_NAMESPACE_BEGIN()

CompVBlitterGL::CompVBlitterGL()
	: m_bInit(false)
	, m_nWidth(0)
	, m_nHeight(0)
	, m_nStride(0)
	, m_bToScreen(false)
	, m_uNameVertexBuffer(0)
	, m_uNameIndiceBuffer(0)
	, m_uNamePrgAttPosition(0)
	, m_uNamePrgAttTexCoord(0)
	, m_uNamePrgUnifMVP(0)
	, m_uNameVAO(0)
{

}

CompVBlitterGL::~CompVBlitterGL()
{
	COMPV_CHECK_CODE_ASSERT(deInit());
}

// Bind to VAO and activate the program
COMPV_ERROR_CODE CompVBlitterGL::bind()
{
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::haveCurrentContext(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);
	COMPV_CHECK_EXP_RETURN(!m_bInit, COMPV_ERROR_CODE_E_INVALID_STATE);
	COMPV_CHECK_CODE_RETURN(m_ptrProgram->useBegin());

	// Because MVP could be dirty we have to send the data again
	// FIXME(dmi): find a way to detect that MVP is dirty
	if (m_bMVP && m_ptrMVP) {
		glUniformMatrix4fv(m_uNamePrgUnifMVP, 1, GL_FALSE, m_ptrMVP->matrix()->ptr());
	}

#if COMPV_VAO && 0 // FIXME(dmi):
	glBindVertexArray(m_uNameVAO);
#else
	glBindBuffer(GL_ARRAY_BUFFER, m_uNameVertexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uNameIndiceBuffer);
	glEnableVertexAttribArray(m_uNamePrgAttPosition);
	glEnableVertexAttribArray(m_uNamePrgAttTexCoord);
	glVertexAttribPointer(m_uNamePrgAttPosition, 3, GL_FLOAT, GL_FALSE, sizeof(CompVGLVertex), 0);
	glVertexAttribPointer(m_uNamePrgAttTexCoord, 2, GL_FLOAT, GL_FALSE, sizeof(CompVGLVertex), (GLvoid*)(sizeof(GLfloat) * 3));
#endif

	return COMPV_ERROR_CODE_S_OK;
}

// Unbind the VAO and deactivate the program
COMPV_ERROR_CODE CompVBlitterGL::unbind()
{
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::haveCurrentContext(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);

#if COMPV_VAO && 0 // FIXME(dmi):
	glBindVertexArray(0);
#else
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
#endif
	if (m_ptrProgram) {
		COMPV_CHECK_CODE_RETURN(m_ptrProgram->useEnd());
	}

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVBlitterGL::setMVP(CompVMVPPtr mvp)
{
	COMPV_CHECK_EXP_RETURN(!mvp, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	m_ptrMVP = mvp;
	if (m_bInit && m_bMVP) {
		COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::haveCurrentContext(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);
		glUniformMatrix4fv(m_uNamePrgUnifMVP, 1, GL_FALSE, mvp->matrix()->ptr());
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVBlitterGL::setSize(size_t width, size_t height, size_t stride)
{
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::haveCurrentContext(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);

	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVBlitterGL::init(size_t width, size_t height, size_t stride, const std::string& prgVertexData, const std::string& prgFragData, bool bMVP /*= false*/, bool bToScreen /*= false*/)
{
	if (m_bInit) {
		return COMPV_ERROR_CODE_S_OK;
	}
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::haveCurrentContext(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);
	COMPV_CHECK_EXP_RETURN(!width || !height || stride < width || prgVertexData.empty() || prgFragData.empty(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	m_bInit = true; // Make sure deInit() will be executed if this function fails
	GLfloat uMax, vMax;

	// Model-View-Projection
	if (!bMVP) {
		m_ptrMVP = NULL;
	}
	else if (!m_ptrMVP) {
		COMPV_CHECK_CODE_RETURN(CompVMVP::newObjProjection2D(&m_ptrMVP));
	}

#if COMPV_VAO // FIXME(dmi): VAO
	// TODO(dmi): use GLUtils
	if (!m_uNameVAO) {
		glGenVertexArrays(1, &m_uNameVAO);
		if (!m_uNameVAO) {
			std::string errString;
			COMPV_CHECK_CODE_BAIL(err = CompVUtilsGL::getLastError(&errString));
			if (!errString.empty()) {
				COMPV_DEBUG_ERROR("Failed to create vao: %s", errString.c_str());
				COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_E_GL);
			}
		}
	}
	glBindVertexArray(m_uNameVAO);
#endif

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
	uMax = static_cast<GLfloat>(width) / static_cast<GLfloat>(stride);
	vMax = 1.f;	
	if (bToScreen) {
		COMPV_CHECK_EXP_RETURN(sizeof(m_Vertices) != sizeof(CompVGLScreenVertices), COMPV_ERROR_CODE_E_SYSTEM);
		memcpy(&m_Vertices, CompVGLScreenVertices, sizeof(CompVGLScreenVertices));
		m_Vertices[0].TexCoord[0] = uMax, m_Vertices[0].TexCoord[1] = 0.f;
		m_Vertices[1].TexCoord[0] = uMax, m_Vertices[0].TexCoord[1] = vMax;
		m_Vertices[2].TexCoord[0] = 0.f, m_Vertices[0].TexCoord[1] = vMax;
		m_Vertices[3].TexCoord[0] = 0.f, m_Vertices[0].TexCoord[1] = 0.f;
	}
	else {
		COMPV_CHECK_EXP_RETURN(sizeof(m_Vertices) != sizeof(CompVGLTexture2DVertices), COMPV_ERROR_CODE_E_SYSTEM);
		memcpy(&m_Vertices, CompVGLTexture2DVertices, sizeof(CompVGLTexture2DVertices));
		m_Vertices[0].TexCoord[0] = uMax, m_Vertices[0].TexCoord[1] = vMax;
		m_Vertices[1].TexCoord[0] = uMax, m_Vertices[0].TexCoord[1] = 0.f;
		m_Vertices[2].TexCoord[0] = 0.f, m_Vertices[0].TexCoord[1] = 0.f;
		m_Vertices[3].TexCoord[0] = 0.f, m_Vertices[0].TexCoord[1] = vMax;
	}
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

	COMPV_CHECK_CODE_BAIL(CompVProgramGL::newObj(&m_ptrProgram));
	COMPV_CHECK_EXP_BAIL(prgVertexData.empty(), err = COMPV_ERROR_CODE_E_GL);
	COMPV_CHECK_EXP_BAIL(prgFragData.empty(), err = COMPV_ERROR_CODE_E_GL);
	COMPV_CHECK_CODE_BAIL(err = m_ptrProgram->shadAttachVertexData(prgVertexData.c_str(), prgVertexData.length()));
	COMPV_CHECK_CODE_BAIL(err = m_ptrProgram->shadAttachFragmentData(prgFragData.c_str(), prgFragData.length()));
	COMPV_CHECK_CODE_BAIL(err = m_ptrProgram->link());
	COMPV_CHECK_CODE_ASSERT(err = m_ptrProgram->useBegin());

	m_uNamePrgAttPosition = glGetAttribLocation(m_ptrProgram->id(), "position");
	m_uNamePrgAttTexCoord = glGetAttribLocation(m_ptrProgram->id(), "texCoord");
	if (bMVP) {
		m_uNamePrgUnifMVP = glGetUniformLocation(m_ptrProgram->id(), "MVP");
	}
	glBindBuffer(GL_ARRAY_BUFFER, m_uNameVertexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uNameIndiceBuffer);
	glEnableVertexAttribArray(m_uNamePrgAttPosition);
	glEnableVertexAttribArray(m_uNamePrgAttTexCoord);
	glVertexAttribPointer(m_uNamePrgAttPosition, 3, GL_FLOAT, GL_FALSE, sizeof(CompVGLVertex), 0);
	glVertexAttribPointer(m_uNamePrgAttTexCoord, 2, GL_FLOAT, GL_FALSE, sizeof(CompVGLVertex), (GLvoid*)(sizeof(GLfloat) * 3));
	if (bMVP) {
		COMPV_DEBUG_INFO_CODE_FOR_TESTING();
		// Set aspect ratio
		//float arX = static_cast<float>(width) / static_cast<float>(height);
		//float arY = static_cast<float>(height) / static_cast<float>(width);
		//COMPV_CHECK_CODE_BAIL(err = m_ptrMVP->projection()->setAspectRatio(arX));
		//COMPV_CHECK_CODE_BAIL(err = m_ptrMVP->model()->matrix()->scale(CompVDrawingVec3f(1.f/arX, 1.f/arY, 1.f)));
		glUniformMatrix4fv(m_uNamePrgUnifMVP, 1, GL_FALSE, m_ptrMVP->matrix()->ptr());
	}

	m_nWidth = width;
	m_nHeight = height;
	m_nStride = stride;
	m_bToScreen = bToScreen;
	m_bMVP = bMVP;

bail:
	if (m_ptrProgram) {
		COMPV_CHECK_CODE_ASSERT(m_ptrProgram->useEnd());
	}
#if COMPV_VAO // FIXME(dmi): VAO
	glBindVertexArray(0);
#endif
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	if (COMPV_ERROR_CODE_IS_NOK(err)) {
		COMPV_CHECK_CODE_ASSERT(deInit());
		m_bInit = false;
	}

	return err;
}

COMPV_ERROR_CODE CompVBlitterGL::deInit()
{
	if (!m_bInit) {
		return COMPV_ERROR_CODE_S_OK;
	}
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::haveCurrentContext(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);
	if (m_uNameVertexBuffer) {
		glDeleteBuffers(1, &m_uNameVertexBuffer);
		m_uNameVertexBuffer = 0;
	}
	if (m_uNameIndiceBuffer) {
		glDeleteBuffers(1, &m_uNameIndiceBuffer);
		m_uNameIndiceBuffer = 0;
	}
	if (m_uNameVAO) {
#if COMPV_VAO // FIXME(dmi): VAO
		glDeleteVertexArrays(1, &m_uNameVAO);
#else
		COMPV_DEBUG_ERROR("WTF");
#endif
		m_uNameVAO = 0;
	}
	m_ptrProgram = NULL;

	m_bInit = false;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */
