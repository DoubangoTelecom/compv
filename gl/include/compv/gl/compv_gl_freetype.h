/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
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
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#undef HAVE_FREETYPE
#define HAVE_FREETYPE 1

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class CompVGLFreeType
{
public:
	static COMPV_ERROR_CODE init();
	static COMPV_ERROR_CODE deInit();
	static FT_Library library();

private:
	static bool s_bInitialized;
	static FT_Library s_library;
};

COMPV_NAMESPACE_END()

#endif /* (defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)) && (HAVE_FREETYPE_FREETYPE_H || HAVE_FREETYPE_H || HAVE_FREETYPE) */

#endif /* _COMPV_GL_FREETYPE_H_ */
