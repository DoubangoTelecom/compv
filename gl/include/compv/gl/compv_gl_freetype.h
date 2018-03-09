/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_GL_FREETYPE_H_)
#define _COMPV_GL_FREETYPE_H_

#include "compv/gl/compv_gl_config.h"
#include "compv/gl/compv_gl_headers.h"
#if (defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)) && (HAVE_FREETYPE_FREETYPE_H || HAVE_FREETYPE_H || HAVE_FREETYPE)
#include "compv/gl/compv_gl_common.h"
#include "compv/base/parallel/compv_mutex.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#undef HAVE_FREETYPE
#define HAVE_FREETYPE 1

#include <map>

#if defined(_ULTIMATE_BASE_LIB_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

struct CompVGLFreeTypeCharacter {
	GLuint nameTexture = kCompVGLNameInvalid;
	GLint size[2];
	GLint bearing[2];
	GLuint advance[2];
};
typedef std::map<GLchar, CompVGLFreeTypeCharacter> CompVGLFreeTypeCharacterMap;

struct CompVGLFreeTypeStyle {
	FT_Face face = nullptr;
	std::string fontName = "";
	size_t fontSize;
	CompVGLFreeTypeStyle(std::string fontName_ = "", size_t fontSize_ = 0) : fontName(fontName_), fontSize(fontSize_) { }
	CompVGLFreeTypeCharacterMap characters;
public:
	virtual ~CompVGLFreeTypeStyle() {
		COMPV_CHECK_EXP_ASSERT(!characters.empty(), COMPV_ERROR_CODE_E_MEMORY_LEAK, "FreeType characters not freeded"); // requires GL-context
		if (face) {
			COMPV_CHECK_EXP_NOP(FT_Done_Face(face) != 0, COMPV_ERROR_CODE_E_FREETYPE);
			face = nullptr;
		}
	}
	bool operator==(const CompVGLFreeTypeStyle &other) const {
		return fontName == other.fontName && fontSize == other.fontSize;
	}
};

typedef std::map<uintptr_t, CompVGLFreeTypeStyle> CompVGLFreeTypeCharacterStyleMap;

class CompVGLFreeType
{
public:
	static COMPV_ERROR_CODE init();
	static COMPV_ERROR_CODE deInit();
	
	static FT_Library library();

	static const char* errorMessage(FT_Error err);

	static COMPV_ERROR_CODE style(const std::string& fontName, size_t fontSize, CompVGLFreeTypeStyle*& style);
	static COMPV_ERROR_CODE character_find(const GLchar inChar, const CompVGLFreeTypeStyle* style, CompVGLFreeTypeCharacter& outChar);
	static COMPV_ERROR_CODE character_remove(const GLchar inChar, CompVGLFreeTypeStyle* style);

private:
	static bool s_bInitialized;
	static FT_Library s_library;
	static CompVGLFreeTypeCharacterStyleMap s_characters;
	static CompVMutexPtr s_mutex;
};

COMPV_NAMESPACE_END()

#endif /* (defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)) && (HAVE_FREETYPE_FREETYPE_H || HAVE_FREETYPE_H || HAVE_FREETYPE) */

#endif /* _COMPV_GL_FREETYPE_H_ */
