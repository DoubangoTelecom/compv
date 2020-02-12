/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/gl/compv_gl_program.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/gl/compv_gl.h"
#include "compv/gl/compv_gl_utils.h"
#include "compv/gl/compv_gl_func.h"
#include "compv/base/compv_fileutils.h"

#define kShadTypeIsVertexTrue	true
#define kShadTypeIsVertexFalse	false
#define kModuleNameGLProgram	"GLProgram"

COMPV_NAMESPACE_BEGIN()

CompVGLProgram::CompVGLProgram()
    : m_uNameShaderVertex(0)
    , m_uNameShaderFragment(0)
    , m_uNamePrg(0)
    , m_bLinked(false)
    , m_bUsed(false)
{

}

CompVGLProgram::~CompVGLProgram()
{
    CompVGLUtils::shaderDelete(&m_uNameShaderVertex);
    CompVGLUtils::shaderDelete(&m_uNameShaderFragment);
    CompVGLUtils::programDelete(&m_uNamePrg);
}

COMPV_ERROR_CODE CompVGLProgram::shaderAttachVertexFile(const char* pcFilePath)
{
    CompVBufferPtr buff_;
    COMPV_CHECK_CODE_RETURN(CompVFileUtils::read(pcFilePath, &buff_));
    COMPV_CHECK_CODE_RETURN(CompVGLProgram::shaderAttachData(reinterpret_cast<const char*>(buff_->ptr()), buff_->size(), kShadTypeIsVertexTrue));
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLProgram::shaderAttachFragmentFile(const char* pcFilePath)
{
    CompVBufferPtr buff_;
    COMPV_CHECK_CODE_RETURN(CompVFileUtils::read(pcFilePath, &buff_));
    COMPV_CHECK_CODE_RETURN(CompVGLProgram::shaderAttachData(reinterpret_cast<const char*>(buff_->ptr()), buff_->size(), kShadTypeIsVertexFalse));
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLProgram::shaderAttachVertexData(const char* dataPtr, size_t dataLength)
{
    COMPV_CHECK_CODE_RETURN(CompVGLProgram::shaderAttachData(dataPtr, dataLength, kShadTypeIsVertexTrue));
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLProgram::shaderAttachFragmentData(const char* dataPtr, size_t dataLength)
{
    COMPV_CHECK_CODE_RETURN(CompVGLProgram::shaderAttachData(dataPtr, dataLength, kShadTypeIsVertexFalse));
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLProgram::link()
{
	COMPV_CHECK_EXP_RETURN((m_bUsed || m_bLinked), COMPV_ERROR_CODE_E_INVALID_STATE, "Program in use or already linked");
	COMPV_CHECK_EXP_RETURN(!CompVGLUtils::isShaderValid(m_uNameShaderVertex), COMPV_ERROR_CODE_E_INVALID_STATE, "No vertex shaders");
	COMPV_CHECK_EXP_RETURN(!CompVGLUtils::isShaderValid(m_uNameShaderFragment), COMPV_ERROR_CODE_E_INVALID_STATE, "No fragment shaders");
    COMPV_CHECK_CODE_RETURN(CompVGLUtils::programLink(m_uNamePrg));
    m_bLinked = true;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLProgram::bind() /*Overrides(CompVBind)*/
{
	COMPV_CHECK_EXP_RETURN(!m_bLinked, COMPV_ERROR_CODE_E_INVALID_STATE, "Program not linked");
    // do not check "m_bUsed", program could be used from different threads several times to make it current
    COMPV_CHECK_CODE_RETURN(CompVGLUtils::programBind(m_uNamePrg));
    m_bUsed = true;
    return COMPV_ERROR_CODE_S_OK;
}


COMPV_ERROR_CODE CompVGLProgram::unbind() /*Overrides(CompVBind)*/
{
	COMPV_CHECK_EXP_RETURN(!m_bLinked, COMPV_ERROR_CODE_E_INVALID_STATE, "Program not linked");
    // do not check "m_bUsed", program could be used from different threads several times to make it current
    COMPV_CHECK_CODE_RETURN(CompVGLUtils::programUnbind(m_uNamePrg));
    m_bUsed = false;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLProgram::shaderAttachData(const char* dataPtr, size_t dataLength, bool vertexType)
{
    COMPV_CHECK_EXP_RETURN(!dataPtr || !dataLength, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    GLuint newShad_ = 0;
    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
    const GLchar *data_;
    GLint length_;

    COMPV_CHECK_CODE_BAIL(err_ = vertexType ? CompVGLUtils::shaderGenVert(&newShad_) : CompVGLUtils::shaderGenFrag(&newShad_));
    data_ = static_cast<const GLchar *>(dataPtr);
    length_ = static_cast<GLint>(dataLength);
    COMPV_CHECK_CODE_BAIL(err_ = CompVGLUtils::shaderSetSource(newShad_, 1, &data_, &length_));
    COMPV_CHECK_CODE_BAIL(err_ = CompVGLUtils::shaderCompile(newShad_));
    COMPV_CHECK_CODE_BAIL(err_ = CompVGLUtils::shaderAttach(m_uNamePrg, newShad_));

bail:
    if (COMPV_ERROR_CODE_IS_OK(err_)) {
        GLuint* shad_ = vertexType ? &m_uNameShaderVertex : &m_uNameShaderFragment;
        if (CompVGLUtils::isShaderValid(*shad_)) {
            COMPV_CHECK_CODE_ASSERT(CompVGLUtils::shaderDelete(shad_));
        }
        *shad_ = newShad_;
    }
    else {
        CompVGLUtils::shaderDelete(&newShad_);
    }
    return err_;
}

COMPV_ERROR_CODE CompVGLProgram::newObj(CompVGLProgramPtrPtr program)
{
    COMPV_CHECK_EXP_RETURN(!program, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVGLProgramPtr program_ = new CompVGLProgram();
    COMPV_CHECK_EXP_RETURN(!program_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    COMPV_CHECK_CODE_RETURN(CompVGLUtils::programGen(&program_->m_uNamePrg));
    *program = program_;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLProgram::newObj(CompVGLProgramPtrPtr program, const char* vertexDataPtr, size_t vertexDataLength, const char* fragmentDataPtr, size_t fragmentDataLength)
{
    COMPV_CHECK_EXP_RETURN(!program || !vertexDataPtr || !vertexDataLength || !fragmentDataPtr || !fragmentDataLength, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVGLProgramPtr program_;
    COMPV_CHECK_CODE_RETURN(CompVGLProgram::newObj(&program_));
    COMPV_CHECK_CODE_RETURN(program_->shaderAttachVertexData(vertexDataPtr, vertexDataLength));
    COMPV_CHECK_CODE_RETURN(program_->shaderAttachFragmentData(fragmentDataPtr, fragmentDataLength));
    COMPV_CHECK_CODE_RETURN(program_->link());
    *program = program_;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLProgram::newObj(CompVGLProgramPtrPtr program, const char* vertexFilePath, const char* fragmentFilePath)
{
    COMPV_CHECK_EXP_RETURN(!program || !vertexFilePath || !fragmentFilePath, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVGLProgramPtr program_;
    COMPV_CHECK_CODE_RETURN(CompVGLProgram::newObj(&program_));
    COMPV_CHECK_CODE_RETURN(program_->shaderAttachVertexFile(vertexFilePath));
    COMPV_CHECK_CODE_RETURN(program_->shaderAttachFragmentFile(fragmentFilePath));
    COMPV_CHECK_CODE_RETURN(program_->link());
    *program = program_;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */
