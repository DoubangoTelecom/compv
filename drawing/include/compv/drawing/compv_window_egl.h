/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_DRAWING_WINDOW_EGL_H_)
#define _COMPV_DRAWING_WINDOW_EGL_H_

#include "compv/base/compv_config.h"
#include "compv/gl/compv_gl_headers.h"
#if defined(HAVE_EGL)
#include "compv/gl/compv_gl_window.h"
#include "compv/gl/compv_gl_context.h"
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

//
//	CompVGLContextEGL
//
class CompVGLContextEGL;
typedef CompVPtr<CompVGLContextEGL* > CompVGLContextEGLPtr;
typedef CompVGLContextEGLPtr* CompVGLContextEGLPtrPtr;

class CompVGLContextEGL : public CompVGLContext
{
protected:
	CompVGLContextEGL(EGLDisplay pEGLDisplay, EGLSurface pEGLSurface, EGLContext pEGLContex);
public:
	virtual ~CompVGLContextEGL();
	COMPV_GET_OBJECT_ID("CompVGLContextEGL");

	virtual COMPV_ERROR_CODE makeCurrent() override /* Overrides(CompVGLContext) */;
	virtual COMPV_ERROR_CODE swapBuffers()  override /* Overrides(CompVGLContext) */;
	virtual COMPV_ERROR_CODE unmakeCurrent()  override /* Overrides(CompVGLContext) */;

	static COMPV_ERROR_CODE newObj(CompVGLContextEGLPtrPtr context, EGLDisplay pEGLDisplay, EGLSurface pEGLSurface, EGLContext pEGLContex);

private:
	EGLDisplay m_pEGLDisplay;
	EGLSurface m_pEGLSurface;
	EGLContext m_pEGLContex;
};

//
//	CompVWindowEGL
//
class CompVWindowEGL;
typedef CompVPtr<CompVWindowEGL* > CompVWindowEGLPtr;
typedef CompVWindowEGLPtr* CompVWindowEGLPtrPtr;

class CompVWindowEGL : public CompVGLWindow
{
protected:
	CompVWindowEGL(size_t width, size_t height, const char* title);
public:
	virtual ~CompVWindowEGL();
	
	virtual bool isClosed() const override /* Overrides(CompVGLWindow) */;
	virtual COMPV_ERROR_CODE close() override /* Overrides(CompVGLWindow) */;
	virtual COMPV_ERROR_CODE beginDraw() override /* Overrides(CompVGLWindow) */;
	// Override(CompVWindow::endDraw) -> CompVGLWindow
	// Override(CompVWindow::surface) -> CompVGLWindow

	virtual EGLNativeWindowType nativeWindow() = 0;

protected:
	virtual CompVGLContextPtr context() override /* Overrides(CompVGLWindow) */;

private:
	COMPV_ERROR_CODE init();
	COMPV_ERROR_CODE deInit();

private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
	EGLDisplay m_pEGLDisplay;
	EGLSurface m_pEGLSurface;
	EGLContext m_pEGLContex;
	CompVGLContextEGLPtr m_ptrContext;
	COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* HAVE_EGL */

#endif /* _COMPV_DRAWING_WINDOW_EGL_H_ */
