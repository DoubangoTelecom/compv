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
"		gl_FragColor = vec4(1.0, 1.0, 1.0, texture2D(tex, texcoord).r);"
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

#if 0
	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Move creting glMemTexts to canvas like what is done from drawPoints and drawLines");
	CompVMatPtr glMemTexts;
	COMPV_CHECK_CODE_RETURN((CompVMat::newObj<CompVGLText2D, COMPV_MAT_TYPE_STRUCT>(&glMemTexts, 1, texts.size(), 1)));
	CompVPointFloat32Vector::const_iterator it;
	bool randomColors = (!options || options->colorType == COMPV_DRAWING_COLOR_TYPE_RANDOM);
	const GLfloat(*color)[3];
	if (randomColors) {
		for (it = positions.begin(), glMemPoint_ = glMemPoints->ptr<CompVGLPoint2D>(); it != positions.end(); ++it, ++glMemPoint_) {
			// x, y
			glMemPoint_->position[0] = static_cast<GLfloat>((*it).x);
			glMemPoint_->position[1] = static_cast<GLfloat>((*it).y);
			// r, g, b
			color = &kCompVGLRandomColors[rand() % kCompVGLRandomColorsCount];
			glMemPoint_->color[0] = (*color)[0];
			glMemPoint_->color[1] = (*color)[1];
			glMemPoint_->color[2] = (*color)[2];
			glMemPoint_->color[3] = 1.f; // alpha
		}
	}
	else {
		for (it = positions.begin(), glMemPoint_ = glMemPoints->ptr<CompVGLPoint2D>(); it != positions.end(); ++it, ++glMemPoint_) {
			// x, y
			glMemPoint_->position[0] = static_cast<GLfloat>((*it).x);
			glMemPoint_->position[1] = static_cast<GLfloat>((*it).y);
			// r, g, b
			glMemPoint_->color[0] = options->color[0];
			glMemPoint_->color[1] = options->color[1];
			glMemPoint_->color[2] = options->color[2];
			glMemPoint_->color[3] = options->color[3];
		}
	}
#endif

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
		// Set position attribute
		GLuint attribute_coord = COMPV_glGetAttribLocation(program()->name(), "coord");
		COMPV_glEnableVertexAttribArray(attribute_coord);
		COMPV_glVertexAttribPointer(attribute_coord, 4, GL_FLOAT, GL_FALSE, 0, 0);

		// Set color attribute
		//GLuint uNameColor = COMPV_glGetAttribLocation(program()->name(), "color");
		//COMPV_glEnableVertexAttribArray(uNameColor);
		//COMPV_glVertexAttribPointer(uNameColor, 4, GL_FLOAT, GL_FALSE, sizeof(CompVGLPoint2D), reinterpret_cast<const GLvoid *>(offsetof(CompVGLPoint2D, color)));

		// Set projection
		COMPV_CHECK_CODE_BAIL(err = CompVGLDraw::setOrtho(0, static_cast<GLfloat>(fboWidth), static_cast<GLfloat>(fboHeight), 0, -1, 1));
	}

	// Set viewport
	COMPV_glViewport(0, 0, static_cast<GLsizei>(fboWidth), static_cast<GLsizei>(fboHeight));

	const char *p;
	const char *text = "Mamadou DIOP (Français)";
	float sx = 2.f / static_cast<float>(fboWidth);
	float sy = 2.f / static_cast<float>(fboHeight);
	float x = 0.f;
	float y = 0.f;

	FT_GlyphSlot g = m_face->glyph;

	GLuint tex;
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	GLuint uniform_tex = COMPV_glGetUniformLocation(program()->name(), "tex");
	glUniform1i(uniform_tex, 0);

	// this affects glTexImage2D and glSubTexImage2D
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

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
		COMPV_glBufferData(GL_ARRAY_BUFFER, sizeof(box), box, GL_DYNAMIC_DRAW);

		// Draw points
		COMPV_glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		x += (g->advance.x / 64) * sx;
		y += (g->advance.y / 64) * sy;
	}

	glDeleteTextures(1, &tex);

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

