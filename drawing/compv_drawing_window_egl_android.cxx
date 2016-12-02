/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/drawing/compv_drawing_window_egl_android.h"
#if COMPV_OS_ANDROID && defined(HAVE_EGL)
#include "compv/drawing/compv_drawing.h"
#include "compv/base/android/compv_android_native_activity.h"


COMPV_NAMESPACE_BEGIN()

//
//	CompVWindowFactoryEGLAndroid
//
static COMPV_ERROR_CODE CompVWindowFactoryEGLAndroid_newObj(CompVWindowPtrPtr window, size_t width, size_t height, const char* title)
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

}

EGLNativeWindowType CompVWindowEGLAndroid::nativeWindow() /* Overrides(CompVWindowEGL) */
{
    if (!m_pNativeWindow) {
        struct android_app* app = AndroidApp_get();
        if (app && app->window) {
            m_pNativeWindow = app->window;
        }
    }
    return m_pNativeWindow;
}

COMPV_ERROR_CODE CompVWindowEGLAndroid::newObj(CompVWindowEGLAndroidPtrPtr eglWindow, size_t width, size_t height, const char* title)
{
    COMPV_CHECK_CODE_RETURN(CompVDrawing::init());
    COMPV_CHECK_EXP_RETURN(eglWindow == NULL || width <= 0 || height <= 0 || !title || !::strlen(title), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    struct android_app* app = AndroidApp_get();
    COMPV_CHECK_EXP_RETURN(!app, COMPV_ERROR_CODE_E_INVALID_STATE);

    // Set default size to fullscreen
    if (app->window) {
        size_t newWidth = static_cast<size_t>(ANativeWindow_getWidth(app->window));
        size_t newheight = static_cast<size_t>(ANativeWindow_getHeight(app->window));
        if (width && height) {
            COMPV_DEBUG_INFO("Android, setting size to fullscreen: (%zd,%zd)->(%zd,%zd)", width, height, newWidth, newheight);
            width = newWidth;
            height = newheight;
        }
    }

    CompVWindowEGLAndroidPtr eglWindow_ = new CompVWindowEGLAndroid(width, height, title);
    COMPV_CHECK_EXP_RETURN(!eglWindow_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    COMPV_CHECK_EXP_RETURN(!eglWindow_->isInitialized(), COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    eglWindow_->m_pNativeWindow = app->window;
    //COMPV_CHECK_CODE_RETURN(CompVGLContextAndroidEGL::newObj(&eglWindow_->m_ptrContext, eglWindow_->m_pEGLDisplay, eglWindow_->m_pEGLSurface, eglWindow_->m_pEGLContex));
    *eglWindow = eglWindow_;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* COMPV_OS_ANDROID && defined(HAVE_EGL) */
