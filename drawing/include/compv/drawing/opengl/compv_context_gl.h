/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_DRAWING_OPENGL_CONTEXT_H_)
#define _COMPV_DRAWING_OPENGL_CONTEXT_H_

#include "compv/drawing/compv_config.h"
#include "compv/gl/compv_gl_headers.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/base/compv_autolock.h"
#include "compv/base/compv_obj.h"
#include "compv/drawing/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class CompVContextGL;
typedef CompVPtr<CompVContextGL* > CompVContextGLPtr;
typedef CompVContextGLPtr* CompVContextGLPtrPtr;

class CompVContextGL : public CompVObj, public CompVLock
{
protected:
	CompVContextGL();
public:
	virtual ~CompVContextGL();
	static bool isSet();

	virtual COMPV_ERROR_CODE makeCurrent();
	virtual COMPV_ERROR_CODE swapBuffers();
	virtual COMPV_ERROR_CODE unmakeCurrent();
};

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

#endif /* _COMPV_DRAWING_OPENGL_CONTEXT_H_ */
