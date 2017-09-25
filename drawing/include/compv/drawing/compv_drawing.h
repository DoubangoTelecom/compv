/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_DRAWING_DRAWING_H_)
#define _COMPV_DRAWING_DRAWING_H_

#include "compv/drawing/compv_drawing_config.h"
#include "compv/drawing/compv_drawing_common.h"
#include "compv/base/compv_obj.h"
#include "compv/base/parallel/compv_thread.h"
#include "compv/base/parallel/compv_semaphore.h"
#include "compv/base/android/compv_android_native_activity.h"
#include "compv/base/drawing/compv_window.h"

#include <functional>

COMPV_NAMESPACE_BEGIN()

#if COMPV_OS_ANDROID
struct CompVDrawingAndroidSavedState {
    float angle;
    int32_t x;
    int32_t y;
};
struct CompVDrawingAndroidEngine {
    bool animating;
    struct android_app* app;
    int32_t width;
    int32_t height;
    CompVDrawingAndroidSavedState state;
};
#endif /* COMPV_OS_ANDROID */

enum COMPV_RUNLOOP_STATE {
	COMPV_RUNLOOP_STATE_LOOP_STARTED,
	COMPV_RUNLOOP_STATE_ANIMATION_STARTED,
	COMPV_RUNLOOP_STATE_ANIMATION_PAUSED,
	COMPV_RUNLOOP_STATE_ANIMATION_RESUMED,
	COMPV_RUNLOOP_STATE_ANIMATION_STOPPED,
	COMPV_RUNLOOP_STATE_LOOP_STOPPED
};

COMPV_OBJECT_DECLARE_PTRS(RunLoopListener)

typedef std::function<COMPV_ERROR_CODE(const COMPV_RUNLOOP_STATE& newState)> CompVRunLoopOnNewState;

class COMPV_DRAWING_API CompVDrawing
{
    friend class CompVWindow;
protected:
    CompVDrawing();
public:
    virtual ~CompVDrawing();
    static COMPV_ERROR_CODE init();
    static COMPV_ERROR_CODE deInit();
    static COMPV_INLINE bool isInitialized() {
        return s_bInitialized;
    }
    static COMPV_INLINE bool isLoopRunning() {
        return s_bLoopRunning;
    }
    static COMPV_ERROR_CODE runLoop(CompVRunLoopOnNewState cbOnNewState = nullptr);
    static COMPV_ERROR_CODE breakLoop();

private:
#if defined(HAVE_SDL_H)
    static COMPV_ERROR_CODE sdl_runLoop();
#endif
#if COMPV_OS_ANDROID
    static int32_t android_engine_handle_input(struct android_app* app, AInputEvent* event);
    static void android_engine_handle_cmd(struct android_app* app, int32_t cmd);
    static COMPV_ERROR_CODE android_runLoop(struct android_app* state);
#endif
	static COMPV_ERROR_CODE signalState(COMPV_RUNLOOP_STATE state);
	static void* COMPV_STDCALL workerThread(void* arg);

private:
    static bool s_bInitialized;
    static bool s_bLoopRunning;
    COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
    static CompVThreadPtr s_WorkerThread;
	static CompVSemaphorePtr s_WorkerSemaphore;
	static CompVRunLoopOnNewState s_cbRunLoopOnNewState;
	static std::vector<COMPV_RUNLOOP_STATE> s_vecStates;
#if COMPV_OS_ANDROID
    static CompVDrawingAndroidEngine s_AndroidEngine;
#endif /* COMPV_OS_ANDROID */

    COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_DRAWING_DRAWING_H_ */
