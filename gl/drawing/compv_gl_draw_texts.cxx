/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/gl/drawing/compv_gl_draw_texts.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/base/compv_mat.h"
#include "compv/gl/compv_gl.h"
#include "compv/gl/compv_gl_utils.h"
#include "compv/gl/compv_gl_info.h"
#include "compv/gl/compv_gl_func.h"

#if defined(HAVE_OPENGLES)
#	define COMPV_GL_FORMAT_Y	GL_LUMINANCE
#else
#	define COMPV_GL_FORMAT_Y	GL_RED
#endif

static const std::string& kProgramVertexData =
#	if defined(HAVE_OPENGLES)
"	precision mediump float;"
#	endif
"	attribute vec4 coord;"
"	varying vec2 texcoord;"
"	uniform mat4 MVP;"
"	void main() {"
"		gl_Position = vec4(coord.xy, 0.0, 1.0);"
"		texcoord = coord.zw;"
"	}";

static const std::string& kProgramFragmentData =
#	if defined(HAVE_OPENGLES)
"	precision mediump float;"
#	endif
"	varying vec2 texcoord;"
"	uniform sampler2D tex;"
"	uniform vec4 color;"
"	void main() {"
"		gl_FragColor = vec4(1, 1, 1, texture2D(tex, texcoord).r) * color;"
"	}";

#define kVertexDataWithMVP_Yes	true
#define kVertexDataWithMVP_No	false

// FreeType tutos:
//	- https://www.freetype.org/freetype2/docs/tutorial/step1.html
//	- https://learnopengl.com/#!In-Practice/Text-Rendering

COMPV_NAMESPACE_BEGIN()

CompVGLDrawTexts::CompVGLDrawTexts()
	: CompVGLDraw(kProgramVertexData, kProgramFragmentData, kVertexDataWithMVP_Yes)
	, m_fboWidth(0)
	, m_fboHeight(0)
#if HAVE_FREETYPE
	, m_face(nullptr)
#endif
{

}

CompVGLDrawTexts::~CompVGLDrawTexts()
{
#if HAVE_FREETYPE
	if (m_face) {
		if (CompVGLFreeType::library()) {
			COMPV_CHECK_EXP_NOP(FT_Done_Face(m_face) != 0, COMPV_ERROR_CODE_E_FREETYPE);
		}
		m_face = nullptr;
	}
#endif
}

