/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/gl/compv_gl_freetype.h"
#if (defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)) && (HAVE_FREETYPE_FREETYPE_H || HAVE_FREETYPE_H || HAVE_FREETYPE)
#include "compv/gl/compv_gl.h"
#include "compv/gl/compv_gl_utils.h"
#include "compv/gl/compv_gl_common.h"
#include "compv/gl/compv_gl_func.h"

#define COMPV_THIS_CLASSNAME	"CompVGLFreeType"

COMPV_NAMESPACE_BEGIN()

bool CompVGLFreeType::s_bInitialized = false;
FT_Library CompVGLFreeType::s_library = nullptr;
CompVGLFreeTypeCharacterStyleMap CompVGLFreeType::s_characters;
CompVMutexPtr CompVGLFreeType::s_mutex = nullptr;

COMPV_ERROR_CODE CompVGLFreeType::init()
{
	if (CompVGLFreeType::s_bInitialized) {
		return COMPV_ERROR_CODE_S_OK;
	}

	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	FT_Error ft_err;

	COMPV_CHECK_CODE_BAIL(err = CompVMutex::newObj(&CompVGLFreeType::s_mutex));
	
	if ((ft_err = FT_Init_FreeType(&CompVGLFreeType::s_library))) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "FT_Init_FreeType failed with error code %d", ft_err);
		COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_E_FREETYPE);
	}
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "FreeType implementation enabled");

	CompVGLFreeType::s_bInitialized = true;

bail:
	if (COMPV_ERROR_CODE_IS_NOK(err)) {
		COMPV_CHECK_CODE_NOP(CompVGL::deInit());
	}

	return err;
}

COMPV_ERROR_CODE CompVGLFreeType::deInit()
{
	COMPV_CHECK_EXP_ASSERT(!CompVGLFreeType::s_characters.empty(), COMPV_ERROR_CODE_E_MEMORY_LEAK, "FreeType characters not freeded"); // requires GL-context

	if (CompVGLFreeType::s_library) {
		COMPV_CHECK_EXP_ASSERT(FT_Done_FreeType(CompVGLFreeType::s_library) != 0, COMPV_ERROR_CODE_E_FREETYPE);
		CompVGLFreeType::s_library = nullptr;
	}

	CompVGLFreeType::s_mutex = nullptr;

	CompVGLFreeType::s_bInitialized = false;

	return COMPV_ERROR_CODE_S_OK;
}

FT_Library CompVGLFreeType::library()
{
	return CompVGLFreeType::s_library;
}

COMPV_ERROR_CODE CompVGLFreeType::style(const std::string& fontName, size_t fontSize, CompVGLFreeTypeStyle& style)
{
	COMPV_CHECK_EXP_RETURN(fontName.empty() || !fontSize, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_EXP_RETURN(!CompVGLUtils::currentContext(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT, "No OpenGL context");

	COMPV_CHECK_CODE_RETURN(s_mutex->lock());

	uintptr_t gl_context = reinterpret_cast<uintptr_t>(CompVGLUtils::currentContext());
	CompVGLFreeTypeCharacterStyleMap::iterator it = s_characters.find(gl_context);
	if (it == s_characters.end()) {
		s_characters[gl_context] = style = CompVGLFreeTypeStyle(fontName, fontSize);
	}
	else {
		style = it->second;
	}

	COMPV_CHECK_CODE_NOP(s_mutex->unlock());
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLFreeType::character(const GLchar inChar, const CompVGLFreeTypeStyle* style, CompVGLFreeTypeCharacter& outChar)
{
	COMPV_CHECK_EXP_RETURN(!style, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_EXP_RETURN(!CompVGLUtils::currentContext(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT, "No OpenGL context");
	
	COMPV_CHECK_CODE_RETURN(s_mutex->lock());

	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	FT_Error ft_err;

	// Find character in the builtin map
	CompVGLFreeTypeCharacterMap::const_iterator it = style->characters.find(inChar);
	if (it != style->characters.end()) {
		outChar = it->second;
	}

	// At this step, it means we've found the character in the builtin map -> build it
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Adding FreeType character (%c) for the current context %p and style(%s, %zu)", 
		inChar, CompVGLUtils::currentContext(), style->fontName.c_str(), style->fontSize);

	CompVGLFreeTypeStyle* style_ = const_cast<CompVGLFreeTypeStyle*>(style);
	
	// Create face for the style if not already done
	if (!style_->face) {
		if ((ft_err = FT_New_Face(CompVGLFreeType::library(), style_->fontName.c_str(), 0, &style_->face))) {
			COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "FT_New_Face('FreeSans.ttf', 0) failed with error '%s'", CompVGLFreeType::errorMessage(ft_err));
			COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_E_FREETYPE);
		}
		if ((ft_err = FT_Set_Pixel_Sizes(style_->face, 0, static_cast<FT_UInt>(style_->fontSize)))) {
			COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "FT_Set_Pixel_Sizes(face, 0, 16) failed with error '%s'", CompVGLFreeType::errorMessage(ft_err));
			COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_E_FREETYPE);
		}
	}
	FT_Face face = style_->face;

	if ((ft_err = FT_Load_Char(style_->face, inChar, FT_LOAD_RENDER))) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "FT_Load_Char('%c') failed with error '%s'", inChar, CompVGLFreeType::errorMessage(ft_err));
		COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_E_FREETYPE);
	}
	
	// Disable byte-alignment restriction
	COMPV_glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Generate texture
	COMPV_CHECK_CODE_BAIL(err = CompVGLUtils::textureGen(&outChar.nameTexture));
	COMPV_glBindTexture(GL_TEXTURE_2D, outChar.nameTexture);
	COMPV_glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RED,
		face->glyph->bitmap.width,
		face->glyph->bitmap.rows,
		0,
		GL_RED,
		GL_UNSIGNED_BYTE,
		face->glyph->bitmap.buffer
	);
	// Set texture options
	COMPV_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	COMPV_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	COMPV_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	COMPV_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Now store character for later use
	outChar.size[0] = face->glyph->bitmap.width;
	outChar.size[1] = face->glyph->bitmap.rows;
	outChar.bearing[0] = face->glyph->bitmap_left;
	outChar.bearing[1] = face->glyph->bitmap_top;
	outChar.advance = face->glyph->advance.x;
	// Add new char
	style_->characters.insert(std::pair<GLchar, CompVGLFreeTypeCharacter>(inChar, outChar));

bail:
	COMPV_CHECK_CODE_NOP(s_mutex->unlock());
	return err;
}

const char* CompVGLFreeType::errorMessage(FT_Error err)
{
#undef __FTERRORS_H__
#define FT_ERRORDEF( e, v, s )  case e: return s;
#define FT_ERROR_START_LIST     switch (err) {
#define FT_ERROR_END_LIST       }
#include FT_ERRORS_H
	return "(Unknown error)";
}

COMPV_NAMESPACE_END()

#endif /* (defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)) && (HAVE_FREETYPE_FREETYPE_H || HAVE_FREETYPE_H || HAVE_FREETYPE) */

