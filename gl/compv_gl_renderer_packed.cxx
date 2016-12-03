/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/gl/compv_gl_renderer_packed.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/gl/compv_gl.h"
#include "compv/gl/compv_gl_utils.h"
#include "compv/gl/compv_gl_func.h"

static const std::string& kProgramVertexData =
    "	attribute vec4 position;"
    "	attribute vec2 texCoord;"
    "	varying vec2 texCoordVarying;"
    "	void main() {"
    "		gl_Position = position;"
    "		texCoordVarying = texCoord;"
    "	}";

static const std::string& kProgramShaderDataRGB24 =
    "	varying vec2 texCoordVarying;"
    "	uniform sampler2D mySampler;"
    "	void main() {"
    "		gl_FragColor = vec4(texture2D(mySampler, texCoordVarying).xyz, 1.0); /* RGB -> RGBA */"
    "	}";

static const std::string& kProgramShaderDataBGR24 =
"	varying vec2 texCoordVarying;"
"	uniform sampler2D mySampler;"
"	void main() {"
"		gl_FragColor = vec4(texture2D(mySampler, texCoordVarying).zyx, 1.0); /* BGR -> RGBA */"
"	}";

static const std::string& kProgramShaderDataBGRA32 =
"	varying vec2 texCoordVarying;"
"	uniform sampler2D mySampler;"
"	void main() {"
"		gl_FragColor = texture2D(mySampler, texCoordVarying).zyxw; /* BGRA -> RGBA */"
"	}";

static const std::string& kProgramShaderDataRGBA32 =
    "	varying vec2 texCoordVarying;"
    "	uniform sampler2D mySampler;"
    "	void main() {"
    "		gl_FragColor = texture2D(mySampler, texCoordVarying).xyzw; /* RGBA -> RGBA */"
    "	}";

static const std::string& kProgramShaderDataARGB32 =
"	varying vec2 texCoordVarying;"
"	uniform sampler2D mySampler;"
"	void main() {"
"		gl_FragColor = texture2D(mySampler, texCoordVarying).yzwx; /* ARGB -> RGBA */"
"	}";

COMPV_NAMESPACE_BEGIN()

CompVGLRendererPacked::CompVGLRendererPacked(COMPV_SUBTYPE ePixelFormat)
    : CompVGLRenderer(ePixelFormat)
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

CompVGLRendererPacked::~CompVGLRendererPacked()
{
    COMPV_CHECK_CODE_NOP(deInit());
}

