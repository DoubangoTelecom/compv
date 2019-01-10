/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_GL_PROGRAM_H_)
#define _COMPV_GL_PROGRAM_H_

#include "compv/gl/compv_gl_config.h"
#include "compv/gl/compv_gl_headers.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/gl/compv_gl_utils.h"
#include "compv/base/compv_bind.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

#define COMPV_GL_PROGRAM_AUTOBIND(program) COMPV_AUTOBIND(CompVGLProgram, (program))

COMPV_OBJECT_DECLARE_PTRS(GLProgram)

class COMPV_GL_API CompVGLProgram : public CompVObj, public CompVBind
{
protected:
    CompVGLProgram();
public:
    virtual ~CompVGLProgram();
    COMPV_OBJECT_GET_ID(CompVGLProgram);
    COMPV_INLINE GLuint name() const{
        return m_uNamePrg;
    };
	COMPV_INLINE bool isBound() const {
		return m_bUsed;
	}

    COMPV_ERROR_CODE shaderAttachVertexFile(const char* pcFilePath);
    COMPV_ERROR_CODE shaderAttachFragmentFile(const char* pcFilePath);
    COMPV_ERROR_CODE shaderAttachVertexData(const char* dataPtr, size_t dataLength);
    COMPV_ERROR_CODE shaderAttachFragmentData(const char* dataPtr, size_t dataLength);
    COMPV_ERROR_CODE link();

    virtual COMPV_ERROR_CODE bind() override /*Overrides(CompVBind)*/;
	virtual COMPV_ERROR_CODE unbind() override /*Overrides(CompVBind)*/;

    static COMPV_ERROR_CODE newObj(CompVGLProgramPtrPtr program);
    static COMPV_ERROR_CODE newObj(CompVGLProgramPtrPtr program, const char* vertexDataPtr, size_t vertexDataLength, const char* fragmentDataPtr, size_t fragmentDataLength);
    static COMPV_ERROR_CODE newObj(CompVGLProgramPtrPtr program, const char* vertexFilePath, const char* fragmentFilePath);

private:
    COMPV_ERROR_CODE shaderAttachData(const char* dataPtr, size_t dataLength, bool vertexType);

private:
    GLuint m_uNameShaderVertex;
    GLuint m_uNameShaderFragment;
    GLuint m_uNamePrg;
    bool m_bLinked;
    bool m_bUsed;
};

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

#endif /* _COMPV_GL_PROGRAM_H_ */
