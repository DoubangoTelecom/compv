/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/

#include "compv/gl/compv_gl.h"
#include "compv/gl/compv_gl_headers.h"
#include "compv/base/compv_debug.h"
#include "compv/base/compv_base.h"

COMPV_NAMESPACE_BEGIN()

bool CompVGL::s_bInitialized = false;

COMPV_ERROR_CODE CompVGL::init()
{
	if (CompVGL::s_bInitialized) {
		return COMPV_ERROR_CODE_S_OK;
	}

	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;

	COMPV_DEBUG_INFO("Initializing [gl] module (v %s)...", COMPV_VERSION_STRING);

	COMPV_CHECK_CODE_BAIL(err = CompVBase::init());

#if defined(HAVE_GL_GLEW_H)
	COMPV_DEBUG_INFO("GLEW version being used: %d.%d.%d", GLEW_VERSION_MAJOR, GLEW_VERSION_MINOR, GLEW_VERSION_MICRO);
#endif /* HAVE_GL_GLEW_H */

#if	defined(HAVE_OPENGLES)
	COMPV_DEBUG_INFO("OpenGL-ES implementation enabled");
#elif defined(HAVE_OPENGL)
	COMPV_DEBUG_INFO("OpenGL implementation enabled");
#endif

	CompVGL::s_bInitialized = true;

bail:
	if (COMPV_ERROR_CODE_IS_NOK(err)) {
		COMPV_CHECK_CODE_ASSERT(CompVGL::deInit());
	}
	return err;
}
	
COMPV_ERROR_CODE CompVGL::deInit()
{
	COMPV_CHECK_CODE_ASSERT(CompVBase::deInit());

	CompVGL::s_bInitialized = false;

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

