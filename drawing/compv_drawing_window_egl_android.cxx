/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/drawing/compv_drawing_window_egl_android.h"
#if COMPV_OS_ANDROID && defined(HAVE_EGL)
#include "compv/drawing/compv_drawing.h"
#include "compv/base/android/compv_android_native_activity.h"

#include <android/native_window_jni.h>

#define COMPV_THIS_CLASSNAME "CompVWindowEGLAndroid"

COMPV_NAMESPACE_BEGIN()

//
//	CompVWindowFactoryEGLAndroid
//
static COMPV_ERROR_CODE CompVWindowFactoryEGLAndroid_newObj(CompVWindowPrivPtrPtr window, size_t width, size_t height, const char* title)
{
    COMPV_CHECK_EXP_RETURN(!window || width <= 0 || height <= 0 || !title || !::strlen(title), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVWindowEGLAndroidPtr eglWindow_;
    COMPV_CHECK_CODE_RETURN(CompVWindowEGLAndroid::newObj(&eglWindow_, width, height, title));
    *window = *eglWindow_;
    return COMPV_ERROR_CODE_S_OK;
}

const CompVWindowFactory CompVWindowFactoryEGLAndroid = {
    "AndroidEGL",
    CompVWindowFactoryEGLAndroid_newObj
};

//
//	CompVWindowEGLAndroid
//

CompVWindowEGLAndroid::CompVWindowEGLAndroid(size_t width, size_t height, const char* title)
    : CompVWindowEGL(width, height, title)
    , m_pNativeWindow(NULL)
{
}

CompVWindowEGLAndroid::~CompVWindowEGLAndroid()
{
	if (m_pNativeWindow) {
		ANativeWindow_release(m_pNativeWindow);
		m_pNativeWindow = NULL;
	}
}

EGLNativeWindowType CompVWindowEGLAndroid::nativeWindow() /* Overrides(CompVWindowEGL) */
{
    if (!m_pNativeWindow) {
        struct android_app* app = AndroidApp_get();
        if (app && app->window) {
			COMPV_CHECK_EXP_NOP(ANativeWindow_getFormat(app->window) != WINDOW_FORMAT_RGBA_8888, COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT);
            m_pNativeWindow = app->window;
			ANativeWindow_acquire(m_pNativeWindow);
        }
    }
    return m_pNativeWindow;
}

COMPV_ERROR_CODE CompVWindowEGLAndroid::attachToSurface(JNIEnv* jniEnv, jobject javaSurface) /* Overrides(CompVWindow) */
{
	COMPV_CHECK_EXP_RETURN(!jniEnv || !javaSurface, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	// 'ANativeWindow_fromSurface' returns the ANativeWindow associated with a Java Surface object, for interacting with it through native code.
	// This acquires a reference on the ANativeWindow that is returned; be sure to use ANativeWindow_release() when done with it so that it doesn't leak.
	// https://developer.android.com/ndk/reference/group___native_activity.html#ga774d0a87ec496b3940fcddccbc31fd9d
	ANativeWindow* nativeNindow = ANativeWindow_fromSurface(jniEnv, javaSurface);
	COMPV_CHECK_EXP_RETURN(!nativeNindow, COMPV_ERROR_CODE_E_SYSTEM);
	// Set window format to RGBA, required by OpenGL surface
	// If 'ANativeWindow_setBuffersGeometry' fails we can call 'eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format)' to retrieve the default supported format. 
	if (ANativeWindow_setBuffersGeometry(nativeNindow, width(), height(), WINDOW_FORMAT_RGBA_8888) != 0) {
		ANativeWindow_release(nativeNindow);
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT);
	}
	// Release the previous native window
	if (m_pNativeWindow) {
		ANativeWindow_release(m_pNativeWindow);
	}
	// Set the native window
	m_pNativeWindow = nativeNindow; //!\\ no 'ANativeWindow_acquire(nativeNindow)' needed: see above
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVWindowEGLAndroid::priv_updateState(COMPV_WINDOW_STATE newState) /*Overrides(CompVWindowPriv)*/
{
	COMPV_AUTOLOCK_THIS(CompVWindowEGLAndroid);
	// The base class implementation will close the context handlers and free the GL context. It must
	// be called before releasing the window handler as the context is based on it.
	COMPV_CHECK_CODE_NOP(CompVWindowEGL::priv_updateState(newState)); // call base class implementation
	switch (newState) {
		// Android native activity returned 'APP_CMD_TERM_WINDOW' which means the window is no longer valid.
		// This even is raised by the system when the app is put on background
		case COMPV_WINDOW_STATE_CONTEXT_DESTROYED:
		case COMPV_WINDOW_STATE_CLOSED:
			if (m_pNativeWindow) {
				ANativeWindow_release(m_pNativeWindow);
				m_pNativeWindow = NULL;
			}
			break;
		default:
			break;
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVWindowEGLAndroid::newObj(CompVWindowEGLAndroidPtrPtr eglWindow, size_t width, size_t height, const char* title)
{
	COMPV_CHECK_EXP_RETURN(!CompVDrawing::isInitialized(), COMPV_ERROR_CODE_E_NOT_INITIALIZED);
    COMPV_CHECK_EXP_RETURN(eglWindow == NULL || width <= 0 || height <= 0 || !title || !::strlen(title), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    struct android_app* app = AndroidApp_get();
    COMPV_CHECK_EXP_RETURN(!app, COMPV_ERROR_CODE_E_INVALID_STATE);

    // Set default size to fullscreen
    if (app->window) {
        size_t newWidth = static_cast<size_t>(ANativeWindow_getWidth(app->window));
        size_t newheight = static_cast<size_t>(ANativeWindow_getHeight(app->window));
        if (width && height) {
			COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Android, setting size to fullscreen: (%zd,%zd)->(%zd,%zd)", width, height, newWidth, newheight);
            width = newWidth;
            height = newheight;
        }
    }

    CompVWindowEGLAndroidPtr eglWindow_ = new CompVWindowEGLAndroid(width, height, title);
    COMPV_CHECK_EXP_RETURN(!eglWindow_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    COMPV_CHECK_EXP_RETURN(!eglWindow_->isInitialized(), COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    eglWindow_->m_pNativeWindow = app->window;
    *eglWindow = eglWindow_;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* COMPV_OS_ANDROID && defined(HAVE_EGL) */
