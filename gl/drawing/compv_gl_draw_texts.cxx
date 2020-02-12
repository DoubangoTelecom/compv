/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/gl/drawing/compv_gl_draw_texts.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/base/compv_mat.h"
#include "compv/base/compv_fileutils.h"
#include "compv/base/utf8/utf8.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/time/compv_time.h"
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

#define COMPV_FONT_SIZE_DEFAULT	16
#define COMPV_FONT_PATH_DEFAULT	"arial.ttf"

typedef GLfloat CompVGLFreeTypeBox[4 * 6]; // (fbo_x, fbo_y, tex_x, tex_y) * 6

#define COMPV_THIS_CLASS_NAME "CompVGLDrawTexts"

// FreeType tutos:
//	- https://www.freetype.org/freetype2/docs/tutorial/step1.html
//	- https://learnopengl.com/#!In-Practice/Text-Rendering
//  - Implementation based on https://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_Text_Rendering_01

COMPV_NAMESPACE_BEGIN()

CompVGLDrawTexts::CompVGLDrawTexts()
	: CompVGLDraw(kProgramVertexData, kProgramFragmentData, kVertexDataWithMVP_Yes)
#if HAVE_FREETYPE
	, m_fboWidth(0)
	, m_fboHeight(0)
	, m_uTextureAtlas(kCompVGLNameInvalid)
	, m_fontSize(COMPV_FONT_SIZE_DEFAULT)
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
	if (m_uTextureAtlas != kCompVGLNameInvalid) {
		if (CompVGLUtils::currentContext()) {
			COMPV_CHECK_CODE_NOP(CompVGLUtils::textureDelete(&m_uTextureAtlas));
		}
		else {
			COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASS_NAME, "No OpenGL context: %d texture will leak", m_uTextureAtlas);
		}
	}
#endif
}

COMPV_ERROR_CODE CompVGLDrawTexts::texts(const CompVVecString& texts, const CompVPointFloat32Vector& positions, const CompVDrawingOptions* options COMPV_DEFAULT(nullptr))
{
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
#if HAVE_FREETYPE
	const bool randomColors = (!options || options->colorType == COMPV_DRAWING_COLOR_TYPE_RANDOM);
	const bool utf8 = (options && options->fontUtf8);
	const std::string fontFullPath = (options ? options->fontFullPath : "");
	const size_t fontSize = (options ? options->fontSize : COMPV_FONT_SIZE_DEFAULT);

	CompVMatPtr ptrAtlas, ptrBoxes;

	size_t numChars, maxChars;

	// Create face if not already done
	COMPV_CHECK_CODE_RETURN(freeTypeCreateFace(fontFullPath, fontSize));

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
				COMPV_CHECK_CODE_BAIL(err = CompVGLUtils::textureGen(&m_uTextureAtlas));
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
		COMPV_glUniform4f(uniform_color, r, g, b, a);

		// Set texture attribute
		GLuint uniform_tex = COMPV_glGetUniformLocation(program()->name(), "tex");
		COMPV_glUniform1i(uniform_tex, 0); // GL_TEXTURE0

		// Set projection
		COMPV_CHECK_CODE_BAIL(err = CompVGLDraw::setOrtho(0, static_cast<GLfloat>(fboWidth), static_cast<GLfloat>(fboHeight), 0, -1, 1));
	}
	else if (randomColors) {
		GLuint uniform_color = COMPV_glGetUniformLocation(program()->name(), "color");
		const GLfloat(*c)[3] = &kCompVGLRandomColors[rand() % kCompVGLRandomColorsCount];
		COMPV_glUniform4f(uniform_color, (*c)[0], (*c)[1], (*c)[2], 1.f);
	}
	
	// Get maximum chars
	maxChars = ((fboWidth * fboHeight) /*/ fontSize*/); // do not divide by fontSize to allow overwritting and give some room

	// Get number of chars
	numChars = 0;
	for (CompVVecString::const_iterator it_texts = texts.begin(); it_texts < texts.end(); ++it_texts) {
		numChars += it_texts->size();
	}
	if (numChars > maxChars) {
		COMPV_DEBUG_WARN_EX(COMPV_THIS_CLASS_NAME, "Too much characters to write, truncating (%zu > %zu)", numChars, maxChars);
		numChars = maxChars;
	}

	// Create boxes' holder
	COMPV_CHECK_CODE_BAIL(err = (CompVMat::newObj<CompVGLFreeTypeBox, COMPV_MAT_TYPE_STRUCT>(&ptrBoxes, 1, (numChars + 1), 1, (numChars + 1))));

	// Build atlas: Filling the atlas not thread-safe because of the shared charcode cache. If we
	// remove the cache to make tha code thread-safe then we'll loose.
	COMPV_glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	COMPV_glActiveTexture(GL_TEXTURE0);
	COMPV_glBindTexture(GL_TEXTURE_2D, m_uTextureAtlas);
	COMPV_CHECK_CODE_BAIL(err = (CompVMat::newObj<uint8_t>(&ptrAtlas, fboHeight, fboWidth, 1, fboWidth)));
	COMPV_CHECK_CODE_BAIL(err = freeTypeFillAtlas(utf8, texts, positions, ptrAtlas, ptrBoxes, numChars));

	// Submit atlas data to texture
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

	// Draw chars (OpenGL/GPU process)
	COMPV_glViewport(0, 0, static_cast<GLsizei>(fboWidth), static_cast<GLsizei>(fboHeight));
	COMPV_glBufferData(GL_ARRAY_BUFFER, sizeof(CompVGLFreeTypeBox) * numChars, ptrBoxes->ptr(), GL_STATIC_DRAW);
	COMPV_glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(numChars) * 6);

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

COMPV_ERROR_CODE CompVGLDrawTexts::freeTypeAddChar(unsigned long charcode)
{
	FT_Error ft_err;
	FT_UInt ft_chridx;
	FT_GlyphSlot ft_g = m_face->glyph;

	ft_chridx = FT_Get_Char_Index(m_face, charcode);
	if ((ft_err = FT_Load_Glyph(m_face, ft_chridx, FT_LOAD_RENDER))) {
		COMPV_DEBUG_WARN_EX(COMPV_THIS_CLASS_NAME, "FT_Load_Char(face, %d) failed with error code %d (%s)", ft_chridx, ft_err, CompVGLFreeType::errorMessage(ft_err));
		return COMPV_ERROR_CODE_E_FREETYPE;
	}

	COMPV_DEBUG_VERBOSE_EX(COMPV_THIS_CLASS_NAME, "Adding new charcode to the cache: %ld", charcode);
	void* memory = CompVMem::malloc(ft_g->bitmap.rows * ft_g->bitmap.width);
	COMPV_CHECK_EXP_RETURN(!memory && ft_g->bitmap.rows && ft_g->bitmap.width, COMPV_ERROR_CODE_E_OUT_OF_MEMORY); // charcode 32 will have zero memory allocated
	CompVFreeTypeChar& fchar = m_freeTypeCache[charcode] = CompVFreeTypeChar();
	fchar.mem = reinterpret_cast<uint8_t*>(memory);
	fchar.left = ft_g->bitmap_left;
	fchar.top = ft_g->bitmap_top;
	fchar.width = ft_g->bitmap.width;
	fchar.height = ft_g->bitmap.rows;
	fchar.advance_x = static_cast<GLuint>(ft_g->advance.x >> 6);
	fchar.advance_y = static_cast<GLuint>(ft_g->advance.y >> 6);

	uint8_t* fchar_buffer = fchar.mem;
	const uint8_t* bitmap_buffer = ft_g->bitmap.buffer;
	for (GLuint bi = 0; bi < fchar.height; ++bi) {
		for (GLuint ai = 0; ai < fchar.width; ++ai) {
			fchar_buffer[ai] = bitmap_buffer[ai];
		}
		bitmap_buffer += ft_g->bitmap.pitch;
		fchar_buffer += fchar.width;
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLDrawTexts::freeTypeCreateFace(const std::string fontFullPath, size_t fontSize)
{
	COMPV_CHECK_EXP_RETURN(!fontSize, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Font size must be > 0");
	COMPV_CHECK_EXP_RETURN(!CompVGLFreeType::library(), COMPV_ERROR_CODE_E_INVALID_STATE, "FreeType library not loaded");
	FT_Error ft_err;
	if (m_face && (fontFullPath.empty() || m_fontFullPath == fontFullPath)) {
		if (fontSize != m_fontSize) {
			COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASS_NAME, "Font size changed: %zu -> %zu.", m_fontSize, fontSize);
			if ((ft_err = FT_Set_Pixel_Sizes(m_face, 0, static_cast<FT_UInt>(fontSize)))) {
				COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASS_NAME, "FT_Set_Pixel_Sizes(face, 0, 16) failed with error '%s'", CompVGLFreeType::errorMessage(ft_err));
				return COMPV_ERROR_CODE_E_FREETYPE;
			}
			m_freeTypeCache.clear();
			m_fontSize = fontSize;
		}
		return COMPV_ERROR_CODE_S_OK;
	}

	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASS_NAME, "Creating new face for the first time or because the font changed (%s -> %s)", m_fontFullPath.c_str(), fontFullPath.c_str());

	// Delete previous face
	if (m_face) {
		if (CompVGLFreeType::library()) {
			COMPV_CHECK_EXP_RETURN(FT_Done_Face(m_face) != 0, COMPV_ERROR_CODE_E_FREETYPE);
		}
		m_face = nullptr;
		m_freeTypeCache.clear();
	}

	std::string fontPath_ = fontFullPath;

	// Try to guess the full path to the font if not provided
	if (fontPath_.empty()) {
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASS_NAME, "No font path provided, trying to guess...");
#if COMPV_OS_WINDOWS
		CHAR dir[MAX_PATH + 1];
		COMPV_CHECK_EXP_RETURN(GetWindowsDirectoryA(dir, MAX_PATH) == 0, COMPV_ERROR_CODE_E_FILE_NOT_FOUND, "GetWindowsDirectoryA faited");
		fontPath_ = dir + std::string("/Fonts/") + std::string(COMPV_FONT_PATH_DEFAULT);
#else
		fontPath_ = COMPV_PATH_FROM_NAME(COMPV_FONT_PATH_DEFAULT); // Android or iOS retrieve full path from assets/bundle	
#endif /* COMPV_OS_WINDOWS */
	}

	// Make sure fontPath_ not empty
	COMPV_CHECK_EXP_RETURN(fontPath_.empty(), COMPV_ERROR_CODE_E_FILE_NOT_FOUND, "No font path provided and we failed to guess one");

	// If the font path doesn't exist then, try to build fullpath (maybe the user provided a relative one)
	if (!CompVFileUtils::exists(fontPath_.c_str())) {
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASS_NAME, "Font path (%s) doesn't exist, trying to get full path...", fontPath_.c_str());
		fontPath_ = COMPV_PATH_FROM_NAME(fontPath_.c_str());
	}

	// Make sure now we have a valid full path
	if (!CompVFileUtils::exists(fontPath_.c_str())) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASS_NAME, "Font path provided (or guessed) but doesn't exist: %s", fontPath_.c_str());
		return COMPV_ERROR_CODE_E_FILE_NOT_FOUND;
	}
	
	// Read file into buffer, we don't provide the full path to FreeType because it'll fail to read data from
	// assets (Android) or bundle (iOS)
	COMPV_CHECK_CODE_RETURN(CompVFileUtils::read(fontPath_.c_str(), &m_ptrFaceBuffer), "Failed to read font file");
	if (m_ptrFaceBuffer->size() > (1024 * 1024)) {
		COMPV_DEBUG_WARN_EX(COMPV_THIS_CLASS_NAME, "Font (%s) loaded but memory usage is very high (%zuMo). You should consider using another font", fontPath_.c_str(), m_ptrFaceBuffer->size()>>20);
	}

	// Create the face from memory
	// Memory must not be deallocated until we're done with the face: https://www.freetype.org/freetype2/docs/reference/ft2-base_interface.html#FT_New_Memory_Face
	if ((ft_err = FT_New_Memory_Face(CompVGLFreeType::library(), reinterpret_cast<const FT_Byte*>(m_ptrFaceBuffer->ptr()), static_cast<FT_Long>(m_ptrFaceBuffer->size()), 0, &m_face))) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASS_NAME, "FT_New_Face(%s, 0) failed with error '%s'", fontPath_.c_str(), CompVGLFreeType::errorMessage(ft_err));
		return COMPV_ERROR_CODE_E_FREETYPE;
	}
	if ((ft_err = FT_Set_Pixel_Sizes(m_face, 0, static_cast<FT_UInt>(fontSize)))) {
		COMPV_CHECK_EXP_NOP(FT_Done_Face(m_face) == 0, COMPV_ERROR_CODE_E_FREETYPE);
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASS_NAME, "FT_Set_Pixel_Sizes(face, 0, %zu) failed with error '%s'", fontSize, CompVGLFreeType::errorMessage(ft_err));
		return COMPV_ERROR_CODE_E_FREETYPE;
	}
	if ((ft_err = FT_Select_Charmap(m_face, ft_encoding_unicode))) {
		COMPV_DEBUG_WARN_EX(COMPV_THIS_CLASS_NAME, "FT_Select_Charmap(%s, ft_encoding_unicode) failed with error '%s'", fontPath_.c_str(), CompVGLFreeType::errorMessage(ft_err));
	}

	m_fontFullPath = fontFullPath; // not fontPath_ because we want to make sure comparing it with the provided parameter value will be ok and face not created several times
	m_fontSize = fontSize;

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLDrawTexts::freeTypeFillAtlas(const bool bUtf8, const CompVVecString& texts, const CompVPointFloat32Vector& positions, CompVMatPtr& ptrAtlas, CompVMatPtr& ptrBoxes, size_t& numChars)
{
	// Internal function, do not check input parameters
	CompVVecString::const_iterator it_texts;
	CompVPointFloat32Vector::const_iterator it_positions;
	uint8_t *atlas_buffer, *bitmap_buffer;

	GLuint ai, bi, xi, yi;
	GLfloat x2, y2, w, h, sx2, sy2, sw, sh;

	COMPV_ERROR_CODE err;
	
	FT_ULong ft_charcode;

	std::vector<int> utf32;
	std::vector<int>::const_iterator it_utf32;
	bool have_utf32;

	std::string::const_iterator it_char;

	CompVFreeTypeCache::const_iterator it_cache;
	const CompVFreeTypeChar* cache_char;

	size_t count;

	const GLuint fboWidth = static_cast<GLuint>(ptrAtlas->cols());
	const GLuint fboHeight = static_cast<GLuint>(ptrAtlas->rows());

	const GLfloat sx = (1.f / (fboWidth));
	const GLfloat sy = (1.f / (fboHeight));

	numChars = 0;
	const size_t maxBoxes = ptrBoxes->cols();
	CompVGLFreeTypeBox* ptrBox = ptrBoxes->ptr<CompVGLFreeTypeBox>();

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
			utf32.clear();
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
			// Get next charcode from the current string
			ft_charcode = have_utf32 ? static_cast<FT_ULong>(*it_utf32++) : static_cast<FT_ULong>(*it_char++);

			// Find the charcode from the cache
			it_cache = m_freeTypeCache.find(ft_charcode);
			if (it_cache == m_freeTypeCache.end()) {
				err = freeTypeAddChar(ft_charcode);
				if (err == COMPV_ERROR_CODE_E_FREETYPE) {
					continue; // do not stop the process if we fail because of FreeType failing to load a charcode (FT_Load_Glyph)
				}
				COMPV_CHECK_CODE_RETURN(err);
				it_cache = m_freeTypeCache.find(ft_charcode);
				COMPV_CHECK_EXP_RETURN((it_cache == m_freeTypeCache.end()), COMPV_ERROR_CODE_E_SYSTEM, "Hum, this must never happen");
			}
			cache_char = &it_cache->second;

			// Do not write partial chars
			if ((xi + cache_char->width) >= fboWidth || (yi + cache_char->height) >= fboHeight) {
				COMPV_DEBUG_VERBOSE_EX(COMPV_THIS_CLASS_NAME, "Trying to write outside the screen domain (partial char): skip");
				break; // end the string
			}

			// Write bitmap to buffer
			bitmap_buffer = cache_char->mem;
			atlas_buffer = ptrAtlas->ptr<uint8_t>(yi, xi);
			for (bi = 0; bi < cache_char->height; ++bi) {
				for (ai = 0; ai < cache_char->width; ++ai) {
					atlas_buffer[ai] = bitmap_buffer[ai];
				}
				bitmap_buffer += cache_char->width;
				atlas_buffer += fboWidth;
			}

			// Screen indices
			x2 = static_cast<GLfloat>(xi + cache_char->left);
			y2 = static_cast<GLfloat>(yi - cache_char->top);
			w = static_cast<GLfloat>(cache_char->width);
			h = static_cast<GLfloat>(cache_char->height);

			// Scaling factors (screen -> gl.uv[0-1])
			sx2 = xi * sx;
			sy2 = yi * sy;
			sw = w * sx;
			sh = h * sy;

			// Indices (6x4) [scren.x, screen.y, gl.u, gl.v]
			(*ptrBox)[0] = x2, (*ptrBox)[1] = y2, (*ptrBox)[2] = sx2, (*ptrBox)[3] = sy2;
			(*ptrBox)[4] = x2, (*ptrBox)[5] = y2 + h, (*ptrBox)[6] = sx2, (*ptrBox)[7] = sy2 + sh;
			(*ptrBox)[8] = x2 + w, (*ptrBox)[9] = y2 + h, (*ptrBox)[10] = sx2 + sw, (*ptrBox)[11] = sy2 + sh;

			(*ptrBox)[12] = x2, (*ptrBox)[13] = y2, (*ptrBox)[14] = sx2, (*ptrBox)[15] = sy2;
			(*ptrBox)[16] = x2 + w, (*ptrBox)[17] = y2 + h, (*ptrBox)[18] = sx2 + sw, (*ptrBox)[19] = sy2 + sh;
			(*ptrBox)[20] = x2 + w, (*ptrBox)[21] = y2, (*ptrBox)[22] = sx2 + sw, (*ptrBox)[23] = sy2;
			++ptrBox;

			// Make sure we haven't reached our capacity
			if (++numChars >= maxBoxes) {
				COMPV_DEBUG_WARN_EX(COMPV_THIS_CLASS_NAME, "Breaking FreeType processing because we reached the maximum bitmaps per process (%zu)", numChars);
				return COMPV_ERROR_CODE_S_OK; // trucation but do not exit
			}

			// Move cursor
			xi += cache_char->advance_x;
			yi += cache_char->advance_y;
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

