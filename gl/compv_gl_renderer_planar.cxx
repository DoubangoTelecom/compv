/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/gl/compv_gl_renderer_planar.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/gl/compv_gl.h"
#include "compv/gl/compv_gl_utils.h"
#include "compv/gl/compv_gl_func.h"

// GL_LUMINANCE_ALPHA and GL_LUMINANCE are both deprecated in OpenGL 3.0 but available up to OpenGL - ES 3.2
// https://www.opengl.org/wiki/Image_Format#Legacy_Image_Formats
#if defined(HAVE_OPENGLES)
#	define COMPV_GL_FORMAT_Y	GL_LUMINANCE
#	define COMPV_GL_FORMAT_U	GL_LUMINANCE
#	define COMPV_GL_FORMAT_V	GL_LUMINANCE
#	define COMPV_GL_FORMAT_UV	GL_LUMINANCE_ALPHA
#	define COMPV_GL_YUV0		"r" // GL_LUMINANCE.r, GL_LUMINANCE_ALPHA.r
#	define COMPV_GL_YUV1		"a" // GL_LUMINANCE_ALPHA.a
#else
#	define COMPV_GL_FORMAT_Y	GL_RED
#	define COMPV_GL_FORMAT_U	GL_RED
#	define COMPV_GL_FORMAT_V	GL_RED
#	define COMPV_GL_FORMAT_UV	GL_RG
#	define COMPV_GL_YUV0		"r" // GL_RED.r, GL_RG.r
#	define COMPV_GL_YUV1		"g" // GL_RG.g
#endif

/*
*** YUV to RGB conversion: http://www.fourcc.org/fccyvrgb.php ***
R = 1.164(Y - 16) + 0(U - 128)			+ 1.596(V - 128)
G = 1.164(Y - 16) - 0.391(U - 128)	- 0.813(V - 128)
B = 1.164(Y - 16) + 2.018(U - 128)	+ 0(V - 128)
Y, U, Y, R, G, B are within [0-255] but shaders require [0.0 - 1.0] for color intensity
(16.f / 255.f) -> 0.06274
(128.f / 255.f) -> 0.5019
*/

static const std::string& kProgramVertexData =
"	attribute vec4 position;"
"	attribute vec2 texCoord;"
"	varying vec2 texCoordVarying;"
"	void main() {"
"		gl_Position = position;"
"		texCoordVarying = texCoord;"
"	}";

static const std::string& kProgramFragmentDataYUV420 =
#if defined(HAVE_OPENGLES)
"	precision mediump float;"
#endif
"	varying vec2 texCoordVarying;"
"	"
"	uniform sampler2D SamplerY;"
"	uniform sampler2D SamplerU;"
"	uniform sampler2D SamplerV;"
"	"
"	const mat3 yuv2rgb = mat3(	1.164,	0,		1.596,	"
"								1.164,	-0.391,	-0.813,	"
"								1.164,	2.018,	0		"
"							);"
"	"
"	void main() {"
"		vec3 yuv = vec3(texture2D(SamplerY, texCoordVarying)." COMPV_GL_YUV0 " - 0.06274, "
"                    texture2D(SamplerU, texCoordVarying)." COMPV_GL_YUV0 " - 0.5019, "
"                    texture2D(SamplerV, texCoordVarying)." COMPV_GL_YUV0 " - 0.5019); "
"		vec3 rgb = yuv * yuv2rgb;"
"		gl_FragColor = vec4(rgb, 1.0);"
"	}";

static const std::string& kProgramFragmentDataNV12 =
#if defined(HAVE_OPENGLES)
"	precision mediump float;"
#endif
"	varying vec2 texCoordVarying;"
"	"
"	uniform sampler2D SamplerY;"
"	uniform sampler2D SamplerUV;"
"	"
"	const mat3 yuv2rgb = mat3(	1.164,	0,		1.596,	"
"								1.164,	-0.391,	-0.813,	"
"								1.164,	2.018,	0		"
"							);"
"	"
"	void main() {"
"		vec3 yuv = vec3(texture2D(SamplerY, texCoordVarying)." COMPV_GL_YUV0 " - 0.06274, "
"                    texture2D(SamplerUV, texCoordVarying)." COMPV_GL_YUV0 " - 0.5019, "
"                    texture2D(SamplerUV, texCoordVarying)." COMPV_GL_YUV1 " - 0.5019); "
"		vec3 rgb = yuv * yuv2rgb;"
"		gl_FragColor = vec4(rgb, 1.0);"
"	}";

