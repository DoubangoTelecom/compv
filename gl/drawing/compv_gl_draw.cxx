/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/gl/drawing/compv_gl_draw.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/gl/compv_gl_vao.h"
#include "compv/gl/compv_gl_utils.h"
#include "compv/gl/compv_gl_info.h"
#include "compv/gl/compv_gl_func.h"

COMPV_NAMESPACE_BEGIN()

CompVGLDraw::CompVGLDraw(const std::string& strProgramVertexData, const std::string& strProgramFragmentData)
	: m_bInitialized(false)
	, m_uNameVAO(kCompVGLNameInvalid)
	, m_uNameVBO(kCompVGLNameInvalid)
	, m_strProgramVertexData(strProgramVertexData)
	, m_strProgramFragmentData(strProgramFragmentData)
{

}

CompVGLDraw::~CompVGLDraw()
{
	COMPV_CHECK_CODE_NOP(deInit());
}


COMPV_ERROR_CODE CompVGLDraw::bind() /*Overrides(CompVBind)*/
{
	COMPV_CHECK_EXP_RETURN(!CompVGLUtils::currentContext(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT, "No OpenGL context");
	COMPV_CHECK_CODE_RETURN(init());

	COMPV_ERROR_CODE err;
	COMPV_CHECK_CODE_BAIL(err = m_ptrProgram->bind());

	if (CompVGLInfo::extensions::vertex_array_object()) {
		COMPV_glBindVertexArray(m_uNameVAO);
		// We've to bind to the VBO even after binding to the VA
		// because not data was submited when VBO was created and the child class
		// will probably call 'COMPV_glBufferData'
		COMPV_glBindBuffer(GL_ARRAY_BUFFER, m_uNameVBO);
	}
	else {
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("OpenGL: VAO not suported");
		COMPV_glBindBuffer(GL_ARRAY_BUFFER, m_uNameVBO);
#if 0
		COMPV_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uNameIndiceBuffer);
		COMPV_glEnableVertexAttribArray(m_uNamePrgAttPosition);
		COMPV_glEnableVertexAttribArray(m_uNamePrgAttTexCoord);
		COMPV_glVertexAttribPointer(m_uNamePrgAttPosition, 3, GL_FLOAT, GL_FALSE, sizeof(CompVGLVertex), 0);
		COMPV_glVertexAttribPointer(m_uNamePrgAttTexCoord, 2, GL_FLOAT, GL_FALSE, sizeof(CompVGLVertex), (GLvoid*)(sizeof(GLfloat) * 3));
#endif
	}

bail:
	if (COMPV_ERROR_CODE_IS_NOK(err)) {
		COMPV_CHECK_CODE_NOP(unbind());
	}
	return err;
}
	
COMPV_ERROR_CODE CompVGLDraw::unbind() /*Overrides(CompVBind)*/
{
	COMPV_CHECK_EXP_RETURN(!CompVGLUtils::currentContext(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT, "No OpenGL context");
	COMPV_CHECK_EXP_RETURN(!m_bInitialized, COMPV_ERROR_CODE_E_INVALID_STATE, "Not initialized");

	COMPV_ERROR_CODE err;

	if (CompVGLInfo::extensions::vertex_array_object()) {
		COMPV_glBindVertexArray(kCompVGLNameInvalid);
	}
	else {
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("OpenGL: VAO not suported");
		COMPV_glBindBuffer(GL_ARRAY_BUFFER, kCompVGLNameInvalid);
		COMPV_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, kCompVGLNameInvalid);
	}

	COMPV_CHECK_CODE_NOP(err = m_ptrProgram->unbind());

	return err;
}

COMPV_ERROR_CODE CompVGLDraw::init()
{
	if (m_bInitialized) {
		return COMPV_ERROR_CODE_S_OK;
	}
	COMPV_ERROR_CODE err;
	COMPV_CHECK_EXP_BAIL(!CompVGLUtils::currentContext(), (err = COMPV_ERROR_CODE_E_GL_NO_CONTEXT), "No OpenGL context");
	m_bInitialized = true; // to make sure we'll have deInit() with a context if init fail

	// VAO
	if (CompVGLInfo::extensions::vertex_array_object()) {
		COMPV_CHECK_CODE_BAIL(err = CompVGLUtils::vertexArraysGen(&m_uNameVAO));
		COMPV_glBindBuffer(GL_ARRAY_BUFFER, m_uNameVBO); // required to have next VBO part of the VAO
	}
	// VBO
	COMPV_CHECK_CODE_BAIL(err = CompVGLUtils::bufferGen(&m_uNameVBO));
	COMPV_glBindBuffer(GL_ARRAY_BUFFER, m_uNameVBO); // not required
	// ... submit data to the VBO if needed

	// PROGRAM
	COMPV_CHECK_CODE_BAIL(err = CompVGLProgram::newObj(&m_ptrProgram, m_strProgramVertexData.c_str(), m_strProgramVertexData.length(), m_strProgramFragmentData.c_str(), m_strProgramFragmentData.length()));

	m_bInitialized = true;

bail:
	COMPV_glBindBuffer(GL_ARRAY_BUFFER, kCompVGLNameInvalid);
	COMPV_glBindVertexArray(kCompVGLNameInvalid);
	if (COMPV_ERROR_CODE_IS_NOK(err)) {
		COMPV_CHECK_CODE_NOP(deInit());
	}
	return err;
}
	
COMPV_ERROR_CODE CompVGLDraw::deInit()
{
	if (!m_bInitialized) {
		return COMPV_ERROR_CODE_S_OK;
	}
	COMPV_ERROR_CODE err;
	COMPV_CHECK_EXP_BAIL(!CompVGLUtils::currentContext(), (err = COMPV_ERROR_CODE_E_GL_NO_CONTEXT), "No OpenGL context");
	COMPV_CHECK_CODE_NOP(err = CompVGLUtils::bufferDelete(&m_uNameVBO));
	COMPV_CHECK_CODE_NOP(err = CompVGLUtils::vertexArraysDelete(&m_uNameVAO));

bail:
	return err;
}

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */
