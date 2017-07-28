/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/gl/compv_gl_blitter.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/gl/compv_gl_utils.h"
#include "compv/gl/compv_gl_info.h"
#include "compv/gl/compv_gl_func.h"
#include "compv/gl/compv_gl_mvp.h"

#define COMPV_THIS_CLASSNAME	"CompVGLBlitter"

#if !defined(GL_INVALID_INDEX)
#	define GL_INVALID_INDEX 0xFFFFFFFFu
#endif

COMPV_NAMESPACE_BEGIN()

CompVGLBlitter::CompVGLBlitter()
    : m_bInit(false)
    , m_bToScreen(false)
    , m_nWidth(0)
    , m_nHeight(0)
    , m_nStride(0)
    , m_uNameVertexBuffer(kCompVGLNameInvalid)
    , m_uNameIndiceBuffer(kCompVGLNameInvalid)
    , m_uNamePrgAttPosition(kCompVGLNameInvalid)
    , m_uNamePrgAttTexCoord(kCompVGLNameInvalid)
    , m_uNamePrgUnifMVP(kCompVGLNameInvalid)
    , m_uNameVAO(kCompVGLNameInvalid)
{

}

CompVGLBlitter::~CompVGLBlitter()
{
    COMPV_CHECK_CODE_NOP(deInit());
}

// Bind to VAO and activate the program
COMPV_ERROR_CODE CompVGLBlitter::bind()  /*Overrides(CompVBind)*/
{
    COMPV_CHECK_EXP_RETURN(!CompVGLUtils::isGLContextSet(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);
    COMPV_CHECK_EXP_RETURN(!m_bInit || !m_ptrProgram, COMPV_ERROR_CODE_E_INVALID_STATE);
    if (m_ptrFBO) {
        COMPV_CHECK_CODE_RETURN(m_ptrFBO->bind());
    }
    COMPV_CHECK_CODE_RETURN(m_ptrProgram->bind());

    // Because MVP could be dirty we have to send the data again
    // FIXME(dmi): find a way to detect that MVP is dirty
    if (m_bMVP && m_ptrMVP) {
        COMPV_glUniformMatrix4fv(m_uNamePrgUnifMVP, 1, GL_FALSE, m_ptrMVP->matrix()->ptr());
    }

    if (CompVGLInfo::extensions::vertex_array_object()) {
        COMPV_glBindVertexArray(m_uNameVAO);
    }
    else {
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("OpenGL: VAO not suported");
        COMPV_glBindBuffer(GL_ARRAY_BUFFER, m_uNameVertexBuffer);
        COMPV_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uNameIndiceBuffer);
        COMPV_glEnableVertexAttribArray(m_uNamePrgAttPosition);
        COMPV_glEnableVertexAttribArray(m_uNamePrgAttTexCoord);
        COMPV_glVertexAttribPointer(m_uNamePrgAttPosition, 3, GL_FLOAT, GL_FALSE, sizeof(CompVGLVertex), reinterpret_cast<const GLvoid *>(offsetof(CompVGLVertex, Position)));
        COMPV_glVertexAttribPointer(m_uNamePrgAttTexCoord, 2, GL_FLOAT, GL_FALSE, sizeof(CompVGLVertex), reinterpret_cast<const GLvoid *>(offsetof(CompVGLVertex, TexCoord)));
    }

    return COMPV_ERROR_CODE_S_OK;
}

