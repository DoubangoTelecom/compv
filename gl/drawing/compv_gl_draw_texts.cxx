/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/gl/drawing/compv_gl_draw_texts.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/base/compv_mat.h"
#include "compv/base/math/compv_math.h"
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
"		gl_Position = MVP * vec4(coord.xy, 1.0, 1.0);"
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
"		gl_FragColor = vec4(1.0, 1.0, 1.0, texture2D(tex, texcoord).r) * color;"
"	}";

#define kVertexDataWithMVP_Yes	true
#define kVertexDataWithMVP_No	false

struct CompVGLFreeTypeBitmap {
	GLfloat left;
	GLfloat top;
	GLfloat width;
	GLfloat rows;
	GLfloat advance_x;
	GLfloat advance_y;
};

#define kMaxFreeTypeCharsPerProcess 3 // FIXME(dmi): use #1000

#define COMPV_THIS_CLASS_NAME "CompVGLDrawTexts"

// FreeType tutos:
//	- https://www.freetype.org/freetype2/docs/tutorial/step1.html
//	- https://learnopengl.com/#!In-Practice/Text-Rendering
//  - Implementation based on https://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_Text_Rendering_01

COMPV_NAMESPACE_BEGIN()

CompVGLDrawTexts::CompVGLDrawTexts()
	: CompVGLDraw(kProgramVertexData, kProgramFragmentData, kVertexDataWithMVP_Yes)
	, m_fboWidth(0)
	, m_fboHeight(0)
	, m_uTextureAtlas(kCompVGLNameInvalid)
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
	if (m_uTextureAtlas != kCompVGLNameInvalid) {
		if (CompVGLUtils::currentContext()) {
			COMPV_CHECK_CODE_NOP(CompVGLUtils::textureDelete(&m_uTextureAtlas));
		}
		else {
			COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASS_NAME, "No OpenGL context: %d texture will leak", m_uTextureAtlas);
		}
	}
}

COMPV_ERROR_CODE CompVGLDrawTexts::texts(const CompVStringVector& texts, const CompVPointFloat32Vector& positions, const CompVDrawingOptions* options COMPV_DEFAULT(nullptr))
{
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Change function parameters to accept CompVGLText2D* like what is done for drawPoints and drawLines");
#if HAVE_FREETYPE
	const bool randomColors = (!options || options->colorType == COMPV_DRAWING_COLOR_TYPE_RANDOM);
	static const std::string fontName = "C:/Windows/Fonts/arial.ttf";
	static const size_t fontSize = 16;

	CompVStringVector::const_iterator it_texts;
	CompVPointFloat32Vector::const_iterator it_positions;
	std::string::const_iterator it_string;
	compv_float32_t x, y;
	CompVMatPtr ptrAtlas, ptrBitmaps;

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
		if (bFirstTimeOrChanged) {
			// Create texture if not already done
			if (m_uTextureAtlas == kCompVGLNameInvalid) {
				COMPV_CHECK_CODE_RETURN(CompVGLUtils::textureGen(&m_uTextureAtlas));
				COMPV_glBindTexture(GL_TEXTURE_2D, m_uTextureAtlas);
				COMPV_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				COMPV_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				COMPV_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				COMPV_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			}
			COMPV_glBindTexture(GL_TEXTURE_2D, m_uTextureAtlas);
			COMPV_glTexImage2D(
				GL_TEXTURE_2D,
				0,
				COMPV_GL_FORMAT_Y,
				fboWidth,
				fboHeight,
				0,
				COMPV_GL_FORMAT_Y,
				GL_UNSIGNED_BYTE,
				nullptr
			);
		}
		// Set coords. attribute
		COMPV_glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 4 * 4, NULL, GL_DYNAMIC_DRAW);
		GLuint attribute_coord = COMPV_glGetAttribLocation(program()->name(), "coord");
		COMPV_glEnableVertexAttribArray(attribute_coord);
		COMPV_glVertexAttribPointer(attribute_coord, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

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

	COMPV_glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	COMPV_glActiveTexture(GL_TEXTURE0);
	COMPV_glBindTexture(GL_TEXTURE_2D, m_uTextureAtlas);
	
	// Create texture
	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Create texture once");

	// Build atlas and send data to texture
	COMPV_CHECK_CODE_RETURN((CompVMat::newObj<uint8_t>(&ptrAtlas, fboHeight, fboWidth, 1, fboWidth)));
	COMPV_CHECK_CODE_RETURN((CompVMat::newObj<CompVGLFreeTypeBitmap, COMPV_MAT_TYPE_STRUCT>(&ptrBitmaps, 1, kMaxFreeTypeCharsPerProcess, 1, kMaxFreeTypeCharsPerProcess)));
	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Comment zeroall");
	ptrAtlas->zero_all();
	COMPV_CHECK_CODE_BAIL(err = fillAtlas(texts, positions, ptrAtlas, ptrBitmaps));
	COMPV_glTexSubImage2D(
		GL_TEXTURE_2D,
		0,
		0,
		0,
		static_cast<GLsizei>(ptrAtlas->cols()),
		static_cast<GLsizei>(ptrAtlas->rows()),
		COMPV_GL_FORMAT_Y,
		GL_UNSIGNED_BYTE,
		ptrAtlas->ptr());

	// this affects glTexImage2D and glSubTexImage2D

	for (it_texts = texts.begin(), it_positions = positions.begin(); it_texts < texts.end(); ++it_texts, ++it_positions) {
		x = it_positions->x;
		y = it_positions->y;
		for (it_string = it_texts->begin(); it_string < it_texts->end(); ++it_string) {
			if ((ft_err = FT_Load_Char(m_face, *it_string, FT_LOAD_RENDER))) {
				COMPV_DEBUG_WARN_EX(COMPV_THIS_CLASS_NAME, "FT_Load_Char(face, %c) failed with error code %d", ft_err, *it_string);
				continue;
			}

			/*COMPV_glTexImage2D(
				GL_TEXTURE_2D,
				0,
				COMPV_GL_FORMAT_Y,
				g->bitmap.width,
				g->bitmap.rows,
				0,
				COMPV_GL_FORMAT_Y,
				GL_UNSIGNED_BYTE,
				g->bitmap.buffer
			);*/
			float x2 = x + g->bitmap_left * 1.f;
			float y2 = y - g->bitmap_top * 1.f;
			float w = g->bitmap.width * 1.f;
			float h = g->bitmap.rows * 1.f;

			float sx = 1.f / fboWidth;
			float sy = 1.f / fboHeight;
			float cx = x * sx;
			float cy = y * sy;
			float sw = w * sx;
			float sh = h * sy;

#if 0
			GLfloat box[4][4] = {
				{ x2,     y2    , 0.f, 0.f }, // (fbo_x, fbo_y), (texture_x, texture_y)
				{ x2 + w, y2    , 1.f, 0.f },
				{ x2,     y2 + h, 0.f, 1.f },
				{ x2 + w, y2 + h, 1.f, 1.f },
			};
#else
			GLfloat box[4][4] = {
				{ x2,     y2    , cx, cy }, // (fbo_x, fbo_y), (texture_x, texture_y)
				{ x2 + w, y2    , cx + sw, cy },
				{ x2,     y2 + h, cx, cy + sh },
				{ x2 + w, y2 + h, cx + sw, cy + sh },
			};
#endif

			COMPV_DEBUG_INFO_CODE_FOR_TESTING("Call COMPV_glBufferData and COMPV_glDrawArrays once");

			// Submit vertices data
			COMPV_glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(box), box);

			// Draw points
			COMPV_glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			x += (g->advance.x >> 6) * 1.f;
			y += (g->advance.y >> 6) * 1.f;
		}
	}

	// Update size
	m_fboWidth = fboWidth;
	m_fboHeight = fboHeight;

bail:
	COMPV_CHECK_CODE_NOP(CompVGLDraw::unbind());
#else
	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED, "No thirdparty library for text drawing could be found (e.g. freetype)");
#endif /* HAVE_FREETYPE */

	return err;
}

#if HAVE_FREETYPE

COMPV_ERROR_CODE CompVGLDrawTexts::fillAtlas(const CompVStringVector& texts, const CompVPointFloat32Vector& positions, CompVMatPtr& ptrAtlas, CompVMatPtr& ptrBitmaps)
{
	// Internal function, do not check input parameters
	CompVStringVector::const_iterator it_texts = texts.begin();
	CompVPointFloat32Vector::const_iterator it_positions;
	std::string::const_iterator it_string;
	uint8_t *atlas_buffer, *bitmap_buffer;

	GLuint ai, bi, xi, yi;
	FT_GlyphSlot g = m_face->glyph;
	FT_Error ft_err;

	const GLuint fboWidth = static_cast<GLuint>(ptrAtlas->cols());
	const GLuint fboHeight = static_cast<GLuint>(ptrAtlas->rows());
	
	size_t countBitmaps = 0;
	const size_t maxBitmpas = ptrBitmaps->cols();
	CompVGLFreeTypeBitmap* ptrBitmap = ptrBitmaps->ptr<CompVGLFreeTypeBitmap>();

	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Uncomment all COMPV_DEBUG_WARN_EX");

	for (it_texts = texts.begin(), it_positions = positions.begin(); it_texts < texts.end(); ++it_texts, ++it_positions) {
		if (it_positions->x < 0.f || it_positions->y < 0.f || (xi = COMPV_MATH_ROUNDFU_2_NEAREST_INT(it_positions->x, GLuint)) >= fboWidth || (yi = COMPV_MATH_ROUNDFU_2_NEAREST_INT(it_positions->y, GLuint)) >= fboHeight) { // cast to unsigned, this is why comparison against 0 must be done before
			//COMPV_DEBUG_WARN_EX(COMPV_THIS_CLASS_NAME, "Trying to write outside the screen domain (start): skip");
			continue;
		}
		for (it_string = it_texts->begin(); it_string < it_texts->end(); ++it_string) {
			if ((ft_err = FT_Load_Char(m_face, *it_string, FT_LOAD_RENDER))) {
				COMPV_DEBUG_WARN_EX(COMPV_THIS_CLASS_NAME, "FT_Load_Char(face, %c) failed with error code %d", ft_err, *it_string);
				continue;
			}

			// Do not write partial chars
			if ((xi + g->bitmap.width) >= fboWidth || (yi + g->bitmap.rows) >= fboHeight) {
				//COMPV_DEBUG_WARN_EX(COMPV_THIS_CLASS_NAME, "Trying to write outside the screen domain (partial char): skip");
				break; // end the string
			}

			COMPV_DEBUG_INFO_CODE_FOR_TESTING("compute atlas_buffer once for each string then advance");

			// Write bitmap to buffer
			bitmap_buffer = g->bitmap.buffer;
			atlas_buffer = ptrAtlas->ptr<uint8_t>(yi, xi);
			for (bi = 0; bi < g->bitmap.rows; ++bi) {
				for (ai = 0; ai < g->bitmap.width; ++ai) {
					atlas_buffer[ai] = bitmap_buffer[ai];
				}
				bitmap_buffer += g->bitmap.width;
				atlas_buffer += fboWidth;
			}
			
			xi += static_cast<GLuint>(g->advance.x >> 6);
			yi += static_cast<GLuint>(g->advance.y >> 6);
			
			if (++countBitmaps > maxBitmpas) {
				//COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASS_NAME, "Breaking FreeType processing because we reached the maximum bitmaps per process (%zu)", countBitmaps);
				//return COMPV_ERROR_CODE_E_OUT_OF_BOUND;
			}
		}		
	}

	return COMPV_ERROR_CODE_S_OK;
}

#endif /* HAVE_FREETYPE */

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