COMPV_ERROR_CODE CompVGLRendererPacked::drawImage(CompVMatPtr mat) COMPV_OVERRIDE_IMPL("CompVGLRenderer")
{
    COMPV_CHECK_EXP_RETURN(!mat || mat->isEmpty(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    COMPV_CHECK_EXP_RETURN(!CompVGLUtils::isGLContextSet(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);

    COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;

    // Get pixel format and make sure it's supported
    COMPV_SUBTYPE pixelFormat = static_cast<COMPV_SUBTYPE>(mat->subType());
    COMPV_CHECK_EXP_RETURN(CompVRenderer::pixelFormat() != pixelFormat, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

    // Check if format changed
    if (mat->cols() != m_uWidth || mat->rows() != m_uHeight || mat->stride() != m_uStride) {
        COMPV_DEBUG_INFO("GL renderer format changed: %d -> %d", CompVRenderer::pixelFormat(), pixelFormat);
        COMPV_CHECK_CODE_RETURN(deInit());
    }

    // Init if not already done
    COMPV_CHECK_CODE_RETURN(init(mat));

    COMPV_CHECK_CODE_BAIL(err = CompVGLRenderer::bind()); // Bind FBO and VAO

    // Texture 0: RGBA-only format from the blitter's fbo, this is the [destination]
    COMPV_glActiveTexture(GL_TEXTURE0);
    COMPV_glBindTexture(GL_TEXTURE_2D, blitter()->fbo()->nameTexture());

    // Texture 1: RGB-family format from the renderer, this is the [source]
    COMPV_glActiveTexture(GL_TEXTURE1);
    COMPV_glBindTexture(GL_TEXTURE_2D, m_uNameTexture);
    COMPV_glTexSubImage2D(
        GL_TEXTURE_2D,
        0,
        0,
        0,
        static_cast<GLsizei>(mat->stride()),
        static_cast<GLsizei>(mat->rows()),
        m_iFormat,
        GL_UNSIGNED_BYTE,
        mat->ptr());

    COMPV_glViewport(0, 0, static_cast<GLsizei>(blitter()->width()), static_cast<GLsizei>(blitter()->height()));
    COMPV_glDrawElements(GL_TRIANGLES, blitter()->indicesCount(), GL_UNSIGNED_BYTE, 0);

    m_uWidth = mat->cols();
    m_uHeight = mat->rows();
    m_uStride = mat->stride();

bail:
    COMPV_CHECK_CODE_NOP(CompVGLRenderer::unbind());
    return err;
}

COMPV_ERROR_CODE CompVGLRendererPacked::deInit()
{
    if (!m_bInit) {
        return COMPV_ERROR_CODE_S_OK;
    }
    COMPV_CHECK_EXP_RETURN(!CompVGLUtils::isGLContextSet(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);
    COMPV_CHECK_CODE_NOP(CompVGLRenderer::deInit()); // Base class implementation
    COMPV_CHECK_CODE_NOP(CompVGLUtils::textureDelete(&m_uNameTexture));

    m_bInit = false;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLRendererPacked::init(CompVMatPtr mat)
{
    if (m_bInit) {
        return COMPV_ERROR_CODE_S_OK;
    }
    COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
    CompVGLProgramPtr ptrProgram;
    COMPV_CHECK_EXP_RETURN(!CompVGLUtils::isGLContextSet(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);
    m_bInit = true; // To make sure deInit() will be fully executed
    COMPV_CHECK_CODE_BAIL(err = CompVGLRenderer::init(mat, m_strPrgVertexData, m_strPrgFragData, false, false)); // Base class implementation
    COMPV_CHECK_CODE_BAIL(err = CompVGLRenderer::bind()); // Bind to the program -> required by 'glGetUniformLocation'
    COMPV_CHECK_CODE_BAIL(err = CompVGLUtils::textureGen(&m_uNameTexture));
    COMPV_glActiveTexture(GL_TEXTURE1); // "GL_TEXTURE0" is for the RGBA destitation (from the blitter)
    COMPV_glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    COMPV_glBindTexture(GL_TEXTURE_2D, m_uNameTexture);
    COMPV_glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    COMPV_glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    COMPV_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    COMPV_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    COMPV_glTexImage2D(GL_TEXTURE_2D, 0, m_iFormat, static_cast<GLsizei>(mat->stride()), static_cast<GLsizei>(mat->rows()), 0, m_iFormat, GL_UNSIGNED_BYTE, NULL);
    COMPV_glGetUniformLocation(&m_uNameSampler, CompVGLRenderer::blitter()->program()->name(), "mySampler");
    COMPV_glUniform1i(m_uNameSampler, 1);

bail:
    COMPV_CHECK_CODE_NOP(err = CompVGLRenderer::unbind());
    COMPV_glBindTexture(GL_TEXTURE_2D, 0);
    if (COMPV_ERROR_CODE_IS_NOK(err)) {
        COMPV_CHECK_CODE_NOP(deInit());
        m_bInit = false;
    }
    return err;
}

COMPV_ERROR_CODE CompVGLRendererPacked::newObj(CompVGLRendererPackedPtrPtr glRenderer, COMPV_SUBTYPE ePackedPixelFormat)
{
    COMPV_CHECK_CODE_RETURN(CompVGL::init());
    COMPV_CHECK_EXP_RETURN(!glRenderer, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    COMPV_CHECK_EXP_RETURN(
        ePackedPixelFormat != COMPV_SUBTYPE_PIXELS_RGB24
        && ePackedPixelFormat != COMPV_SUBTYPE_PIXELS_BGR24
        && ePackedPixelFormat != COMPV_SUBTYPE_PIXELS_RGBA32
        && ePackedPixelFormat != COMPV_SUBTYPE_PIXELS_BGRA32
        && ePackedPixelFormat != COMPV_SUBTYPE_PIXELS_ABGR32
        && ePackedPixelFormat != COMPV_SUBTYPE_PIXELS_ARGB32
		&& ePackedPixelFormat != COMPV_SUBTYPE_PIXELS_YUYV422,
        COMPV_ERROR_CODE_E_INVALID_PARAMETER);

    CompVGLRendererPackedPtr glRenderer_ = new CompVGLRendererPacked(ePackedPixelFormat);
    COMPV_CHECK_EXP_RETURN(!glRenderer_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	switch (ePackedPixelFormat)
	{
	case COMPV_SUBTYPE_PIXELS_RGBA32:
	case COMPV_SUBTYPE_PIXELS_BGRA32:
	case COMPV_SUBTYPE_PIXELS_ABGR32:
	case COMPV_SUBTYPE_PIXELS_ARGB32:
		glRenderer_->m_iFormat = GL_RGBA; // 32bits
		break;
	case COMPV_SUBTYPE_PIXELS_RGB24:
	case COMPV_SUBTYPE_PIXELS_BGR24:
		glRenderer_->m_iFormat = GL_RGB; // 24bits
		break;
	default:
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT);
		break;
	}

    switch (ePackedPixelFormat) {
    case COMPV_SUBTYPE_PIXELS_RGB24:
        glRenderer_->m_strPrgFragData = kProgramShaderDataRGB24;
        break;
	case COMPV_SUBTYPE_PIXELS_BGR24:
		glRenderer_->m_strPrgFragData = kProgramShaderDataBGR24;
		break;
	case COMPV_SUBTYPE_PIXELS_BGRA32:
		glRenderer_->m_strPrgFragData = kProgramShaderDataBGRA32;
		break;
    case COMPV_SUBTYPE_PIXELS_RGBA32:
        glRenderer_->m_strPrgFragData = kProgramShaderDataRGBA32;
        break;
	case COMPV_SUBTYPE_PIXELS_ARGB32:
		glRenderer_->m_strPrgFragData = kProgramShaderDataARGB32;
		break;
    default:
        COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT);
        break;
    }

	*glRenderer = glRenderer_;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */
