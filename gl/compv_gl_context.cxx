/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/gl/compv_gl_context.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/gl/compv_gl_utils.h"

COMPV_NAMESPACE_BEGIN()

CompVGLContext::CompVGLContext()
    : CompVObj()
    , CompVLock()
{

}

CompVGLContext::~CompVGLContext()
{

}

bool CompVGLContext::isSet()
{
    return !!CompVGLUtils::isGLContextSet();
}

COMPV_ERROR_CODE CompVGLContext::makeCurrent()
{
    COMPV_CHECK_CODE_RETURN(CompVLock::lock());
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLContext::swapBuffers()
{
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLContext::unmakeCurrent()
{
    COMPV_CHECK_CODE_RETURN(CompVLock::unlock());
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */
