/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_DRAWING_WINDOW_ANDROID_EGL_H_)
#define _COMPV_DRAWING_WINDOW_ANDROID_EGL_H_

#include "compv/base/compv_config.h"
#include "compv/gl/compv_gl_headers.h"
#if COMPV_OS_ANDROID && defined(HAVE_EGL)
#include "compv/drawing/opengl/compv_window_gl.h"
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

//
//	CompVContextGLAndroidEGL
//
class CompVContextGLAndroidEGL;
typedef CompVPtr<CompVContextGLAndroidEGL* > CompVContextGLAndroidEGLPtr;
typedef CompVContextGLAndroidEGLPtr* CompVContextGLAndroidEGLPtrPtr;

class CompVContextGLAndroidEGL : public CompVContextGL
{
protected:
	CompVContextGLAndroidEGL(EGLDisplay pEGLDisplay, EGLSurface pEGLSurface, EGLContext pEGLContex);
public:
	virtual ~CompVContextGLAndroidEGL();
	COMPV_GET_OBJECT_ID("CompVContextGLAndroidEGL");

	virtual COMPV_ERROR_CODE makeCurrent() override;
	virtual COMPV_ERROR_CODE swapBuffers() override;
	virtual COMPV_ERROR_CODE unmakeCurrent() override;

	static COMPV_ERROR_CODE newObj(CompVContextGLAndroidEGLPtrPtr context, EGLDisplay pEGLDisplay, EGLSurface pEGLSurface, EGLContext pEGLContex);

private:
	EGLDisplay m_pEGLDisplay;
	EGLSurface m_pEGLSurface;
	EGLContext m_pEGLContex;
};

//
//	CompVWindowAndroidEGL
//
class CompVWindowAndroidEGL;
typedef CompVPtr<CompVWindowAndroidEGL* > CompVWindowAndroidEGLPtr;
typedef CompVWindowAndroidEGLPtr* CompVWindowAndroidEGLPtrPtr;

class CompVWindowAndroidEGL : public CompVWindowGL
{
protected:
	CompVWindowAndroidEGL(size_t width, size_t height, const char* title);
public:
	virtual ~CompVWindowAndroidEGL();
	virtual COMPV_INLINE const char* getObjectId() {
		return "CompVWindowAndroidEGL";
	};

	/* CompVWindow overrides */
	virtual bool isClosed()const;
	virtual COMPV_ERROR_CODE close();
	virtual COMPV_ERROR_CODE beginDraw();
	// Override(CompVWindow::endDraw) -> CompVWindowGL
	// Override(CompVWindow::surface) -> CompVWindowGL

	static COMPV_ERROR_CODE newObj(CompVWindowAndroidEGLPtrPtr eglWindow, size_t width, size_t height, const char* title);

protected:
	/* CompVWindowGL overrides */
	virtual CompVContextGLPtr context();

private:
	COMPV_ERROR_CODE init();
	COMPV_ERROR_CODE deInit();
	
private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
	EGLDisplay m_pEGLDisplay;
	EGLSurface m_pEGLSurface;
	EGLContext m_pEGLContex;
	CompVContextGLAndroidEGLPtr m_ptrContext;
	COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* COMPV_OS_ANDROID && defined(HAVE_EGL) */

#endif /* _COMPV_DRAWING_WINDOW_ANDROID_EGL_H_ */