// Unbind the VAO and deactivate the program
COMPV_ERROR_CODE CompVGLBlitter::unbind()  /*Overrides(CompVBind)*/
{
    COMPV_CHECK_EXP_RETURN(!CompVGLUtils::isGLContextSet(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);

    if (CompVGLInfo::extensions::vertex_array_object()) {
        COMPV_glBindVertexArray(kCompVGLNameInvalid);
    }
    else {
        COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("OpenGL: VAO not suported");
        COMPV_glBindBuffer(GL_ARRAY_BUFFER, kCompVGLNameInvalid);
        COMPV_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, kCompVGLNameInvalid);
    }

    if (m_ptrProgram) {
        COMPV_CHECK_CODE_NOP(m_ptrProgram->unbind());
    }
    if (m_ptrFBO) {
        COMPV_CHECK_CODE_NOP(m_ptrFBO->bind());
    }

    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLBlitter::setMVP(CompVMVPPtr mvp)
{
    COMPV_CHECK_EXP_RETURN(!mvp, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    m_ptrMVP = mvp;
    if (m_bInit && m_bMVP) {
        COMPV_CHECK_EXP_RETURN(!CompVGLUtils::isGLContextSet(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);
        COMPV_GL_PROGRAM_AUTOBIND(*m_ptrProgram);
        COMPV_glUniformMatrix4fv(m_uNamePrgUnifMVP, 1, GL_FALSE, mvp->matrix()->ptr());
        return COMPV_ERROR_CODE_S_OK;
    }

    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLBlitter::setFBO(CompVGLFboPtr fbo)
{
    m_ptrFBO = fbo;
    return COMPV_ERROR_CODE_S_OK;
}

// Create or update FBO
COMPV_ERROR_CODE CompVGLBlitter::requestFBO(size_t width, size_t height)
{
    if (!m_ptrFBO) {
        CompVGLFboPtr ptrFBO;
        COMPV_CHECK_CODE_RETURN(CompVGLFbo::newObj(&ptrFBO, width, height));
        COMPV_CHECK_CODE_RETURN(setFBO(ptrFBO));
    }
    else {
        COMPV_CHECK_CODE_RETURN(m_ptrFBO->updateSize(width, height));
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLBlitter::updateSize(size_t width, size_t height, size_t stride)
{
    if (m_nWidth != width || m_nHeight != height) {
        CompVGLVertex newVertices[4];
        COMPV_CHECK_CODE_RETURN(CompVGLUtils::updateVertices(width, height, stride, m_bToScreen, &newVertices));

        if (m_ptrFBO) {
            COMPV_CHECK_CODE_RETURN(m_ptrFBO->updateSize(width, height));
        }
        if (m_uNameVertexBuffer) {
            COMPV_glBindBuffer(GL_ARRAY_BUFFER, m_uNameVertexBuffer);
            COMPV_glBufferData(GL_ARRAY_BUFFER, sizeof(newVertices), newVertices, GL_STATIC_DRAW);
        }

        m_nWidth = width;
        m_nHeight = height;
        m_nStride = stride;
        *m_Vertices = *newVertices;
    }

    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLBlitter::close()
{
    if (m_ptrFBO) {
        COMPV_CHECK_CODE_NOP(m_ptrFBO->close());
    }
    COMPV_CHECK_CODE_NOP(deInit());
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLBlitter::init(size_t width, size_t height, size_t stride, const std::string& prgVertexData, const std::string& prgFragData, bool bMVP COMPV_DEFAULT(false), bool bToScreen COMPV_DEFAULT(false))
{
    if (m_bInit) {
        return COMPV_ERROR_CODE_S_OK;
    }
    COMPV_CHECK_EXP_RETURN(!CompVGLUtils::isGLContextSet(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);
    COMPV_CHECK_EXP_RETURN(!width || !height || stride < width || prgVertexData.empty() || prgFragData.empty(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
    m_bInit = true; // Make sure deInit() will be executed if this function fails
    CompVGLVertex newVertices[4];

    // FBO
    if (m_ptrFBO) {
        COMPV_CHECK_CODE_RETURN(m_ptrFBO->updateSize(width, height));
    }

    // Model-View-Projection
    if (!bMVP) {
        m_ptrMVP = NULL;
    }
    else if (!m_ptrMVP) {
        CompVGLMVPPtr glMVP;
        COMPV_CHECK_CODE_RETURN(CompVGLMVP::newObj(&glMVP, COMPV_PROJECTION_2D));
        m_ptrMVP = *glMVP;
    }

    if (CompVGLInfo::extensions::vertex_array_object()) {
        if (!m_uNameVAO) {
            COMPV_CHECK_CODE_BAIL(err = CompVGLUtils::vertexArraysGen(&m_uNameVAO));
        }
        COMPV_glBindVertexArray(m_uNameVAO);
    }

    // Vertex buffer
    if (!m_uNameVertexBuffer) {
        COMPV_CHECK_CODE_BAIL(err = CompVGLUtils::bufferGen(&m_uNameVertexBuffer));
    }
    COMPV_glBindBuffer(GL_ARRAY_BUFFER, m_uNameVertexBuffer);
    COMPV_CHECK_CODE_RETURN(CompVGLUtils::updateVertices(width, height, stride, bToScreen, &newVertices));
    COMPV_glBufferData(GL_ARRAY_BUFFER, sizeof(newVertices), newVertices, GL_STATIC_DRAW);

    // Indice buffer
    if (!m_uNameIndiceBuffer) {
        COMPV_CHECK_CODE_BAIL(err = CompVGLUtils::bufferGen(&m_uNameIndiceBuffer));
        COMPV_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uNameIndiceBuffer);
        COMPV_glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kCompVGLTexture2DIndices), kCompVGLTexture2DIndices, GL_STATIC_DRAW);
    }

    COMPV_CHECK_EXP_BAIL(prgVertexData.empty(), err = COMPV_ERROR_CODE_E_GL);
    COMPV_CHECK_EXP_BAIL(prgFragData.empty(), err = COMPV_ERROR_CODE_E_GL);
    COMPV_CHECK_CODE_BAIL(CompVGLProgram::newObj(&m_ptrProgram, prgVertexData.c_str(), prgVertexData.length(), prgFragData.c_str(), prgFragData.length()));
    COMPV_CHECK_CODE_NOP(err = m_ptrProgram->bind());

	m_uNamePrgAttPosition = COMPV_glGetAttribLocation(m_ptrProgram->name(), "position");
	m_uNamePrgAttTexCoord = COMPV_glGetAttribLocation(m_ptrProgram->name(), "texCoord");

    if (bMVP) {
		m_uNamePrgUnifMVP = COMPV_glGetUniformLocation(m_ptrProgram->name(), "MVP");
		// If "MVP" uniform is unused then, the compiler can remove it: https://stackoverflow.com/questions/23058149/opengl-es-shaders-wrong-uniforms-location
		// Make sure you're using MVP variable.
		COMPV_CHECK_EXP_NOP(m_uNamePrgUnifMVP == GL_INVALID_INDEX, COMPV_ERROR_CODE_E_GL, "Invalid uniform location (make sure 'MVP' variable is used in the code)");
    }
    COMPV_glBindBuffer(GL_ARRAY_BUFFER, m_uNameVertexBuffer);
    COMPV_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uNameIndiceBuffer);
    COMPV_glEnableVertexAttribArray(m_uNamePrgAttPosition);
    COMPV_glEnableVertexAttribArray(m_uNamePrgAttTexCoord);
    COMPV_glVertexAttribPointer(m_uNamePrgAttPosition, 3, GL_FLOAT, GL_FALSE, sizeof(CompVGLVertex), reinterpret_cast<const GLvoid *>(offsetof(CompVGLVertex, Position)));
    COMPV_glVertexAttribPointer(m_uNamePrgAttTexCoord, 2, GL_FLOAT, GL_FALSE, sizeof(CompVGLVertex), reinterpret_cast<const GLvoid *>(offsetof(CompVGLVertex, TexCoord)));
    if (bMVP) {
        COMPV_DEBUG_INFO_CODE_FOR_TESTING();
        // Set aspect ratio
        //float arX = static_cast<float>(width) / static_cast<float>(height);
        //float arY = static_cast<float>(height) / static_cast<float>(width);
        //COMPV_CHECK_CODE_BAIL(err = m_ptrMVP->projection()->setAspectRatio(arX));
        //COMPV_CHECK_CODE_BAIL(err = m_ptrMVP->model()->matrix()->scale(CompVVec3f(1.f/arX, 1.f/arY, 1.f)));
        COMPV_glUniformMatrix4fv(m_uNamePrgUnifMVP, 1, GL_FALSE, m_ptrMVP->matrix()->ptr());
    }

    m_nWidth = width;
    m_nHeight = height;
    m_nStride = stride;
    m_bToScreen = bToScreen;
    *m_Vertices = *newVertices;
    m_bMVP = bMVP;

bail:
    if (m_ptrProgram) {
        COMPV_CHECK_CODE_NOP(m_ptrProgram->unbind());
    }
    if (CompVGLInfo::extensions::vertex_array_object()) {
        COMPV_glBindVertexArray(kCompVGLNameInvalid);
    }
    COMPV_glActiveTexture(GL_TEXTURE0);
    COMPV_glBindTexture(GL_TEXTURE_2D, kCompVGLNameInvalid);
    COMPV_glBindBuffer(GL_ARRAY_BUFFER, kCompVGLNameInvalid);
    COMPV_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, kCompVGLNameInvalid);
    if (COMPV_ERROR_CODE_IS_NOK(err)) {
        COMPV_CHECK_CODE_NOP(deInit());
        m_bInit = false;
    }

    return err;
}

COMPV_ERROR_CODE CompVGLBlitter::deInit()
{
    if (!m_bInit) {
        return COMPV_ERROR_CODE_S_OK;
    }
    COMPV_CHECK_EXP_RETURN(!CompVGLUtils::isGLContextSet(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);

    CompVGLUtils::bufferDelete(&m_uNameVertexBuffer);
    CompVGLUtils::bufferDelete(&m_uNameIndiceBuffer);
    CompVGLUtils::vertexArraysDelete(&m_uNameVAO);
    m_ptrProgram = NULL;
    m_ptrFBO = NULL;

    m_bInit = false;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLBlitter::newObj(CompVGLBlitterPtrPtr blitter)
{
    COMPV_CHECK_EXP_RETURN(!blitter, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVGLBlitterPtr blitter_ = new CompVGLBlitter();
    COMPV_CHECK_EXP_RETURN(!blitter_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    // Do not set FBO, up to the caller
    *blitter = blitter_;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */
