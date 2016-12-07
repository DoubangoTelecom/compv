/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/gl/compv_gl_renderer.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/base/compv_base.h"
#include "compv/gl/compv_gl.h"
#include "compv/gl/compv_gl_utils.h"
#include "compv/gl/compv_gl_func.h"
#include "compv/gl/compv_gl_info.h"

// GL_LUMINANCE_ALPHA and GL_LUMINANCE are both deprecated in OpenGL 3.0 but available up to OpenGL - ES 3.2
// https://www.opengl.org/wiki/Image_Format#Legacy_Image_Formats
#if defined(HAVE_OPENGLES)
#	define COMPV_GL_FORMAT_Y	GL_LUMINANCE
#	define COMPV_GL_FORMAT_U	GL_LUMINANCE
#	define COMPV_GL_FORMAT_V	GL_LUMINANCE
#	define COMPV_GL_FORMAT_UV	GL_LUMINANCE_ALPHA
#	define COMPV_GL_BYTE0		"r" // GL_LUMINANCE.r, GL_LUMINANCE_ALPHA.r
#	define COMPV_GL_BYTE1		"a" // GL_LUMINANCE_ALPHA.a
#else
#	define COMPV_GL_FORMAT_Y	GL_RED
#	define COMPV_GL_FORMAT_U	GL_RED
#	define COMPV_GL_FORMAT_V	GL_RED
#	define COMPV_GL_FORMAT_UV	GL_RG
#	define COMPV_GL_BYTE0		"r" // GL_RED.r, GL_RG.r
#	define COMPV_GL_BYTE1		"g" // GL_RG.g
#endif
#	define COMPV_GL_FORMAT_RGB		GL_RGB
#	define COMPV_GL_FORMAT_RGBA		GL_RGBA
#	define COMPV_GL_FORMAT_R		COMPV_GL_FORMAT_U
#	define COMPV_GL_FORMAT_RG		COMPV_GL_FORMAT_UV

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

static const std::string& kProgramFragmentDataYUVPlanar = /* Used for YUV420P, YVU420P YUV422P */
#if defined(HAVE_OPENGLES)
"	precision mediump float;"
#endif
"	varying vec2 texCoordVarying;"
"	"
"	uniform sampler2D SamplerY;"
"	uniform sampler2D SamplerU;"
"	uniform sampler2D SamplerV;"
"	"
"	const mat3 yuv2rgb = mat3(1.164, 0, 1.596,\n 1.164, -0.391, -0.813,\n 1.164, 2.018, 0);"
"	"
"	void main() {"
"		vec3 yuv = vec3(texture2D(SamplerY, texCoordVarying)." COMPV_GL_BYTE0 " - 0.06274, "
"                    texture2D(SamplerU, texCoordVarying)." COMPV_GL_BYTE0 " - 0.5019, "
"                    texture2D(SamplerV, texCoordVarying)." COMPV_GL_BYTE0 " - 0.5019); "
"		vec3 rgb = yuv * yuv2rgb;"
"		gl_FragColor = vec4(rgb, 1.0);"
"	}";

static const std::string& kProgramFragmentDataYUV420P = kProgramFragmentDataYUVPlanar; /* YUV420 aka I420 aka IYUV */

static const std::string& kProgramFragmentDataYVU420P = kProgramFragmentDataYUVPlanar; /* YVU420 aka YV12 */

static const std::string& kProgramFragmentDataNV12 =
#if defined(HAVE_OPENGLES)
"	precision mediump float;"
#endif
"	varying vec2 texCoordVarying;"
"	"
"	uniform sampler2D SamplerY;"
"	uniform sampler2D SamplerUV;"
"	"
"	const mat3 yuv2rgb = mat3(1.164, 0, 1.596,\n 1.164, -0.391, -0.813,\n 1.164, 2.018, 0);"
"	"
"	void main() {"
"		vec3 yuv = vec3(texture2D(SamplerY, texCoordVarying)." COMPV_GL_BYTE0 " - 0.06274, "
"                    texture2D(SamplerUV, texCoordVarying)." COMPV_GL_BYTE0 " - 0.5019, "
"                    texture2D(SamplerUV, texCoordVarying)." COMPV_GL_BYTE1 " - 0.5019); "
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
"	const mat3 yuv2rgb = mat3(1.164, 0, 1.596,\n 1.164, -0.391, -0.813,\n 1.164, 2.018, 0);"
"	"
"	void main() {"
"		vec3 yuv = vec3(texture2D(SamplerY, texCoordVarying)." COMPV_GL_BYTE0 " - 0.06274, "
"                    texture2D(SamplerUV, texCoordVarying)." COMPV_GL_BYTE1 " - 0.5019, "
"                    texture2D(SamplerUV, texCoordVarying)." COMPV_GL_BYTE0 " - 0.5019); "
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
"		gl_FragColor = vec4(texture2D(SamplerY, texCoordVarying)." COMPV_GL_BYTE0 COMPV_GL_BYTE0 COMPV_GL_BYTE0 ", 1.0); /* Grayscale -> [R = G = B = Gray] */"
"	}";

static const std::string& kProgramFragmentDataYUYV422 = /* YUY2: https://www.fourcc.org/pixel-format/yuv-yuy2/ */
#if defined(HAVE_OPENGLES)
"	precision mediump float;"
#endif
"	varying vec2 texCoordVarying;"
"	uniform sampler2D SamplerRGBA;"
"	uniform vec2 TextureSize;"
"	const mat3 yuv2rgb = mat3(1.164, 0, 1.596,\n 1.164, -0.391, -0.813,\n 1.164, 2.018, 0);"
"	void main() {"
"		float texCoordX = texCoordVarying.x * TextureSize.x; /* real texture coord (in pixels) */"
"		vec3 yuv;"
"		vec4 y0u0y1v0 = texture2D(SamplerRGBA, texCoordVarying);"
"		"
"		if (fract(texCoordX) < 0.5) {"
"			yuv = vec3(y0u0y1v0.r - 0.06274, y0u0y1v0.g - 0.5019, y0u0y1v0.a - 0.5019); /* y0u0v0 */"
"		}"
"		else {"
"			yuv = vec3(y0u0y1v0.b - 0.06274,  y0u0y1v0.g - 0.5019, y0u0y1v0.a - 0.5019); /* y1u0v0 */ "
"		}"
"		vec3 rgb = yuv * yuv2rgb; "
"		gl_FragColor = vec4(rgb, 1.0);"
"	}";

static const std::string& kProgramFragmentDataUYVY422 = /* UYVY: https://www.fourcc.org/pixel-format/yuv-uyvy/ */
#if defined(HAVE_OPENGLES)
"	precision mediump float;"
#endif
"	varying vec2 texCoordVarying;"
"	uniform sampler2D SamplerRGBA;"
"	uniform vec2 TextureSize;"
"	const mat3 yuv2rgb = mat3(1.164, 0, 1.596,\n 1.164, -0.391, -0.813,\n 1.164, 2.018, 0);"
"	void main() {"
"		float texCoordX = texCoordVarying.x * TextureSize.x; /* real texture coord (in pixels) */"
"		vec3 yuv;"
"		vec4 u0y0v0y1 = texture2D(SamplerRGBA, texCoordVarying);"
"		"
"		if (fract(texCoordX) < 0.5) {"
"			yuv = vec3(u0y0v0y1.g - 0.06274, u0y0v0y1.r - 0.5019, u0y0v0y1.b - 0.5019); /* y0u0v0 */"
"		}"
"		else {"
"			yuv = vec3(u0y0v0y1.a - 0.06274,  u0y0v0y1.r - 0.5019, u0y0v0y1.b - 0.5019); /* y1u0v0 */ "
"		}"
"		vec3 rgb = yuv * yuv2rgb; "
"		gl_FragColor = vec4(rgb, 1.0);"
"	}";

static const std::string& kProgramFragmentDataYUV422P = kProgramFragmentDataYUVPlanar;

static const std::string& kProgramFragmentDataYUV444P = kProgramFragmentDataYUVPlanar;

static const std::string& kProgramFragmentDataRGB24 =
"	varying vec2 texCoordVarying;"
"	uniform sampler2D SamplerRGB;"
"	void main() {"
"		gl_FragColor = vec4(texture2D(SamplerRGB, texCoordVarying).xyz, 1.0); /* RGB -> RGBA */"
"	}";

static const std::string& kProgramFragmentDataBGR24 =
"	varying vec2 texCoordVarying;"
"	uniform sampler2D SamplerRGB;"
"	void main() {"
"		gl_FragColor = vec4(texture2D(SamplerRGB, texCoordVarying).zyx, 1.0); /* BGR -> RGBA */"
"	}";

static const std::string& kProgramFragmentDataBGRA32 =
"	varying vec2 texCoordVarying;"
"	uniform sampler2D SamplerRGBA;"
"	void main() {"
"		gl_FragColor = texture2D(SamplerRGBA, texCoordVarying).zyxw; /* BGRA -> RGBA */"
"	}";

static const std::string& kProgramFragmentDataABGR32 =
"	varying vec2 texCoordVarying;"
"	uniform sampler2D SamplerRGBA;"
"	void main() {"
"		gl_FragColor = texture2D(SamplerRGBA, texCoordVarying).wzyx; /* ABGR -> RGBA */"
"	}";

static const std::string& kProgramFragmentDataRGBA32 =
"	varying vec2 texCoordVarying;"
"	uniform sampler2D SamplerRGBA;"
"	void main() {"
"		gl_FragColor = texture2D(SamplerRGBA, texCoordVarying).xyzw; /* RGBA -> RGBA */"
"	}";

static const std::string& kProgramFragmentDataARGB32 =
"	varying vec2 texCoordVarying;"
"	uniform sampler2D SamplerRGBA;"
"	void main() {"
"		gl_FragColor = texture2D(SamplerRGBA, texCoordVarying).yzwx; /* ARGB -> RGBA */"
"	}";

static const std::string& kProgramFragmentDataRGB565Native =
"	varying vec2 texCoordVarying;"
"	uniform sampler2D SamplerRG565;"
"	void main() {"
"		gl_FragColor = vec4(texture2D(SamplerRG565, texCoordVarying).xyz, 1.0); /* RGB565LE -> RGBA */"
"	}";

static const std::string& kProgramFragmentDataBGR565Native =
"	varying vec2 texCoordVarying;"
"	uniform sampler2D SamplerRG565;"
"	void main() {"
"		gl_FragColor = vec4(texture2D(SamplerRG565, texCoordVarying).zyx, 1.0); /* BGR565LE -> RGBA */"
"	}";

static const std::string& kProgramFragmentDataRGB565LE = /* Not optimized, called only on endianness mismatch. Requires OpenGL ES 3.0 */
"	varying vec2 texCoordVarying;"
"	uniform sampler2D SamplerRG565;"
"	void main() {"
"		vec4 rgb565le = texture2D(SamplerRG565, texCoordVarying);"
"		uint pixel = (uint(rgb565le." COMPV_GL_BYTE1 " * 255.0) << 8) | uint(rgb565le." COMPV_GL_BYTE0 " * 255.0);"
"		gl_FragColor = vec4(float((pixel >> 11) & 0x1f) / 31.0, float((pixel >> 5) & 0x3f) / 63.0, float(pixel & 0x1f) / 31.0, 1.0); /* RGB565LE -> RGBA */"
"	}";

static const std::string& kProgramFragmentDataRGB565BE = /* Not optimized, called only on endianness mismatch. Requires OpenGL ES 3.0 */
"	varying vec2 texCoordVarying;"
"	uniform sampler2D SamplerRG565;"
"	void main() {"
"		vec4 rgb565be = texture2D(SamplerRG565, texCoordVarying);"
"		uint pixel = (uint(rgb565be." COMPV_GL_BYTE0 " * 255.0) << 8) | uint(rgb565be." COMPV_GL_BYTE1 " * 255.0);"
"		gl_FragColor = vec4(float((pixel >> 11) & 0x1f) / 31.0, float((pixel >> 5) & 0x3f) / 63.0, float(pixel & 0x1f) / 31.0, 1.0); /* RGB565BE -> RGBA */"
"	}";

static const std::string& kProgramFragmentDataBGR565LE = /* Not optimized, called only on endianness mismatch. Requires OpenGL ES 3.0 */
"	varying vec2 texCoordVarying;"
"	uniform sampler2D SamplerRG565;"
"	void main() {"
"		vec4 rgb565le = texture2D(SamplerRG565, texCoordVarying);"
"		uint pixel = (uint(rgb565le." COMPV_GL_BYTE1 " * 255.0) << 8) | uint(rgb565le." COMPV_GL_BYTE0 " * 255.0);"
"		gl_FragColor = vec4(float(pixel & 0x1f) / 31.0, float((pixel >> 5) & 0x3f) / 63.0, float((pixel >> 11) & 0x1f) / 31.0, 1.0); /* BGR565LE -> RGBA */"
"	}";

static const std::string& kProgramFragmentDataBGR565BE = /* Not optimized, called only on endianness mismatch. Requires OpenGL ES 3.0 */
"	varying vec2 texCoordVarying;"
"	uniform sampler2D SamplerRG565;"
"	void main() {"
"		vec4 rgb565be = texture2D(SamplerRG565, texCoordVarying);"
"		uint pixel = (uint(rgb565be." COMPV_GL_BYTE0 " * 255.0) << 8) | uint(rgb565be." COMPV_GL_BYTE1 " * 255.0);"
"		gl_FragColor = vec4(float(pixel & 0x1f) / 31.0, float((pixel >> 5) & 0x3f) / 63.0, float((pixel >> 11) & 0x1f) / 31.0, 1.0); /* RGB565BE -> RGBA */"
"	}";

COMPV_NAMESPACE_BEGIN()

CompVGLRenderer::CompVGLRenderer(COMPV_SUBTYPE ePixelFormat)
    : CompVRenderer(ePixelFormat)
    , m_bInit(false)
    , m_uTexturesCount(0)
    , m_strPrgVertexData("")
    , m_strPrgFragData("")
{
    memset(&m_uNameTextures, kCompVGLNameInvalid, sizeof(m_uNameTextures));
    memset(&m_uWidths, 0, sizeof(m_uWidths));
    memset(&m_uHeights, 0, sizeof(m_uHeights));
    memset(&m_uStrides, 0, sizeof(m_uStrides));
	memset(&m_uWidthsTexture, 0, sizeof(m_uWidths));
	memset(&m_uHeightsTexture, 0, sizeof(m_uHeights));
	memset(&m_uStridesTexture, 0, sizeof(m_uStrides));
}

CompVGLRenderer::~CompVGLRenderer()
{
    if (m_bInit) {
        COMPV_CHECK_CODE_NOP(deInit());
    }
}

CompVCanvasPtr CompVGLRenderer::canvas() /*Overrides(CompVGLRenderer)*/
{
	if (!m_ptrCanvas) {
		CompVCanvasImplPtr canvasImpl;
		COMPV_CHECK_EXP_BAIL(!CompVGLUtils::isGLContextSet(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);
		COMPV_CHECK_EXP_BAIL(!m_ptrBlitter->isInitialized(), COMPV_ERROR_CODE_E_INVALID_STATE);
		COMPV_CHECK_CODE_BAIL(CompVCanvasFactory::newObj(&canvasImpl));
		COMPV_CHECK_CODE_BAIL(CompVGLCanvas::newObj(&m_ptrCanvas, m_ptrBlitter->fbo(), canvasImpl));
	}
bail:
	return *m_ptrCanvas;
}

COMPV_ERROR_CODE CompVGLRenderer::drawImage(const CompVMatPtr mat) /*Overrides(CompVGLRenderer)*/
{
    COMPV_CHECK_EXP_RETURN(!mat || mat->isEmpty(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    COMPV_CHECK_EXP_RETURN(!CompVGLUtils::isGLContextSet(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);

    // Get pixel format and make sure it's supported
    COMPV_CHECK_EXP_RETURN(CompVRenderer::pixelFormat() != mat->subType(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);

    // Check if format changed
	bool bFormatChanged = m_uTexturesCount != mat->planeCount();
	if (!bFormatChanged) {
		for (int32_t i = 0; i < static_cast<int32_t>(mat->planeCount()); ++i) {
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

	COMPV_CHECK_CODE_BAIL(err = m_ptrBlitter->bind()); // Bind FBO, VAO and activate the program

	// Texture 0: RGBA format from the blitter's fbo, this is the [destination]
	COMPV_glActiveTexture(GL_TEXTURE0);
	COMPV_glBindTexture(GL_TEXTURE_2D, m_ptrBlitter->fbo()->nameTexture());

	// Texture 1+: YUV data format from the renderer, this is the [source]
    for (size_t t = 0; t < m_uTexturesCount; ++t) {
        COMPV_glActiveTexture(GLenum(GL_TEXTURE1 + t));
        COMPV_glBindTexture(GL_TEXTURE_2D, m_uNameTextures[t]);
        COMPV_glTexSubImage2D(
            GL_TEXTURE_2D,
            0,
            0,
            0,
            static_cast<GLsizei>(m_uStridesTexture[t]),
            static_cast<GLsizei>(m_uHeightsTexture[t]),
			m_eFormats[t],
			m_ePixelDataType,
            mat->ptr(0, 0, static_cast<int32_t>(t)));
    }

	COMPV_glViewport(0, 0, static_cast<GLsizei>(m_ptrBlitter->width()), static_cast<GLsizei>(m_ptrBlitter->height()));
	COMPV_glDrawElements(GL_TRIANGLES, m_ptrBlitter->indicesCount(), GL_UNSIGNED_BYTE, 0);

bail:
	COMPV_glActiveTexture(GL_TEXTURE0);
	COMPV_glBindTexture(GL_TEXTURE_2D, kCompVGLNameInvalid);
	for (size_t t = 0; t < m_uTexturesCount; ++t) {
		COMPV_glActiveTexture(GLenum(GL_TEXTURE1 + t));
		COMPV_glBindTexture(GL_TEXTURE_2D, kCompVGLNameInvalid);
	}
	COMPV_CHECK_CODE_NOP(m_ptrBlitter->unbind());
    return err;
}

COMPV_ERROR_CODE CompVGLRenderer::close()
{
	if (m_ptrBlitter) {
		COMPV_CHECK_CODE_NOP(m_ptrBlitter->close());
	}
	if (m_ptrCanvas) {
		COMPV_CHECK_CODE_NOP(m_ptrCanvas->close());
	}
	COMPV_CHECK_CODE_NOP(deInit());
	return COMPV_ERROR_CODE_S_OK;
}

// Private function: do not check imput parameters
COMPV_ERROR_CODE CompVGLRenderer::init(const CompVMatPtr mat)
{
	if (m_bInit) {
		return COMPV_ERROR_CODE_S_OK;
	}
	GLuint uNameLocation;
	const COMPV_SUBTYPE pixelFormat = mat->subType();
	GLint textureFilter;
    COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
    COMPV_CHECK_EXP_RETURN(!CompVGLUtils::isGLContextSet(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);
	// Set "m_bInit" to true to make sure deInit() will be fully executed
	m_bInit = true;
	// Create/Update FBO for the blitter
	COMPV_CHECK_CODE_BAIL(err = m_ptrBlitter->requestFBO(mat->cols(), mat->rows()));
	// Init blitter
	COMPV_CHECK_CODE_BAIL(err = m_ptrBlitter->init(mat->cols(), mat->rows(), mat->stride(), m_strPrgVertexData, m_strPrgFragData, false/*NoMVP*/, false/*NotToScreen*/));
	// Bind to the program -> required by 'glGetUniformLocation'
	COMPV_CHECK_CODE_BAIL(err = m_ptrBlitter->bind());
    m_uTexturesCount = mat->planeCount();
    for (size_t planeId = 0; planeId < mat->planeCount(); ++planeId) {
        const int32_t planeIdInt32 = static_cast<int32_t>(planeId);
        m_uHeights[planeId] = mat->rows(planeIdInt32);
        m_uWidths[planeId] = mat->cols(planeIdInt32);
        m_uStrides[planeId] = mat->stride(planeIdInt32);
		switch (pixelFormat) {
		case COMPV_SUBTYPE_PIXELS_YUYV422: // YUY2: https://www.fourcc.org/pixel-format/yuv-yuy2/
		case COMPV_SUBTYPE_PIXELS_UYVY422: // UYVY: https://www.fourcc.org/pixel-format/yuv-uyvy/
			// Each [YUYV] or [UYVY] uint32 value is packed as a single RGBA sample which means 2Y for each. The width will be /2.
			m_uWidthsTexture[planeId] = static_cast<GLsizei>((m_uWidths[planeId] + 1) >> 1), m_uHeightsTexture[planeId] = static_cast<GLsizei>(m_uHeights[planeId]), m_uStridesTexture[planeId] = static_cast<GLsizei>((m_uStrides[planeId] + 1) >> 1);
			textureFilter = GL_NEAREST; // *must*
			break;
		default:
			m_uWidthsTexture[planeId] = static_cast<GLsizei>(m_uWidths[planeId]), m_uHeightsTexture[planeId] = static_cast<GLsizei>(m_uHeights[planeId]), m_uStridesTexture[planeId] = static_cast<GLsizei>(m_uStrides[planeId]);
			textureFilter = GL_LINEAR; // any
			break;
		}
		COMPV_CHECK_CODE_BAIL(err = CompVGLUtils::textureGen(&m_uNameTextures[planeId]));
        COMPV_glActiveTexture(GLenum(GL_TEXTURE1 + planeId)); // "GL_TEXTURE0" is for the RGBA destitation (from the blitter)
        COMPV_glBindTexture(GL_TEXTURE_2D, m_uNameTextures[planeId]);
        COMPV_glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        COMPV_glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        COMPV_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, textureFilter);
        COMPV_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, textureFilter);
        COMPV_glTexImage2D(GL_TEXTURE_2D, 0, m_eFormats[planeId], static_cast<GLsizei>(m_uStridesTexture[planeId]), static_cast<GLsizei>(m_uHeightsTexture[planeId]), 0, m_eFormats[planeId], m_ePixelDataType, NULL);
		COMPV_glGetUniformLocation(&uNameLocation, m_ptrBlitter->program()->name(), m_pSamplerNames[planeId]);
		COMPV_glUniform1i(uNameLocation, static_cast<GLint>(1 + planeId));
		// Set TextureSize = (stride, height)
		switch (pixelFormat) {
		case COMPV_SUBTYPE_PIXELS_YUYV422:
		case COMPV_SUBTYPE_PIXELS_UYVY422: {
			const GLfloat TextureSize[2] = { static_cast<GLfloat>(m_uStridesTexture[planeId]), static_cast<GLfloat>(m_uHeightsTexture[planeId]) };
			COMPV_glGetUniformLocation(&uNameLocation, CompVGLRenderer::m_ptrBlitter->program()->name(), "TextureSize");
			COMPV_glUniform2fv(uNameLocation, 1, TextureSize);
			break;
		}
		default:
			break;
		}
    }

bail:
	for (size_t planeId = 0; planeId < mat->planeCount(); ++planeId) {
		COMPV_glActiveTexture(GLenum(GL_TEXTURE1 + planeId));
		COMPV_glBindTexture(GL_TEXTURE_2D, kCompVGLNameInvalid);
	}
	COMPV_CHECK_CODE_NOP(m_ptrBlitter->unbind());
    if (COMPV_ERROR_CODE_IS_NOK(err)) {
        COMPV_CHECK_CODE_NOP(deInit());
    }
    return err;
}

COMPV_ERROR_CODE CompVGLRenderer::deInit()
{
	if (!m_bInit) {
		return COMPV_ERROR_CODE_S_OK;
	}
    COMPV_CHECK_EXP_RETURN(!CompVGLUtils::isGLContextSet(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);
    COMPV_CHECK_CODE_NOP(m_ptrBlitter->deInit());
    for (size_t t = 0; t < m_uTexturesCount; ++t) {
		COMPV_CHECK_CODE_NOP(CompVGLUtils::textureDelete(&m_uNameTextures[t]));
        m_uWidths[t] = 0;
        m_uHeights[t] = 0;
        m_uStrides[t] = 0;
		m_uWidthsTexture[t] = 0;
		m_uHeightsTexture[t] = 0;
		m_uStridesTexture[t] = 0;
    }
    m_uTexturesCount = 0;

    m_bInit = false;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLRenderer::newObj(CompVGLRendererPtrPtr glRenderer, COMPV_SUBTYPE ePixelFormat)
{
    COMPV_CHECK_CODE_RETURN(CompVGL::init());
    COMPV_CHECK_EXP_RETURN(!glRenderer, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    COMPV_CHECK_EXP_RETURN(
        ePixelFormat != COMPV_SUBTYPE_PIXELS_Y
        && ePixelFormat != COMPV_SUBTYPE_PIXELS_YUV420P
		&& ePixelFormat != COMPV_SUBTYPE_PIXELS_YVU420P
		&& ePixelFormat != COMPV_SUBTYPE_PIXELS_NV12
		&& ePixelFormat != COMPV_SUBTYPE_PIXELS_NV21
		&& ePixelFormat != COMPV_SUBTYPE_PIXELS_YUYV422
		&& ePixelFormat != COMPV_SUBTYPE_PIXELS_UYVY422
		&& ePixelFormat != COMPV_SUBTYPE_PIXELS_YUV422P
		&& ePixelFormat != COMPV_SUBTYPE_PIXELS_YUV444P
		&& ePixelFormat != COMPV_SUBTYPE_PIXELS_RGB24
		&& ePixelFormat != COMPV_SUBTYPE_PIXELS_BGR24
		&& ePixelFormat != COMPV_SUBTYPE_PIXELS_RGBA32
		&& ePixelFormat != COMPV_SUBTYPE_PIXELS_BGRA32
		&& ePixelFormat != COMPV_SUBTYPE_PIXELS_ABGR32
		&& ePixelFormat != COMPV_SUBTYPE_PIXELS_ARGB32
		&& ePixelFormat != COMPV_SUBTYPE_PIXELS_RGB565LE
		&& ePixelFormat != COMPV_SUBTYPE_PIXELS_RGB565BE
		&& ePixelFormat != COMPV_SUBTYPE_PIXELS_BGR565LE
		&& ePixelFormat != COMPV_SUBTYPE_PIXELS_BGR565BE,
        COMPV_ERROR_CODE_E_INVALID_PARAMETER);

    CompVGLRendererPtr glRenderer_ = new CompVGLRenderer(ePixelFormat);
    COMPV_CHECK_EXP_RETURN(!glRenderer_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	COMPV_CHECK_CODE_RETURN(CompVGLBlitter::newObj(&glRenderer_->m_ptrBlitter));

	// Vertex and fragment programs source
	glRenderer_->m_strPrgVertexData = kProgramVertexData;
	switch (ePixelFormat) {
	case COMPV_SUBTYPE_PIXELS_YUV420P:
		glRenderer_->m_strPrgFragData = kProgramFragmentDataYUV420P;
		break;
	case COMPV_SUBTYPE_PIXELS_YVU420P:
		glRenderer_->m_strPrgFragData = kProgramFragmentDataYVU420P;
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
	case COMPV_SUBTYPE_PIXELS_YUYV422:
		glRenderer_->m_strPrgFragData = kProgramFragmentDataYUYV422;
		break;
	case COMPV_SUBTYPE_PIXELS_UYVY422:
		glRenderer_->m_strPrgFragData = kProgramFragmentDataUYVY422;
		break;
	case COMPV_SUBTYPE_PIXELS_YUV422P:
		glRenderer_->m_strPrgFragData = kProgramFragmentDataYUV422P;
		break;
	case COMPV_SUBTYPE_PIXELS_YUV444P:
		glRenderer_->m_strPrgFragData = kProgramFragmentDataYUV444P;
		break;
	case COMPV_SUBTYPE_PIXELS_RGB24:
		glRenderer_->m_strPrgFragData = kProgramFragmentDataRGB24;
		break;
	case COMPV_SUBTYPE_PIXELS_BGR24:
		glRenderer_->m_strPrgFragData = kProgramFragmentDataBGR24;
		break;
	case COMPV_SUBTYPE_PIXELS_RGBA32:
		glRenderer_->m_strPrgFragData = kProgramFragmentDataRGBA32;
		break;
	case COMPV_SUBTYPE_PIXELS_BGRA32:
		glRenderer_->m_strPrgFragData = kProgramFragmentDataBGRA32;
		break;
	case COMPV_SUBTYPE_PIXELS_ABGR32:
		glRenderer_->m_strPrgFragData = kProgramFragmentDataABGR32;
		break;
	case COMPV_SUBTYPE_PIXELS_ARGB32:
		glRenderer_->m_strPrgFragData = kProgramFragmentDataARGB32;
		break;
	case COMPV_SUBTYPE_PIXELS_RGB565LE:
#if defined(HAVE_OPENGLES)
		COMPV_CHECK_EXP_RETURN(CompVBase::isBigEndian(), COMPV_ERROR_CODE_E_GL);
#endif
		glRenderer_->m_strPrgFragData = CompVBase::isLittleEndian() ? kProgramFragmentDataRGB565Native : kProgramFragmentDataRGB565LE;
		break;
	case COMPV_SUBTYPE_PIXELS_RGB565BE:
#if defined(HAVE_OPENGLES)
		COMPV_CHECK_EXP_RETURN(CompVBase::isLittleEndian(), COMPV_ERROR_CODE_E_GL);
#endif
		glRenderer_->m_strPrgFragData = CompVBase::isBigEndian() ? kProgramFragmentDataRGB565Native : kProgramFragmentDataRGB565BE;
		break;
	case COMPV_SUBTYPE_PIXELS_BGR565LE:
#if defined(HAVE_OPENGLES)
		COMPV_CHECK_EXP_RETURN(CompVBase::isBigEndian(), COMPV_ERROR_CODE_E_GL);
#endif
		glRenderer_->m_strPrgFragData = CompVBase::isLittleEndian() ? kProgramFragmentDataBGR565Native : kProgramFragmentDataBGR565LE;
		break;
	case COMPV_SUBTYPE_PIXELS_BGR565BE:
#if defined(HAVE_OPENGLES)
		COMPV_CHECK_EXP_RETURN(CompVBase::isLittleEndian(), COMPV_ERROR_CODE_E_GL);
#endif
		glRenderer_->m_strPrgFragData = CompVBase::isBigEndian() ? kProgramFragmentDataBGR565Native : kProgramFragmentDataBGR565BE;
		break;
	default:
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT);
		break;
	}

	// Samplers and and formats
	switch (ePixelFormat) {
	case COMPV_SUBTYPE_PIXELS_YUV420P:
	case COMPV_SUBTYPE_PIXELS_YUV422P:
	case COMPV_SUBTYPE_PIXELS_YUV444P:
	case COMPV_SUBTYPE_PIXELS_Y:
		glRenderer_->m_pSamplerNames[0] = "SamplerY", glRenderer_->m_eFormats[0] = COMPV_GL_FORMAT_Y;
		glRenderer_->m_pSamplerNames[1] = "SamplerU", glRenderer_->m_eFormats[1] = COMPV_GL_FORMAT_U;
		glRenderer_->m_pSamplerNames[2] = "SamplerV", glRenderer_->m_eFormats[2] = COMPV_GL_FORMAT_V;
		glRenderer_->m_ePixelDataType = GL_UNSIGNED_BYTE;
		break;
	case COMPV_SUBTYPE_PIXELS_YVU420P: // YUV420  but V plane first
		glRenderer_->m_pSamplerNames[0] = "SamplerY", glRenderer_->m_eFormats[0] = COMPV_GL_FORMAT_Y;
		glRenderer_->m_pSamplerNames[1] = "SamplerV", glRenderer_->m_eFormats[1] = COMPV_GL_FORMAT_V;
		glRenderer_->m_pSamplerNames[2] = "SamplerU", glRenderer_->m_eFormats[2] = COMPV_GL_FORMAT_U;
		glRenderer_->m_ePixelDataType = GL_UNSIGNED_BYTE;
		break;
	case COMPV_SUBTYPE_PIXELS_NV12:
	case COMPV_SUBTYPE_PIXELS_NV21:
		glRenderer_->m_pSamplerNames[0] = "SamplerY", glRenderer_->m_eFormats[0] = COMPV_GL_FORMAT_Y;
		glRenderer_->m_pSamplerNames[1] = "SamplerUV", glRenderer_->m_eFormats[1] = COMPV_GL_FORMAT_UV;
		glRenderer_->m_ePixelDataType = GL_UNSIGNED_BYTE;
		break;
	case COMPV_SUBTYPE_PIXELS_RGB24:
	case COMPV_SUBTYPE_PIXELS_BGR24:
		glRenderer_->m_pSamplerNames[0] = "SamplerRGB", glRenderer_->m_eFormats[0] = COMPV_GL_FORMAT_RGB;
		glRenderer_->m_ePixelDataType = GL_UNSIGNED_BYTE;
		break;
	case COMPV_SUBTYPE_PIXELS_RGB565LE:
	case COMPV_SUBTYPE_PIXELS_BGR565LE:
		if (CompVBase::isLittleEndian()) {
			glRenderer_->m_pSamplerNames[0] = "SamplerRG565", glRenderer_->m_eFormats[0] = COMPV_GL_FORMAT_RGB;
			glRenderer_->m_ePixelDataType = GL_UNSIGNED_SHORT_5_6_5;
		}
		else {
			glRenderer_->m_pSamplerNames[0] = "SamplerRG565", glRenderer_->m_eFormats[0] = COMPV_GL_FORMAT_RG;
			glRenderer_->m_ePixelDataType = GL_UNSIGNED_BYTE;
		}
		break;
	case COMPV_SUBTYPE_PIXELS_RGB565BE:
	case COMPV_SUBTYPE_PIXELS_BGR565BE:
		if (CompVBase::isBigEndian()) {
			glRenderer_->m_pSamplerNames[0] = "SamplerRG565", glRenderer_->m_eFormats[0] = COMPV_GL_FORMAT_RGB;
			glRenderer_->m_ePixelDataType = GL_UNSIGNED_SHORT_5_6_5;
		}
		else {
			glRenderer_->m_pSamplerNames[0] = "SamplerRG565", glRenderer_->m_eFormats[0] = COMPV_GL_FORMAT_RG;
			glRenderer_->m_ePixelDataType = GL_UNSIGNED_BYTE;
		}
		break;
	case COMPV_SUBTYPE_PIXELS_YUYV422:
	case COMPV_SUBTYPE_PIXELS_UYVY422:
	case COMPV_SUBTYPE_PIXELS_RGBA32:
	case COMPV_SUBTYPE_PIXELS_BGRA32:
	case COMPV_SUBTYPE_PIXELS_ABGR32:
	case COMPV_SUBTYPE_PIXELS_ARGB32:
		glRenderer_->m_pSamplerNames[0] = "SamplerRGBA", glRenderer_->m_eFormats[0] = COMPV_GL_FORMAT_RGBA;
		glRenderer_->m_ePixelDataType = GL_UNSIGNED_BYTE;
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