COMPV_ERROR_CODE CompVGLDrawTexts::texts(const CompVStringVector& texts, const CompVPointFloat32Vector& positions, const CompVDrawingOptions* options COMPV_DEFAULT(nullptr))
{
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Change function parameters to accept CompVGLText2D* like what is done for drawPoints and drawLines");
#if HAVE_FREETYPE
	const bool randomColors = (!options || options->colorType == COMPV_DRAWING_COLOR_TYPE_RANDOM);
	static const std::string fontName = "C:/Windows/Fonts/arial.ttf";
	static const size_t fontSize = 16;

	static size_t count = 188548;
	const std::string str = std::string("Mamadou DIOP (Mauritanie): ") + std::to_string(++count);
	const char *p;
	const char *text = str.c_str();

	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Change m_face if font or pixel size change");

	FT_Error ft_err;
	if (!m_face) {
		COMPV_DEBUG_INFO_CODE_FOR_TESTING("Use relative path for the the font or read from memory");
		COMPV_DEBUG_INFO_CODE_FOR_TESTING("Under windows check fonts in 'C:/Windows/Fonts'");
		if ((ft_err = FT_New_Face(CompVGLFreeType::library(), "C:/Windows/Fonts/arial.ttf", 0, &m_face))) {
			COMPV_DEBUG_ERROR("FT_New_Face('FreeSans.ttf', 0) failed with error '%s'", CompVGLFreeType::errorMessage(ft_err));
			return COMPV_ERROR_CODE_E_FREETYPE;
		}
		if ((ft_err = FT_Set_Pixel_Sizes(m_face, 0, 16))) {
			COMPV_DEBUG_ERROR("FT_Set_Pixel_Sizes(face, 0, 16) failed with error '%s'", CompVGLFreeType::errorMessage(ft_err));
			return COMPV_ERROR_CODE_E_FREETYPE;
		}
	}

	FT_GlyphSlot g = m_face->glyph;

	GLuint nameTexture = kCompVGLNameInvalid;

	// Bind to VAO, VBO, Program
	GLint fboWidth = 0, fboHeight = 0;
	bool bFirstTimeOrChanged;
	COMPV_CHECK_CODE_BAIL(err = CompVGLDraw::bind());

	// Get FBO width
	COMPV_glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &fboWidth);
	COMPV_glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &fboHeight);
	COMPV_CHECK_EXP_BAIL(!fboWidth || !fboHeight, (err = COMPV_ERROR_CODE_E_GL), "fboWidth or fboHeight is equal to zero");
	bFirstTimeOrChanged = (m_fboWidth != fboWidth || m_fboHeight != fboHeight);

	if (!CompVGLInfo::extensions::vertex_array_object() || bFirstTimeOrChanged) {

		COMPV_glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 4 * 4, NULL, GL_DYNAMIC_DRAW);

		// Set position attribute
		GLuint attribute_coord = COMPV_glGetAttribLocation(program()->name(), "coord");
		COMPV_glEnableVertexAttribArray(attribute_coord);
		COMPV_glVertexAttribPointer(attribute_coord, 4, GL_FLOAT, GL_FALSE, 0, 0);

		// Set color attribute
		GLuint uniform_color = COMPV_glGetUniformLocation(program()->name(), "color");
		GLfloat r, g, b, a;
		if (randomColors) {
			const GLfloat(*c)[3] = &kCompVGLRandomColors[rand() % kCompVGLRandomColorsCount];
			r = (*c)[0], g = (*c)[1], b = (*c)[2], a = 1.f;
		}
		else {
			r = options->color[0], g = options->color[1], b = options->color[2], a = options->color[3];
		}
		COMPV_DEBUG_INFO_CODE_FOR_TESTING("Add support for glUniform4f");
		glUniform4f(uniform_color, r, g, b, a);

		// Set texture attribute
		GLuint uniform_tex = COMPV_glGetUniformLocation(program()->name(), "tex");
		glUniform1i(uniform_tex, 0);

		// Set projection
		COMPV_CHECK_CODE_BAIL(err = CompVGLDraw::setOrtho(0, static_cast<GLfloat>(fboWidth), static_cast<GLfloat>(fboHeight), 0, -1, 1));
	}
	else if (randomColors) {
		GLuint uniform_color = COMPV_glGetUniformLocation(program()->name(), "color");
		const GLfloat(*c)[3] = &kCompVGLRandomColors[rand() % kCompVGLRandomColorsCount];
		COMPV_DEBUG_INFO_CODE_FOR_TESTING("Add support for glUniform4f");
		glUniform4f(uniform_color, (*c)[0], (*c)[1], (*c)[2], 1.f);
	}

	// Set viewport
	COMPV_glViewport(0, 0, static_cast<GLsizei>(fboWidth), static_cast<GLsizei>(fboHeight));

	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Under windows check fonts in 'C:/Windows/Fonts'");

	float sx = 2.f / static_cast<float>(fboWidth);
	float sy = 2.f / static_cast<float>(fboHeight);
	float x = 0.f;
	float y = 0.f;

	COMPV_glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	// Create texture
	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Create texture once");
	COMPV_glActiveTexture(GL_TEXTURE0);
	COMPV_CHECK_CODE_BAIL(err = CompVGLUtils::textureGen(&nameTexture));
	COMPV_glBindTexture(GL_TEXTURE_2D, nameTexture);
	COMPV_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	COMPV_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	COMPV_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	COMPV_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	COMPV_glActiveTexture(GL_TEXTURE0);

	// this affects glTexImage2D and glSubTexImage2D

	for (p = text; *p; p++) {
		if ((ft_err = FT_Load_Char(m_face, *p, FT_LOAD_RENDER))) {
			COMPV_DEBUG_ERROR("FT_Load_Char(face, %c) failed with error code %d", ft_err, *p);
			continue;
		}

		COMPV_glTexImage2D(
			GL_TEXTURE_2D,
			0,
			COMPV_GL_FORMAT_Y,
			g->bitmap.width,
			g->bitmap.rows,
			0,
			COMPV_GL_FORMAT_Y,
			GL_UNSIGNED_BYTE,
			g->bitmap.buffer
		);

		float x2 = x + g->bitmap_left * sx;
		float y2 = -y - g->bitmap_top * sy;
		float w = g->bitmap.width * sx;
		float h = g->bitmap.rows * sy;

		GLfloat box[4][4] = {
			{ x2,     -y2    , 0, 0 },
			{ x2 + w, -y2    , 1, 0 },
			{ x2,     -y2 - h, 0, 1 },
			{ x2 + w, -y2 - h, 1, 1 },
		};

		COMPV_DEBUG_INFO_CODE_FOR_TESTING("Call COMPV_glBufferData and COMPV_glDrawArrays once");

		// Submit vertices data
		COMPV_glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(box), box);

		// Draw points
		COMPV_glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		x += (g->advance.x / 64) * sx;
		y += (g->advance.y / 64) * sy;

		COMPV_DEBUG_INFO_CODE_FOR_TESTING("Remove");
		//CompVGLFreeType::character_remove(*p, style);
	}

	// Update size
	m_fboWidth = fboWidth;
	m_fboHeight = fboHeight;

bail:
	COMPV_CHECK_CODE_NOP( CompVGLUtils::textureDelete(&nameTexture));
	COMPV_CHECK_CODE_NOP(CompVGLDraw::unbind());
#else
	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED, "No thirdparty library for text drawing could be found (e.g. freetype)");
#endif /* HAVE_FREETYPE */

	return err;
}

COMPV_ERROR_CODE CompVGLDrawTexts::newObj(CompVGLDrawTextsPtrPtr drawTexts)
{
#if HAVE_FREETYPE
	COMPV_CHECK_EXP_RETURN(!drawTexts, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVGLDrawTextsPtr drawTexts_ = new CompVGLDrawTexts();
	COMPV_CHECK_EXP_RETURN(!drawTexts_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

	*drawTexts = drawTexts_;
#else
	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED, "No thirdparty library for text drawing could be found (e.g. freetype)");
#endif /* HAVE_FREETYPE */

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

