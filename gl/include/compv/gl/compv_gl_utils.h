/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_GL_UTILS_H_)
#define _COMPV_GL_UTILS_H_

#include "compv/gl/compv_gl_config.h"
#include "compv/gl/compv_gl_headers.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/gl/compv_gl_common.h"
#include "compv/base/compv_buffer.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class COMPV_GL_API CompVGLUtils
{
public:
    static void* currentContext();
    static bool isGLContextSet() {
        return CompVGLUtils::currentContext() != NULL;
    }

    static COMPV_ERROR_CODE lastError(std::string *error);
    static COMPV_ERROR_CODE checkLastError();

    static COMPV_ERROR_CODE bufferGen(GLuint* uBuffer);
    static COMPV_ERROR_CODE bufferDelete(GLuint* uBuffer);
    static bool isBufferValid(GLuint uBuffer);

    static COMPV_ERROR_CODE renderBufferGen(GLuint* uRenderBuffer);
    static COMPV_ERROR_CODE renderBufferDelete(GLuint* uRenderBuffer);
    static bool isRenderBufferValid(GLuint uRenderBuffer);

    static COMPV_ERROR_CODE frameBufferGen(GLuint* uFrameBuffer);
    static COMPV_ERROR_CODE frameBufferDelete(GLuint* uFrameBuffer);
    static bool isFrameBufferValid(GLuint uFrameBuffer);

    static COMPV_ERROR_CODE vertexArraysGen(GLuint* uVertexArrays);
    static COMPV_ERROR_CODE vertexArraysDelete(GLuint* uVertexArrays);
    static bool isVertexArrays(GLuint uVertexArrays);

    static COMPV_ERROR_CODE shaderGen(GLuint* uShader, GLenum shaderType);
    static COMPV_ERROR_CODE shaderGenVert(GLuint* uShader);
    static COMPV_ERROR_CODE shaderGenFrag(GLuint* uShader);
    static COMPV_ERROR_CODE shaderDelete(GLuint* uShader);
    static bool isShaderValid(GLuint uShader);
    static COMPV_ERROR_CODE shaderSetSource(GLuint uShader, GLsizei count, const GLchar **string, const GLint *length);
    static COMPV_ERROR_CODE shaderSetSource(GLuint uShader, const CompVBufferPtr& buff);
    static COMPV_ERROR_CODE shaderSetSource(GLuint uShader, const char* filePath);
    static COMPV_ERROR_CODE shaderCompile(GLuint uShader);
    static COMPV_ERROR_CODE shaderCompileGetStatus(GLuint uShader, std::string *error);
    static COMPV_ERROR_CODE shaderAttach(GLuint uProgram, GLuint uShader);

    static COMPV_ERROR_CODE textureGen(GLuint* uTex);
    static COMPV_ERROR_CODE textureDelete(GLuint* uTex);
    static bool isTextureValid(GLuint uTex);
    static bool isTexture2DEnabled();
    static COMPV_ERROR_CODE texture2DGetCurrent(GLuint* uTex);
    static COMPV_ERROR_CODE texture2DSetCurrent(GLuint uTex, bool checkErr = false);

    static COMPV_ERROR_CODE programGen(GLuint* uProgram);
    static COMPV_ERROR_CODE programDelete(GLuint* uProgram);
    static bool isProgramValid(GLuint uProgram);
    static bool isProgramCurrent(GLuint uProgram);
    static COMPV_ERROR_CODE programLink(GLuint uProgram);
    static COMPV_ERROR_CODE programLinkGetStatus(GLuint uProgram, std::string *error);
    static COMPV_ERROR_CODE programBind(GLuint uProgram, bool checkErr = false);
    static COMPV_ERROR_CODE programUnbind(GLuint uProgram, bool checkErr = false);

    static COMPV_ERROR_CODE updateVertices(size_t width, size_t height, size_t stride, bool bToScreen, CompVGLVertex(*Vertices)[4]);
};

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

#endif /* _COMPV_GL_UTILS_H_ */
