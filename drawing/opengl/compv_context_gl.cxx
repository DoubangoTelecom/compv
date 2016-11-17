/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/drawing/opengl/compv_context_gl.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)

COMPV_NAMESPACE_BEGIN()

CompVContextGL::CompVContextGL()
	: CompVObj()
	, CompVLock()
	, m_bContextSet(false)
{

}

CompVContextGL::~CompVContextGL()
{

}

COMPV_ERROR_CODE CompVContextGL::makeCurrent()
{
	COMPV_CHECK_CODE_RETURN(CompVLock::lock());
	m_bContextSet = true;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVContextGL::swabBuffers()
{
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVContextGL::unmakeCurrent()
{
	COMPV_CHECK_CODE_RETURN(CompVLock::unlock());
	m_bContextSet = false;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */
