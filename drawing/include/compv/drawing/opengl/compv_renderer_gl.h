/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_DRAWING_RENDERER_GL_H_)
#define _COMPV_DRAWING_RENDERER_GL_H_

#include "compv/base/compv_config.h"
#include "compv/drawing/opengl/compv_headers_gl.h"
#if defined(HAVE_OPENGL) ||defined(HAVE_OPENGLES)
#include "compv/base/compv_common.h"
#include "compv/base/compv_obj.h"
#include "compv/drawing/compv_renderer.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class CompVRendererGL;
typedef CompVPtr<CompVRendererGL* > CompVRendererGLPtr;
typedef CompVRendererGLPtr* CompVRendererGLPtrPtr;

class CompVRendererGL : public CompVRenderer
{
protected:
	CompVRendererGL();
public:
	virtual ~CompVRendererGL();
	virtual COMPV_INLINE const char* getObjectId() {
		return "CompVRendererGL";
	};

	static COMPV_ERROR_CODE newObj(CompVRendererGLPtrPtr glRenderer);

private:
	COMPV_DISABLE_WARNINGS_BEGIN(4251 4267)
	COMPV_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) ||defined(HAVE_OPENGLES) */

#endif /* _COMPV_DRAWING_RENDERER_GL_H_ */
