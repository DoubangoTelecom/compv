/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_GL_VAO_H_)
#define _COMPV_GL_VAO_H_

#include "compv/gl/compv_config.h"
#include "compv/gl/compv_gl_headers.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/gl/compv_common.h"
#include "compv/base/compv_sharedlib.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class COMPV_GL_API CompVGLVAO
{
public:
	static COMPV_ERROR_CODE init();
	static COMPV_ERROR_CODE deInit();
	static bool haveAPI();

#if COMPV_OS_ANDROID && defined(GL_ES_VERSION_2_0) && !defined(GL_ES_VERSION_3_0)
	static PFNGLBINDVERTEXARRAYOESPROC bindVertexArrayOES;
	static PFNGLDELETEVERTEXARRAYSOESPROC deleteVertexArraysOES;
	static PFNGLGENVERTEXARRAYSOESPROC genVertexArraysOES;
	static PFNGLISVERTEXARRAYOESPROC isVertexArrayOES;
#endif

private:
	static bool s_bInitialized;
#if COMPV_OS_ANDROID && defined(GL_ES_VERSION_2_0) && !defined(GL_ES_VERSION_3_0)
	static CompVSharedLibPtr s_ptrGLESv2;
#endif
};

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

#endif /* _COMPV_GL_VAO_H_ */
