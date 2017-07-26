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

COMPV_ERROR_CODE CompVGLFreeType::init()
{
	if (CompVGLFreeType::s_bInitialized) {
		return COMPV_ERROR_CODE_S_OK;
	}

	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;

	FT_Error ft_err;
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
	if (CompVGLFreeType::s_library) {
		COMPV_CHECK_EXP_ASSERT(FT_Done_FreeType(CompVGLFreeType::s_library) != 0, COMPV_ERROR_CODE_E_FREETYPE);
		CompVGLFreeType::s_library = nullptr;
	}

	CompVGLFreeType::s_bInitialized = false;

	return COMPV_ERROR_CODE_S_OK;
}

FT_Library CompVGLFreeType::library()
{
	return CompVGLFreeType::s_library;
}

COMPV_NAMESPACE_END()

#endif /* (defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)) && (HAVE_FREETYPE_FREETYPE_H || HAVE_FREETYPE_H || HAVE_FREETYPE) */

