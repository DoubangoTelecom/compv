/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/gl/drawing/compv_gl_draw_texts.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/base/compv_mat.h"
#include "compv/base/utf8/utf8.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/time/compv_time.h"
#include "compv/gl/compv_gl.h"
#include "compv/gl/compv_gl_utils.h"
#include "compv/gl/compv_gl_info.h"
#include "compv/gl/compv_gl_func.h"

#include <codecvt>
#include <locale>

#if defined(HAVE_OPENGLES)
#	define COMPV_GL_FORMAT_Y	GL_LUMINANCE
#else
#	define COMPV_GL_FORMAT_Y	GL_RED
#endif

static const std::string& kProgramVertexData =
#	if defined(HAVE_OPENGLES)
"	precision mediump float;"
#	endif
"	attribute vec4 box;"
"	varying vec2 texcoord;"
"	uniform mat4 MVP;"
"	void main() {"
"		gl_Position = MVP * vec4(box.xy, 1.0, 1.0);"
"		texcoord = box.zw;"
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

typedef GLfloat CompVGLFreeTypeBox[4 * 6]; // (fbo_x, fbo_y, tex_x, tex_y) * 6

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
	const bool utf8 = (options && options->fontUtf8);
	static const std::string fontName = "C:/Users/dmi/Downloads/arial-unicode-ms/ARIALUNI.ttf";
	static const size_t fontSize = 16;

	CompVStringVector::const_iterator it_texts;
	CompVPointFloat32Vector::const_iterator it_positions;
	std::string::const_iterator it_string;
	CompVMatPtr ptrAtlas, ptrBoxes;
	const CompVGLFreeTypeBox* boxesPtr;
	size_t numChars, maxChars;

	COMPV_DEBUG_INFO_CODE_FOR_TESTING("use %%windir%% to retrieve windows directory");

	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Change m_face if font or pixel size change");

	FT_Error ft_err;
	if (!m_face) {
		COMPV_DEBUG_INFO_CODE_FOR_TESTING("Use relative path for the the font or read from memory");
		COMPV_DEBUG_INFO_CODE_FOR_TESTING("Under windows check fonts in 'C:/Windows/Fonts'");
		if ((ft_err = FT_New_Face(CompVGLFreeType::library(), "C:/Windows/Fonts/arial.ttf", 0, &m_face))) {
			COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASS_NAME, "FT_New_Face('FreeSans.ttf', 0) failed with error '%s'", CompVGLFreeType::errorMessage(ft_err));
			return COMPV_ERROR_CODE_E_FREETYPE;
		}
		if ((ft_err = FT_Select_Charmap(m_face, ft_encoding_unicode))) {
			COMPV_DEBUG_WARN_EX(COMPV_THIS_CLASS_NAME, "FT_Select_Charmap(face, ft_encoding_unicode) failed with error '%s'", CompVGLFreeType::errorMessage(ft_err));
		}
		if ((ft_err = FT_Set_Pixel_Sizes(m_face, 0, fontSize))) {
			COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASS_NAME, "FT_Set_Pixel_Sizes(face, 0, 16) failed with error '%s'", CompVGLFreeType::errorMessage(ft_err));
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
		// Set boxes attribute
		GLuint attribute_box = COMPV_glGetAttribLocation(program()->name(), "box");
		COMPV_glEnableVertexAttribArray(attribute_box);
		COMPV_glVertexAttribPointer(attribute_box, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, 0);

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
		COMPV_glUniform1i(uniform_tex, 0); // GL_TEXTURE0

										   // Set projection
		COMPV_CHECK_CODE_BAIL(err = CompVGLDraw::setOrtho(0, static_cast<GLfloat>(fboWidth), static_cast<GLfloat>(fboHeight), 0, -1, 1));
	}
	else if (randomColors) {
		GLuint uniform_color = COMPV_glGetUniformLocation(program()->name(), "color");
		const GLfloat(*c)[3] = &kCompVGLRandomColors[rand() % kCompVGLRandomColorsCount];
		COMPV_DEBUG_INFO_CODE_FOR_TESTING("Add support for glUniform4f");
		glUniform4f(uniform_color, (*c)[0], (*c)[1], (*c)[2], 1.f);
	}


	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Under windows check fonts in 'C:/Windows/Fonts'");

	// Create texture
	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Create texture once");

	// Get maximum chars
	maxChars = (fboWidth * fboHeight) /*/ fontSize*/; // do not divide by fontSize to allow overwritting and give some room

	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Create MT implementation for fillAtlas and compute numChars for each thread");

	// Get number of chars
	numChars = 0;
	for (it_texts = texts.begin(); it_texts < texts.end(); ++it_texts) {
		numChars += it_texts->size();
	}
	if (numChars > maxChars) {
		COMPV_DEBUG_WARN_EX(COMPV_THIS_CLASS_NAME, "Too much characters to write, truncating (%zu > %zu)", numChars, maxChars);
		numChars = maxChars;
	}

	// Build atlas and send data to texture
	COMPV_glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	COMPV_glActiveTexture(GL_TEXTURE0);
	COMPV_glBindTexture(GL_TEXTURE_2D, m_uTextureAtlas);
	COMPV_CHECK_CODE_RETURN((CompVMat::newObj<uint8_t>(&ptrAtlas, fboHeight, fboWidth, 1, fboWidth)));
	COMPV_CHECK_CODE_RETURN((CompVMat::newObj<CompVGLFreeTypeBox, COMPV_MAT_TYPE_STRUCT>(&ptrBoxes, 1, numChars, 1, numChars)));
	COMPV_CHECK_CODE_BAIL(err = fillAtlas(utf8, texts, positions, ptrAtlas, ptrBoxes, numChars));
	//uint64_t timeStart = CompVTime::nowMillis();
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

	// Submit vertices data
	boxesPtr = ptrBoxes->ptr<const CompVGLFreeTypeBox>();

	COMPV_glViewport(0, 0, static_cast<GLsizei>(fboWidth), static_cast<GLsizei>(fboHeight));
	COMPV_glBufferData(GL_ARRAY_BUFFER, sizeof(CompVGLFreeTypeBox) * numChars, boxesPtr, GL_STATIC_DRAW);
	COMPV_glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(numChars) * 6);

	//uint64_t timeEnd = CompVTime::nowMillis();
	//COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASS_NAME, "fillAtlas Elapsed time = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

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

COMPV_ERROR_CODE CompVGLDrawTexts::fillAtlas(const bool bUtf8, const CompVStringVector& texts, const CompVPointFloat32Vector& positions, CompVMatPtr& ptrAtlas, CompVMatPtr& ptrBoxes, size_t& numChars)
{
	// Internal function, do not check input parameters
	CompVStringVector::const_iterator it_texts = texts.begin();
	CompVPointFloat32Vector::const_iterator it_positions;
	uint8_t *atlas_buffer, *bitmap_buffer;

	GLuint ai, bi, xi, yi;
	GLfloat x2, y2, w, h, sx2, sy2, sw, sh;
	FT_GlyphSlot g = m_face->glyph;
	FT_Error ft_err;
	FT_UInt ft_chridx;

	std::vector<int> utf32;
	std::vector<int>::const_iterator it_utf32;
	bool have_utf32;

	std::string::const_iterator it_char;

	size_t count;

	const GLuint fboWidth = static_cast<GLuint>(ptrAtlas->cols());
	const GLuint fboHeight = static_cast<GLuint>(ptrAtlas->rows());

	const GLfloat sx = (1.f / (fboWidth));
	const GLfloat sy = (1.f / (fboHeight));

	numChars = 0;
	const size_t maxBoxes = ptrBoxes->cols();
	CompVGLFreeTypeBox* ptrBox = ptrBoxes->ptr<CompVGLFreeTypeBox>();

	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Uncomment all COMPV_DEBUG_WARN_EX");

	// Loop through the strings
	for (it_texts = texts.begin(), it_positions = positions.begin(); it_texts < texts.end(); ++it_texts, ++it_positions) {
		if (it_positions->x < 0.f || it_positions->y < 0.f || (xi = COMPV_MATH_ROUNDFU_2_NEAREST_INT(it_positions->x, GLuint)) >= fboWidth || (yi = COMPV_MATH_ROUNDFU_2_NEAREST_INT(it_positions->y, GLuint)) >= fboHeight) { // cast to unsigned, this is why comparison against 0 must be done before																																																		   //COMPV_DEBUG_WARN_EX(COMPV_THIS_CLASS_NAME, "Trying to write outside the screen domain (start): skip");
			continue;
		}
		
		// Check whether the string can be converted to utf32 (when utf8 encoded only)
		// Please note that you also need a font supporting unicode (e.g. 'arialuni.ttf')
		// A way to declare utf8 string: const string myString = u8"Déclarer une chaine en Français";
		have_utf32 = (bUtf8 && utf8::is_valid(it_texts->begin(), it_texts->end()));
		if (have_utf32) {
			utf8::utf8to32(it_texts->begin(), it_texts->end(), std::back_inserter(utf32));
			it_utf32 = utf32.begin();
			count = utf32.size();
		}
		else {
			it_char = it_texts->begin();
			count = it_texts->size();
		}

		// Loop through the characters
		for (size_t i = 0; i < count; ++i) {
			ft_chridx = FT_Get_Char_Index(m_face, have_utf32 ? *it_utf32++ : *it_char++);
			if ((ft_err = FT_Load_Glyph(m_face, ft_chridx, FT_LOAD_RENDER))) {
				COMPV_DEBUG_WARN_EX(COMPV_THIS_CLASS_NAME, "FT_Load_Char(face, %s) failed with error code %d", (*it_texts).c_str(), ft_err);
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
				bitmap_buffer += g->bitmap.pitch;
				atlas_buffer += fboWidth;
			}

			x2 = static_cast<GLfloat>(xi + g->bitmap_left);
			y2 = static_cast<GLfloat>(yi - g->bitmap_top);
			w = static_cast<GLfloat>(g->bitmap.width);
			h = static_cast<GLfloat>(g->bitmap.rows);

			sx2 = xi * sx;
			sy2 = yi * sy;
			sw = w * sx;
			sh = h * sy;

			(*ptrBox)[0] = x2, (*ptrBox)[1] = y2, (*ptrBox)[2] = sx2, (*ptrBox)[3] = sy2;
			(*ptrBox)[4] = x2, (*ptrBox)[5] = y2 + h, (*ptrBox)[6] = sx2, (*ptrBox)[7] = sy2 + sh;
			(*ptrBox)[8] = x2 + w, (*ptrBox)[9] = y2 + h, (*ptrBox)[10] = sx2 + sw, (*ptrBox)[11] = sy2 + sh;

			(*ptrBox)[12] = x2, (*ptrBox)[13] = y2, (*ptrBox)[14] = sx2, (*ptrBox)[15] = sy2;
			(*ptrBox)[16] = x2 + w, (*ptrBox)[17] = y2 + h, (*ptrBox)[18] = sx2 + sw, (*ptrBox)[19] = sy2 + sh;
			(*ptrBox)[20] = x2 + w, (*ptrBox)[21] = y2, (*ptrBox)[22] = sx2 + sw, (*ptrBox)[23] = sy2;
			++ptrBox;

			if (++numChars >= maxBoxes) {
				COMPV_DEBUG_WARN_EX(COMPV_THIS_CLASS_NAME, "Breaking FreeType processing because we reached the maximum bitmaps per process (%zu)", numChars);
				return COMPV_ERROR_CODE_S_OK; // trucation but do not exit
			}

			xi += static_cast<GLuint>(g->advance.x >> 6);
			yi += static_cast<GLuint>(g->advance.y >> 6);
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

