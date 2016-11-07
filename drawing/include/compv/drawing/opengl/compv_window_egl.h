/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_DRAWING_WINDOW_EGL_H_)
#define _COMPV_DRAWING_WINDOW_EGL_H_

#include "compv/base/compv_config.h"
#include "compv/drawing/opengl/compv_headers_gl.h"
#if defined(HAVE_EGL) && 0
#include "compv/base/compv_common.h"
#include "compv/drawing/compv_window.h"
#include "compv/base/parallel/compv_thread.h"
#include "compv/base/parallel/compv_mutex.h"

#include <string>

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class CompVWindowEGL;
typedef CompVPtr<CompVWindowEGL* > CompVWindowEGLPtr;
typedef CompVWindowEGLPtr* CompVWindowEGLPtrPtr;

class CompVWindowEGL : public CompVWindow
{
protected:
	CompVWindowEGL(int width, int height, const char* title);
public:
	virtual ~CompVWindowEGL();
	virtual COMPV_INLINE const char* getObjectId() {
		return "CompVWindowEGL";
	};

	virtual bool isClosed()const;
	virtual COMPV_ERROR_CODE close();
	virtual COMPV_ERROR_CODE beginDraw();
	virtual COMPV_ERROR_CODE endDraw();
	virtual CompVSurfacePtr surface();

	static COMPV_ERROR_CODE newObj(CompVWindowEGLPtrPtr eglWindow, int width, int height, const char* title);

protected:
	virtual CompVGLContext getGLContext()const { return static_cast<CompVGLContext>(m_pEGLContex); }

private:
	COMPV_ERROR_CODE init();
	COMPV_ERROR_CODE deInit();
	COMPV_ERROR_CODE makeGLContextCurrent();
	COMPV_ERROR_CODE unmakeGLContextCurrent();

private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
	CompVMutexPtr m_ptrSDLMutex;
	CompVSurfacePtr m_ptrSurface;
	EGLDisplay m_pEGLDisplay;
	EGLSurface m_pEGLSurface;
	EGLContext m_pEGLContex;
	bool m_bDrawing;
	COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* HAVE_EGL */

#endif /* _COMPV_DRAWING_WINDOW_EGL_H_ */
