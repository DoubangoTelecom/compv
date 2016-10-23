/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/ui/opengl/compv_program_gl.h"
#if defined(HAVE_GL_GLEW_H)
#include "compv/ui/compv_ui.h"
#include "compv/ui/opengl/compv_utils_gl.h"
#include "compv/base/compv_fileutils.h"

#define kShadTypeIsVertexTrue	true
#define kShadTypeIsVertexFalse	false
#define kModuleNameGLProgram	"GLProgram"

COMPV_NAMESPACE_BEGIN()

CompVProgramGL::CompVProgramGL()
: m_uPrg(0)
, m_uShadVertex(0)
, m_uShadFragment(0)
, m_bLinked(false)
, m_bUsed(false)
{

}

CompVProgramGL::~CompVProgramGL()
{
	CompVUtilsGL::shadDelete(&m_uShadVertex);
	CompVUtilsGL::shadDelete(&m_uShadFragment);
	CompVUtilsGL::prgDelete(&m_uPrg);
}

COMPV_ERROR_CODE CompVProgramGL::shadAttachVertexFile(const char* pcFilePath)
{
	CompVBufferPtr buff_;
	COMPV_CHECK_CODE_RETURN(CompVFileUtils::read(pcFilePath, &buff_));
	COMPV_CHECK_CODE_RETURN(shaderAttachData(reinterpret_cast<const char*>(buff_->getPtr()), buff_->getSize(), kShadTypeIsVertexTrue));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVProgramGL::shadAttachFragmentFile(const char* pcFilePath)
{
	CompVBufferPtr buff_;
	COMPV_CHECK_CODE_RETURN(CompVFileUtils::read(pcFilePath, &buff_));
	COMPV_CHECK_CODE_RETURN(shaderAttachData(reinterpret_cast<const char*>(buff_->getPtr()), buff_->getSize(), kShadTypeIsVertexFalse));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVProgramGL::shadAttachVertexData(const char* dataPtr, size_t dataLength)
{
	COMPV_CHECK_CODE_RETURN(shaderAttachData(dataPtr, dataLength, kShadTypeIsVertexTrue));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVProgramGL::shadAttachFragmentData(const char* dataPtr, size_t dataLength)
{
	COMPV_CHECK_CODE_RETURN(shaderAttachData(dataPtr, dataLength, kShadTypeIsVertexFalse));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVProgramGL::link()
{
	if (m_bUsed || m_bLinked) {
		COMPV_DEBUG_ERROR_EX(kModuleNameGLProgram, "Program in use or already linked");
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_STATE);
	}
	if (!CompVUtilsGL::shadIsValid(m_uShadVertex)) {
		COMPV_DEBUG_ERROR_EX(kModuleNameGLProgram, "No vertex shaders");
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_STATE);
	}
	if (!CompVUtilsGL::shadIsValid(m_uShadFragment)) {
		COMPV_DEBUG_ERROR_EX(kModuleNameGLProgram, "No fragment shaders");
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_STATE);
	}
	COMPV_CHECK_CODE_RETURN(CompVUtilsGL::prgLink(m_uPrg));
	m_bLinked = true;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVProgramGL::useBegin()
{
	if (!m_bLinked) {
		COMPV_DEBUG_ERROR_EX(kModuleNameGLProgram, "Program not linked");
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_STATE);
	}
	// do not check "m_bUsed", program could be used from different threads several times to make it current
	COMPV_CHECK_CODE_RETURN(CompVUtilsGL::prgUseBegin(m_uPrg));
	m_bUsed = true;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVProgramGL::useEnd()
{
	if (!m_bLinked) {
		COMPV_DEBUG_ERROR_EX(kModuleNameGLProgram, "Program not linked");
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_STATE);
	}
	// do not check "m_bUsed", program could be used from different threads several times to make it current
	COMPV_CHECK_CODE_RETURN(CompVUtilsGL::prgUseEnd(m_uPrg));
	m_bUsed = false;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVProgramGL::shaderAttachData(const char* dataPtr, size_t dataLength, bool vertexType)
{
	COMPV_CHECK_EXP_RETURN(!dataPtr || !dataLength, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	GLuint newShad_ = 0;
	COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;

	COMPV_CHECK_CODE_BAIL(err_ = vertexType ? CompVUtilsGL::shadCreateVert(&newShad_) : CompVUtilsGL::shadCreateFrag(&newShad_));
	const GLchar *data_ = static_cast<const GLchar *>(dataPtr);
	const GLint length_ = static_cast<GLint>(dataLength);
	COMPV_CHECK_CODE_BAIL(err_ = CompVUtilsGL::shadSetSource(newShad_, 1, &data_, &length_));
	COMPV_CHECK_CODE_BAIL(err_ = CompVUtilsGL::shadCompile(newShad_));
	COMPV_CHECK_CODE_BAIL(err_ = CompVUtilsGL::shadAttach(m_uPrg, newShad_));

bail:
	if (COMPV_ERROR_CODE_IS_OK(err_)) {
		GLuint* shad_ = vertexType ? &m_uShadVertex : &m_uShadFragment;
		if (CompVUtilsGL::shadIsValid(*shad_)) {
			COMPV_CHECK_CODE_ASSERT(CompVUtilsGL::shadDelete(shad_));
		}
		*shad_ = newShad_;
	}
	else {
		CompVUtilsGL::shadDelete(&newShad_);
	}
	return err_;
}

COMPV_ERROR_CODE CompVProgramGL::newObj(CompVProgramGLPtrPtr program)
{
	COMPV_CHECK_CODE_RETURN(CompVUI::init());
	COMPV_CHECK_EXP_RETURN(!program, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	GLuint newPrg_ = 0;
	COMPV_CHECK_CODE_RETURN(CompVUtilsGL::prgCreate(&newPrg_));
	CompVProgramGLPtr program_ = new CompVProgramGL();
	if (!program_) {
		CompVUtilsGL::prgDelete(&newPrg_);
	}
	program_->m_uPrg = newPrg_;

	*program = program_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* HAVE_GL_GLEW_H */