static const std::string& kProgramFragmentDataNV21 =
#if defined(HAVE_OPENGLES)
"	precision mediump float;"
#endif
"	varying vec2 texCoordVarying;"
"	"
"	uniform sampler2D SamplerY;"
"	uniform sampler2D SamplerUV;"
"	"
"	const mat3 yuv2rgb = mat3(	1.164,	0,		1.596,	"
"								1.164,	-0.391,	-0.813,	"
"								1.164,	2.018,	0		"
"							);"
"	"
"	void main() {"
"		vec3 yuv = vec3(texture2D(SamplerY, texCoordVarying)." COMPV_GL_YUV0 " - 0.06274, "
"                    texture2D(SamplerUV, texCoordVarying)." COMPV_GL_YUV1 " - 0.5019, "
"                    texture2D(SamplerUV, texCoordVarying)." COMPV_GL_YUV0 " - 0.5019); "
"		vec3 rgb = yuv * yuv2rgb;"
"		gl_FragColor = vec4(rgb, 1.0);"
"	}";

static const std::string& kProgramFragmentDataY =
#if defined(HAVE_OPENGLES)
"	precision mediump float;"
#endif
"	varying vec2 texCoordVarying;"
"	"
"	uniform sampler2D SamplerY;"
"	"
"	void main() {"
"		gl_FragColor = vec4(texture2D(SamplerY, texCoordVarying)." COMPV_GL_YUV0 COMPV_GL_YUV0 COMPV_GL_YUV0 ", 1.0); /* Grayscale -> [R = G = B = Gray] */"
"	}";

COMPV_NAMESPACE_BEGIN()

CompVGLRendererPlanar::CompVGLRendererPlanar(COMPV_SUBTYPE ePixelFormat)
    : CompVGLRenderer(ePixelFormat)
    , m_bInit(false)
    , m_uTexturesCount(0)
    , m_strPrgVertexData("")
    , m_strPrgFragData("")
{
    memset(&m_uNameTextures, kCompVGLNameInvalid, sizeof(m_uNameTextures));
    memset(&m_uWidths, 0, sizeof(m_uWidths));
    memset(&m_uHeights, 0, sizeof(m_uHeights));
    memset(&m_uStrides, 0, sizeof(m_uStrides));
}

CompVGLRendererPlanar::~CompVGLRendererPlanar()
{
    if (m_bInit) {
        COMPV_CHECK_CODE_NOP(deInit());
    }
}

COMPV_ERROR_CODE CompVGLRendererPlanar::drawImage(CompVMatPtr mat) COMPV_OVERRIDE_IMPL("CompVGLRenderer")
{
    COMPV_CHECK_EXP_RETURN(!mat || mat->isEmpty(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    COMPV_CHECK_EXP_RETURN(!CompVGLUtils::isGLContextSet(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);

    // Get pixel format and make sure it's supported
    COMPV_SUBTYPE pixelFormat = static_cast<COMPV_SUBTYPE>(mat->subType());
    COMPV_CHECK_EXP_RETURN(CompVRenderer::pixelFormat() != pixelFormat, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

    // Check if format changed
	bool bFormatChanged = m_uTexturesCount != mat->compCount();
	if (!bFormatChanged) {
		for (int32_t i = 0; i < static_cast<int32_t>(mat->compCount()); ++i) {
			if (mat->cols(i) != m_uWidths[i] || mat->rows(i) != m_uHeights[i] || mat->stride(i) != m_uStrides[i]) {
				bFormatChanged = true;
				break;
			}
		}
	}
    if (bFormatChanged) {
        COMPV_DEBUG_INFO("GL renderer format changed");
        COMPV_CHECK_CODE_RETURN(deInit());
    }

    // Init if not already done
    COMPV_CHECK_CODE_RETURN(init(mat));

	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;

	COMPV_CHECK_CODE_BAIL(err = CompVGLRenderer::bind()); // Bind FBO and VAO

	// Texture 0: RGBA format from the blitter's fbo, this is the [destination]
	COMPV_glActiveTexture(GL_TEXTURE0);
	COMPV_glBindTexture(GL_TEXTURE_2D, blitter()->fbo()->nameTexture());

	// Texture 1+: YUV data format from the renderer, this is the [source]
    for (size_t t = 0; t < m_uTexturesCount; ++t) {
        COMPV_glActiveTexture(GLenum(GL_TEXTURE1 + t));
        COMPV_glBindTexture(GL_TEXTURE_2D, m_uNameTextures[t]);
        COMPV_glTexSubImage2D(
            GL_TEXTURE_2D,
            0,
            0,
            0,
            static_cast<GLsizei>(m_uStrides[t]),
            static_cast<GLsizei>(m_uHeights[t]),
			m_eFormats[t],
			GL_UNSIGNED_BYTE,
            mat->ptr(0, 0, static_cast<int32_t>(t)));
    }

	COMPV_glViewport(0, 0, static_cast<GLsizei>(blitter()->width()), static_cast<GLsizei>(blitter()->height()));
	COMPV_glDrawElements(GL_TRIANGLES, blitter()->indicesCount(), GL_UNSIGNED_BYTE, 0);

bail:
	COMPV_CHECK_CODE_NOP(CompVGLRenderer::unbind());
    return err;
}

// Private function: do not check imput parameters
COMPV_ERROR_CODE CompVGLRendererPlanar::init(CompVMatPtr mat)
{
	if (m_bInit) {
		return COMPV_ERROR_CODE_S_OK;
	}
	GLuint nameSampler;
	const char* idSampler = NULL;
    COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
    COMPV_CHECK_EXP_RETURN(!CompVGLUtils::isGLContextSet(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);
    COMPV_CHECK_CODE_BAIL(err = CompVGLRenderer::init(mat, m_strPrgVertexData, m_strPrgFragData, false, false)); // Base class implementation
	COMPV_CHECK_CODE_BAIL(err = CompVGLRenderer::bind()); // Bind to the program -> required by 'glGetUniformLocation'
	m_bInit = true;
    m_uTexturesCount = mat->compCount();
    for (size_t compId = 0; compId < mat->compCount(); ++compId) {
        const int32_t compIdInt32 = static_cast<int32_t>(compId);
        m_uHeights[compId] = mat->rows(compIdInt32);
        m_uWidths[compId] = mat->cols(compIdInt32);
        m_uStrides[compId] = mat->stride(compIdInt32);
		COMPV_CHECK_CODE_BAIL(err = CompVGLUtils::textureGen(&m_uNameTextures[compId]));
        COMPV_glActiveTexture(GLenum(GL_TEXTURE1 + compId)); // "GL_TEXTURE0" is for the RGBA destitation (from the blitter)
        COMPV_glBindTexture(GL_TEXTURE_2D, m_uNameTextures[compId]);
        COMPV_glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        COMPV_glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        COMPV_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        COMPV_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        COMPV_glTexImage2D(GL_TEXTURE_2D, 0, m_eFormats[compId], static_cast<GLsizei>(m_uStrides[compId]), static_cast<GLsizei>(m_uHeights[compId]), 0, m_eFormats[compId], GL_UNSIGNED_BYTE, NULL);
		COMPV_glGetUniformLocation(&nameSampler, blitter()->program()->name(), m_pSamplerNames[compId]);
		COMPV_glUniform1i(nameSampler, static_cast<GLint>(1 + compId));
    }

bail:
    if (COMPV_ERROR_CODE_IS_NOK(err)) {
        COMPV_CHECK_CODE_NOP(deInit());
    }
    return err;
}

COMPV_ERROR_CODE CompVGLRendererPlanar::deInit()
{
	if (!m_bInit) {
		return COMPV_ERROR_CODE_S_OK;
	}
    COMPV_CHECK_EXP_RETURN(!CompVGLUtils::isGLContextSet(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);
    COMPV_CHECK_CODE_RETURN(CompVGLRenderer::deInit()); // Base class implementation
    for (size_t t = 0; t < m_uTexturesCount; ++t) {
		COMPV_CHECK_CODE_NOP(CompVGLUtils::textureDelete(&m_uNameTextures[t]));
        m_uWidths[t] = 0;
        m_uHeights[t] = 0;
        m_uStrides[t] = 0;
    }
    m_uTexturesCount = 0;

    m_bInit = false;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLRendererPlanar::newObj(CompVGLRendererPlanarPtrPtr glRenderer, COMPV_SUBTYPE ePixelFormat)
{
    COMPV_CHECK_CODE_RETURN(CompVGL::init());
    COMPV_CHECK_EXP_RETURN(!glRenderer, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    COMPV_CHECK_EXP_RETURN(
        ePixelFormat != COMPV_SUBTYPE_PIXELS_Y
        && ePixelFormat != COMPV_SUBTYPE_PIXELS_YUV420P
		&& ePixelFormat != COMPV_SUBTYPE_PIXELS_NV12
		&& ePixelFormat != COMPV_SUBTYPE_PIXELS_NV21
		&& ePixelFormat != COMPV_SUBTYPE_PIXELS_RGBA32,
        COMPV_ERROR_CODE_E_INVALID_PARAMETER);

    CompVGLRendererPlanarPtr glRenderer_ = new CompVGLRendererPlanar(ePixelFormat);
    COMPV_CHECK_EXP_RETURN(!glRenderer_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

	// Vertex and fragment programs source
	glRenderer_->m_strPrgVertexData = kProgramVertexData;
	switch (ePixelFormat) {
	case COMPV_SUBTYPE_PIXELS_YUV420P:
		glRenderer_->m_strPrgFragData = kProgramFragmentDataYUV420;
		break;
	case COMPV_SUBTYPE_PIXELS_Y:
		glRenderer_->m_strPrgFragData = kProgramFragmentDataY;
		break;
	case COMPV_SUBTYPE_PIXELS_NV12:
		glRenderer_->m_strPrgFragData = kProgramFragmentDataNV12;
		break;
	case COMPV_SUBTYPE_PIXELS_NV21:
		glRenderer_->m_strPrgFragData = kProgramFragmentDataNV21;
		break;
	default:
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT);
		break;
	}

	// Samplers and and formats
	switch (ePixelFormat) {
	case COMPV_SUBTYPE_PIXELS_YUV420P:
	case COMPV_SUBTYPE_PIXELS_Y:
		glRenderer_->m_pSamplerNames[0] = "SamplerY", glRenderer_->m_eFormats[0] = COMPV_GL_FORMAT_Y;
		glRenderer_->m_pSamplerNames[1] = "SamplerU", glRenderer_->m_eFormats[1] = COMPV_GL_FORMAT_U;
		glRenderer_->m_pSamplerNames[2] = "SamplerV", glRenderer_->m_eFormats[2] = COMPV_GL_FORMAT_V;
		break;
	case COMPV_SUBTYPE_PIXELS_NV12:
	case COMPV_SUBTYPE_PIXELS_NV21:
		glRenderer_->m_pSamplerNames[0] = "SamplerY", glRenderer_->m_eFormats[0] = COMPV_GL_FORMAT_Y;
		glRenderer_->m_pSamplerNames[1] = "SamplerUV", glRenderer_->m_eFormats[1] = COMPV_GL_FORMAT_UV;
		break;
	default:
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT);
		break;
	}

    COMPV_CHECK_EXP_RETURN(!(*glRenderer = glRenderer_), COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */
