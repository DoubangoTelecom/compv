/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/gl/compv_gl_utils.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/gl/compv_gl_func.h"
#include "compv/gl/compv_gl_info.h"
#include "compv/base/compv_debug.h"
#include "compv/base/compv_fileutils.h"
#include "compv/base/compv_mem.h"

#define COMPV_THIS_CLASSNAME	"CompVGLUtils"

COMPV_NAMESPACE_BEGIN()

void* CompVGLUtils::currentContext()
{
#if defined(HAVE_EGL)
    EGLContext ctx = eglGetCurrentContext();
    if (ctx == EGL_NO_CONTEXT) {
        return NULL;
    }
    return static_cast<void*>(ctx);
#else
#	if COMPV_OS_WINDOWS
    return static_cast<void*>(wglGetCurrentContext());
#	elif COMPV_OS_IPHONE || COMPV_OS_IPHONE_SIMULATOR
    COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "EAGL not implemented yet");
    return NULL;
#	elif COMPV_OS_APPLE
    return static_cast<void*>(aglGetCurrentContext());
#	else
    return static_cast<void*>(glXGetCurrentContext());
#	endif
#endif
}

COMPV_ERROR_CODE CompVGLUtils::lastError(std::string *error)
{
    //!\ You must *not* call any "COMPV_gl*" function here as they recursively call this function.
    // Doing so will lead to a stack overflow crash.
    // issue.
    GLenum err;
    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
    char buff_[33] = { 0 };
    while ((err = glGetError()) != GL_NO_ERROR) {
        if (error) {
#if defined(HAVE_GL_GLU_H) || defined(HAVE_GLU_H) || defined(__glu_h__) || defined(__GLU_H__)
            const char* str = reinterpret_cast<const char*>(gluErrorString(err));
            if (str) {
                *error += "\n" + std::string(str);
            }
            else {
                snprintf(buff_, sizeof(buff_), "%d", static_cast<int>(err));
                *error += "\ncode:" + std::string(buff_);
                // err_ = COMPV_ERROR_CODE_E_GL;
            }
#else
            snprintf(buff_, sizeof(buff_), "%d", static_cast<int>(err));
            *error += "\n" + std::string(buff_);
#endif
        }
    }
    return err_;
}

COMPV_ERROR_CODE CompVGLUtils::checkLastError()
{
    std::string errString_;
    COMPV_CHECK_CODE_RETURN(CompVGLUtils::lastError(&errString_));
    if (!errString_.empty()) {
        COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "OpenGL error: %s", errString_.c_str());
        COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_GL);
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLUtils::bufferGen(GLuint* uBuffer)
{
    COMPV_CHECK_EXP_RETURN(!uBuffer, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    COMPV_glGenBuffers(1, uBuffer);
    COMPV_DEBUG_VERBOSE_EX(COMPV_THIS_CLASSNAME, "glGenBuffers returned %u", *uBuffer);
    if (!CompVGLUtils::isBufferValid(*uBuffer)) {
        std::string errString;
        COMPV_CHECK_CODE_RETURN(CompVGLUtils::lastError(&errString));
        if (!errString.empty()) {
            COMPV_DEBUG_ERROR("Failed to create vertex buffer: %s", errString.c_str());
            COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_GL);
        }
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLUtils::bufferDelete(GLuint* uBuffer)
{
    COMPV_CHECK_EXP_RETURN(!uBuffer, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    if (*uBuffer) {
        COMPV_CHECK_EXP_RETURN(!CompVGLUtils::isBufferValid(*uBuffer), COMPV_ERROR_CODE_E_GL);
        COMPV_glDeleteBuffers(1, uBuffer);
        *uBuffer = kCompVGLNameInvalid;
    }
    return COMPV_ERROR_CODE_S_OK;
}

bool CompVGLUtils::isBufferValid(GLuint uBuffer)
{
    return uBuffer && glIsBuffer(uBuffer);
}

COMPV_ERROR_CODE CompVGLUtils::renderBufferGen(GLuint* uRenderBuffer)
{
    COMPV_CHECK_EXP_RETURN(!uRenderBuffer, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    COMPV_glGenRenderbuffers(1, uRenderBuffer);
	COMPV_DEBUG_VERBOSE_EX(COMPV_THIS_CLASSNAME, "glGenRenderbuffers returned %u", *uRenderBuffer);
    if (!CompVGLUtils::isRenderBufferValid(*uRenderBuffer)) {
        std::string errString;
        COMPV_CHECK_CODE_RETURN(CompVGLUtils::lastError(&errString));
        if (!errString.empty()) {
            COMPV_DEBUG_ERROR("Failed to create render buffer: %s", errString.c_str());
            COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_GL);
        }
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLUtils::renderBufferDelete(GLuint* uRenderBuffer)
{
    COMPV_CHECK_EXP_RETURN(!uRenderBuffer, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    if (*uRenderBuffer) {
        COMPV_CHECK_EXP_RETURN(!CompVGLUtils::isRenderBufferValid(*uRenderBuffer), COMPV_ERROR_CODE_E_GL);
        COMPV_glDeleteRenderbuffers(1, uRenderBuffer);
        *uRenderBuffer = kCompVGLNameInvalid;
    }
    return COMPV_ERROR_CODE_S_OK;
}

bool CompVGLUtils::isRenderBufferValid(GLuint uRenderBuffer)
{
    return uRenderBuffer && glIsRenderbuffer(uRenderBuffer);
}

COMPV_ERROR_CODE CompVGLUtils::frameBufferGen(GLuint* uFrameBuffer)
{
    COMPV_CHECK_EXP_RETURN(!uFrameBuffer, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    COMPV_glGenFramebuffers(1, uFrameBuffer);
	COMPV_DEBUG_VERBOSE_EX(COMPV_THIS_CLASSNAME, "glGenFramebuffers returned %u", *uFrameBuffer);
    if (!CompVGLUtils::isFrameBufferValid(*uFrameBuffer)) {
        std::string errString;
        COMPV_CHECK_CODE_RETURN(CompVGLUtils::lastError(&errString));
        if (!errString.empty()) {
            COMPV_DEBUG_ERROR("Failed to create frame buffer: %s", errString.c_str());
            COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_GL);
        }
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLUtils::frameBufferDelete(GLuint* uFrameBuffer)
{
    COMPV_CHECK_EXP_RETURN(!uFrameBuffer, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    if (*uFrameBuffer) {
        COMPV_CHECK_EXP_RETURN(!CompVGLUtils::isFrameBufferValid(*uFrameBuffer), COMPV_ERROR_CODE_E_GL);
        COMPV_glDeleteFramebuffers(1, uFrameBuffer);
        *uFrameBuffer = kCompVGLNameInvalid;
    }
    return COMPV_ERROR_CODE_S_OK;
}

bool CompVGLUtils::isFrameBufferValid(GLuint uFrameBuffer)
{
    return uFrameBuffer && glIsFramebuffer(uFrameBuffer);
}

COMPV_ERROR_CODE CompVGLUtils::vertexArraysGen(GLuint* uVertexArrays)
{
    COMPV_CHECK_EXP_RETURN(!uVertexArrays, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    COMPV_CHECK_EXP_RETURN(!CompVGLInfo::extensions::vertex_array_object(), COMPV_ERROR_CODE_E_GL, "Vertex Array not supported");
    COMPV_glGenVertexArrays(1, uVertexArrays);
    if (!CompVGLUtils::isVertexArrays(*uVertexArrays)) {
        std::string errString;
        COMPV_CHECK_CODE_RETURN(CompVGLUtils::lastError(&errString));
        if (!errString.empty()) {
            COMPV_DEBUG_ERROR("Failed to create vertex arrays: %s", errString.c_str());
            COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_GL);
        }
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLUtils::vertexArraysDelete(GLuint* uVertexArrays)
{
    COMPV_CHECK_EXP_RETURN(!uVertexArrays, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    if (*uVertexArrays) {
        COMPV_CHECK_EXP_RETURN(!CompVGLUtils::isVertexArrays(*uVertexArrays), COMPV_ERROR_CODE_E_GL);
        COMPV_CHECK_EXP_RETURN(!CompVGLInfo::extensions::vertex_array_object(), COMPV_ERROR_CODE_E_GL);
        COMPV_glDeleteVertexArrays(1, uVertexArrays);
        *uVertexArrays = kCompVGLNameInvalid;
    }
    return COMPV_ERROR_CODE_S_OK;
}

bool CompVGLUtils::isVertexArrays(GLuint uVertexArrays)
{
    return uVertexArrays
#if defined(HAVE_OPENGL)
           && glIsVertexArray(uVertexArrays)
#endif
           ;
}

COMPV_ERROR_CODE CompVGLUtils::shaderGen(GLuint* uShader, GLenum shadType)
{
    COMPV_CHECK_EXP_RETURN(!uShader, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
    *uShader = kCompVGLNameInvalid;
    GLuint shader_ = COMPV_glCreateShader(shadType);
	COMPV_DEBUG_VERBOSE_EX(COMPV_THIS_CLASSNAME, "glCreateShader returned %u", shader_);
    if (!CompVGLUtils::isShaderValid(shader_)) {
        std::string errString_;
        COMPV_CHECK_CODE_BAIL(err_ = CompVGLUtils::lastError(&errString_));
        if (!errString_.empty()) {
            COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "glCreateShader failed: %s", errString_.c_str());
        }
        COMPV_CHECK_CODE_BAIL(err_ = COMPV_ERROR_CODE_E_GL);
    }
    *uShader = shader_;
    return COMPV_ERROR_CODE_S_OK;
bail:

    return err_;
}

COMPV_ERROR_CODE CompVGLUtils::shaderGenVert(GLuint* uShader)
{
    COMPV_CHECK_CODE_RETURN(CompVGLUtils::shaderGen(uShader, GL_VERTEX_SHADER));
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLUtils::shaderGenFrag(GLuint* uShader)
{
    COMPV_CHECK_CODE_RETURN(CompVGLUtils::shaderGen(uShader, GL_FRAGMENT_SHADER));
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLUtils::shaderDelete(GLuint* uShader)
{
    COMPV_CHECK_EXP_RETURN(!uShader, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    if (*uShader) {
        COMPV_CHECK_EXP_RETURN(!CompVGLUtils::isShaderValid(*uShader), COMPV_ERROR_CODE_E_GL);
        COMPV_glDeleteShader(*uShader);
        *uShader = kCompVGLNameInvalid;
    }
    return COMPV_ERROR_CODE_S_OK;
}

bool CompVGLUtils::isShaderValid(GLuint uShader)
{
    return uShader != kCompVGLNameInvalid && glIsShader(uShader) == GL_TRUE;
}

COMPV_ERROR_CODE CompVGLUtils::shaderSetSource(GLuint uShader, GLsizei count, const GLchar **string, const GLint *length)
{
    COMPV_CHECK_EXP_RETURN(!CompVGLUtils::isShaderValid(uShader) || count <= kCompVGLNameInvalid || !string || !*string, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    COMPV_glShaderSource(uShader, count, string, length);
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLUtils::shaderSetSource(GLuint uShader, const CompVBufferPtr& buff)
{
    COMPV_CHECK_EXP_RETURN(!CompVGLUtils::isShaderValid(uShader) || !buff || buff->isEmpty(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    const char* source_ = static_cast<const char*>(buff->ptr());
    const GLint length_ = static_cast<GLint>(buff->size());
    COMPV_CHECK_CODE_RETURN(CompVGLUtils::shaderSetSource(uShader, 1, &source_, &length_));
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLUtils::shaderSetSource(GLuint uShader, const char* filePath)
{
    COMPV_CHECK_EXP_RETURN(!CompVGLUtils::isShaderValid(uShader) || !filePath, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVBufferPtr buff_;
    COMPV_CHECK_CODE_RETURN(CompVFileUtils::read(filePath, &buff_));
    COMPV_CHECK_CODE_RETURN(CompVGLUtils::shaderSetSource(uShader, buff_));
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLUtils::shaderCompile(GLuint uShader)
{
    COMPV_CHECK_EXP_RETURN(!CompVGLUtils::isShaderValid(uShader), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    std::string errString_;
    COMPV_glCompileShader(uShader);
    COMPV_CHECK_CODE_RETURN(CompVGLUtils::shaderCompileGetStatus(uShader, &errString_));
    if (!errString_.empty()) {
        COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "glCompileShader failed: %s", errString_.c_str());
        COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_GL);
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLUtils::shaderCompileGetStatus(GLuint uShader, std::string *error)
{
    COMPV_CHECK_EXP_RETURN(!CompVGLUtils::isShaderValid(uShader) || !error, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    GLint ok_ = kCompVGLNameInvalid;
    COMPV_glGetShaderiv(uShader, GL_COMPILE_STATUS, &ok_);
    if (ok_ == GL_FALSE) {
        *error = "Unknown error";
        GLint maxLength = kCompVGLNameInvalid;
        COMPV_glGetShaderiv(uShader, GL_INFO_LOG_LENGTH, &maxLength);
        if (maxLength > 0) {
            GLchar* infoLog = static_cast<GLchar*>(CompVMem::malloc(maxLength + 1));
            if (infoLog) {
                infoLog[maxLength] = '\0';
                COMPV_glGetShaderInfoLog(uShader, maxLength, &maxLength, infoLog);
                *error = std::string((const char*)infoLog, maxLength);
                CompVMem::free((void**)&infoLog);
            }
            else {
                COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
            }
        }
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLUtils::shaderAttach(GLuint uProgram, GLuint uShader)
{
    COMPV_CHECK_EXP_RETURN(!CompVGLUtils::isProgramValid(uProgram) || !CompVGLUtils::isShaderValid(uShader), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    COMPV_glAttachShader(uProgram, uShader);
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLUtils::textureGen(GLuint* uTex)
{
    COMPV_CHECK_EXP_RETURN(!uTex, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    *uTex = kCompVGLNameInvalid;
    COMPV_glGenTextures(1, uTex); // returned value not texture yet until bind() is called
	COMPV_DEBUG_VERBOSE_EX(COMPV_THIS_CLASSNAME, "glGenTextures returned %u", *uTex);
    if (!*uTex) {
        std::string errString_;
        COMPV_CHECK_CODE_RETURN(CompVGLUtils::lastError(&errString_));
        if (!errString_.empty()) {
            COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "glGenTextures failed: %s", errString_.c_str());
        }
        COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_GL);
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLUtils::textureDelete(GLuint* uTex)
{
    // A texture is really a texture only after bind() -> do not use IsValid()
    if (uTex && *uTex) {
        COMPV_glDeleteTextures(1, uTex);
        *uTex = kCompVGLNameInvalid;
    }
    return COMPV_ERROR_CODE_S_OK;
}

bool CompVGLUtils::isTextureValid(GLuint uTex)
{
    return uTex != kCompVGLNameInvalid && glIsTexture(uTex) == GL_TRUE;
}

bool CompVGLUtils::isTexture2DEnabled()
{
    return (glIsEnabled(GL_TEXTURE_2D) == GL_TRUE);
}

COMPV_ERROR_CODE CompVGLUtils::texture2DGetCurrent(GLuint* uTex)
{
    COMPV_CHECK_EXP_RETURN(!uTex, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    GLint text_ = kCompVGLNameInvalid;
    COMPV_glGetIntegerv(GL_TEXTURE_BINDING_2D, &text_);
    *uTex = static_cast<GLuint>(text_);
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLUtils::texture2DSetCurrent(GLuint uTex, bool checkErr COMPV_DEFAULT(false))
{
    COMPV_CHECK_EXP_RETURN(!CompVGLUtils::isTextureValid(uTex), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    COMPV_glBindTexture(GL_TEXTURE_2D, uTex);
    if (checkErr) {
        std::string errString_;
        COMPV_CHECK_CODE_RETURN(CompVGLUtils::lastError(&errString_));
        if (!errString_.empty()) {
            COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "glBindTexture failed: %s", errString_.c_str());
            COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_GL);
        }
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLUtils::programGen(GLuint* uPrg)
{
    COMPV_CHECK_EXP_RETURN(!uPrg, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    *uPrg = glCreateProgram();
	COMPV_DEBUG_VERBOSE_EX(COMPV_THIS_CLASSNAME, "glCreateProgram returned %u", *uPrg);
    if (!CompVGLUtils::isProgramValid(*uPrg)) {
        std::string errString_;
        COMPV_CHECK_CODE_RETURN(CompVGLUtils::lastError(&errString_));
        if (!errString_.empty()) {
            COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "glCreateProgram failed: %s", errString_.c_str());
        }
        COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_GL);
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLUtils::programDelete(GLuint* uPrg)
{
    COMPV_CHECK_EXP_RETURN(!uPrg, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    if (*uPrg) {
        COMPV_CHECK_EXP_RETURN(!CompVGLUtils::isProgramValid(*uPrg), COMPV_ERROR_CODE_E_GL);
        COMPV_glDeleteProgram(*uPrg);
        *uPrg = kCompVGLNameInvalid;
    }
    return COMPV_ERROR_CODE_S_OK;
}

bool CompVGLUtils::isProgramValid(GLuint uPrg)
{
    return (uPrg != kCompVGLNameInvalid && glIsProgram(uPrg) == GL_TRUE);
}

bool CompVGLUtils::isProgramCurrent(GLuint uPrg)
{
    if (CompVGLUtils::isProgramValid(uPrg)) {
        GLuint prg_ = kCompVGLNameInvalid;
        COMPV_glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*)&prg_);
        return (prg_ == uPrg);
    }
    return false;
}

COMPV_ERROR_CODE CompVGLUtils::programLink(GLuint uPrg)
{
    COMPV_CHECK_EXP_RETURN(!CompVGLUtils::isProgramValid(uPrg), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    COMPV_glLinkProgram(uPrg);
    std::string errString_;
    COMPV_CHECK_CODE_RETURN(CompVGLUtils::programLinkGetStatus(uPrg, &errString_));
    if (!errString_.empty()) {
        COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Program link info log: %s", errString_.c_str());
        COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_GL);
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLUtils::programLinkGetStatus(GLuint uPrg, std::string *error)
{
    COMPV_CHECK_EXP_RETURN(!CompVGLUtils::isProgramValid(uPrg) || !error, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    GLint ok_ = kCompVGLNameInvalid;
    COMPV_glGetProgramiv(uPrg, GL_LINK_STATUS, (int *)&ok_);
    if (ok_ == GL_FALSE) {
        *error = "Unknown error";
        GLint maxLength = kCompVGLNameInvalid;
        COMPV_glGetProgramiv(uPrg, GL_INFO_LOG_LENGTH, &maxLength);
        if (maxLength > 0) {
            GLchar* infoLog = static_cast<GLchar*>(CompVMem::malloc(maxLength + 1));
            if (infoLog) {
                infoLog[maxLength] = '\0';
                COMPV_glGetProgramInfoLog(uPrg, maxLength, &maxLength, infoLog);
                *error = std::string((const char*)infoLog, maxLength);
            }
            else {
                COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
            }
        }
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLUtils::programBind(GLuint uPrg, bool checkErr COMPV_DEFAULT(false))
{
    COMPV_CHECK_EXP_RETURN(!CompVGLUtils::isProgramValid(uPrg), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    COMPV_glUseProgram(uPrg);
    if (checkErr) {
        std::string errString_;
        COMPV_CHECK_CODE_RETURN(CompVGLUtils::lastError(&errString_));
        if (!errString_.empty()) {
            COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "glUseProgram failed: %s", errString_.c_str());
            COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_GL);
        }
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLUtils::programUnbind(GLuint uPrg, bool checkErr COMPV_DEFAULT(false))
{
    COMPV_CHECK_EXP_RETURN(!CompVGLUtils::isProgramValid(uPrg), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    if (CompVGLUtils::isProgramCurrent(uPrg)) {
        COMPV_glUseProgram(0);
    }
    else {
        COMPV_DEBUG_WARN_EX(COMPV_THIS_CLASSNAME, "Program (%u) not in use", uPrg);
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLUtils::updateVertices(size_t width, size_t height, size_t stride, bool bToScreen, CompVGLVertex(*Vertices)[4])
{
    GLfloat uMax = static_cast<GLfloat>(width) / static_cast<GLfloat>(stride);
    GLfloat vMax = 1.f;
    if (bToScreen) {
        COMPV_CHECK_EXP_RETURN(sizeof(*Vertices) != sizeof(kCompVGLScreenVertices), COMPV_ERROR_CODE_E_SYSTEM);
        memcpy(&(*Vertices)[0], kCompVGLScreenVertices, sizeof(kCompVGLScreenVertices));
        (*Vertices)[0].TexCoord[0] = uMax, (*Vertices)[0].TexCoord[1] = 0.f;
        (*Vertices)[1].TexCoord[0] = uMax, (*Vertices)[0].TexCoord[1] = vMax;
        (*Vertices)[2].TexCoord[0] = 0.f, (*Vertices)[0].TexCoord[1] = vMax;
        (*Vertices)[3].TexCoord[0] = 0.f, (*Vertices)[0].TexCoord[1] = 0.f;
    }
    else {
        COMPV_CHECK_EXP_RETURN(sizeof(*Vertices) != sizeof(kCompVGLTexture2DVertices), COMPV_ERROR_CODE_E_SYSTEM);
        memcpy(&(*Vertices)[0], kCompVGLTexture2DVertices, sizeof(kCompVGLTexture2DVertices));
        (*Vertices)[0].TexCoord[0] = uMax, (*Vertices)[0].TexCoord[1] = vMax;
        (*Vertices)[1].TexCoord[0] = uMax, (*Vertices)[0].TexCoord[1] = 0.f;
        (*Vertices)[2].TexCoord[0] = 0.f, (*Vertices)[0].TexCoord[1] = 0.f;
        (*Vertices)[3].TexCoord[0] = 0.f, (*Vertices)[0].TexCoord[1] = vMax;
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_SUBTYPE CompVGLUtils::subType(const CompVMatPtr& image)
{
	if (image) {
		if (image->planeCount() == 1 && image->subType() == COMPV_SUBTYPE_RAW_UINT8) {
			return COMPV_SUBTYPE_PIXELS_Y;
		}
		return image->subType();
	}
	return COMPV_SUBTYPE_NONE;
}

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */
