/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_GL_H_)
#define _COMPV_GL_H_

#include "compv/gl/compv_gl_config.h"
#include "compv/base/compv_sharedlib.h"

#if HAVE_FREETYPE_FREETYPE_H || HAVE_FREETYPE_H
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#undef HAVE_FREETYPE
#define HAVE_FREETYPE 1
#endif /* HAVE_FREETYPE_FREETYPE_H || HAVE_FREETYPE_H */

COMPV_NAMESPACE_BEGIN()

class COMPV_GL_API CompVGL
{
public:
    static COMPV_ERROR_CODE init();
    static COMPV_ERROR_CODE deInit();
#	if defined(HAVE_GL_GLEW_H)
    static COMPV_ERROR_CODE glewInit();
#endif
#if HAVE_FREETYPE
	static FT_Library freeTypeLibrary();
#endif

private:
    static bool s_bInitialized;
#if HAVE_FREETYPE
	static FT_Library s_freetype;
#endif /* HAVE_FREETYPE */
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_GL_H_ */
