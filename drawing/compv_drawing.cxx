/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/drawing/compv_drawing.h"
#include "compv/drawing/compv_drawing_image_libjpeg.h"
#include "compv/gl/compv_gl_headers.h"
#include "compv/base/compv_base.h"
#include "compv/base/compv_mat.h"
#include "compv/base/image/compv_image_decoder.h"
#include "compv/base/drawing/compv_window_registry.h"
#include "compv/base/android/compv_android_native_activity.h"
#include "compv/gl/compv_gl.h"
#include "compv/gl/compv_gl_info.h"


#include "compv/drawing/compv_drawing_window_sdl.h"
#include "compv/drawing/compv_drawing_window_egl_android.h"
#include "compv/drawing/compv_drawing_canvas_skia.h"
#include "compv/gl/drawing/compv_gl_canvas_impl.h"

#define COMPV_THIS_CLASSNAME "CompVDrawing"

COMPV_NAMESPACE_BEGIN()

bool CompVDrawing::s_bInitialized = false;
bool CompVDrawing::s_bLoopRunning = false;
CompVThreadPtr CompVDrawing::s_WorkerThread = NULL;
CompVSemaphorePtr CompVDrawing::s_WorkerSemaphore = NULL;
CompVRunLoopListenerPtr CompVDrawing::s_ptrListener = NULL;
std::vector<COMPV_RUNLOOP_STATE> CompVDrawing::s_vecStates;
#if COMPV_OS_ANDROID
CompVDrawingAndroidEngine CompVDrawing::s_AndroidEngine = {
    .animating = false,
    .app = NULL,
    .width = 0,
    .height = 0,
    .state = {.angle = 0.f,.x = 0, .y = 0 },
};
#endif /* COMPV_OS_ANDROID */

#if HAVE_SDL_H
static SDL_Window *s_pSDLMainWindow = NULL;
static SDL_GLContext s_pSDLMainContext = NULL;
#endif /* HAVE_SDL_H */

CompVDrawing::CompVDrawing()
{

}

CompVDrawing::~CompVDrawing()
{

}

COMPV_ERROR_CODE CompVDrawing::init()
{
    if (s_bInitialized) {
        return COMPV_ERROR_CODE_S_OK;
    }
    COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
#if defined(HAVE_GL_GLEW_H)
	GLenum glewErr;
#endif

    COMPV_DEBUG_INFO("Initializing [drawing] module (v %s)...", COMPV_VERSION_STRING);

    COMPV_CHECK_CODE_BAIL(err = CompVBase::init());
    COMPV_CHECK_CODE_BAIL(err = CompVGL::init());

    /* Android */
#if COMPV_OS_ANDROID
    COMPV_DEBUG_INFO("[Drawing] module: android API version: %d", __ANDROID_API__);
    COMPV_CHECK_CODE_BAIL(err = CompVWindowFactory::set(&CompVWindowFactoryEGLAndroid));
#endif

    /* Canvas */
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
	COMPV_CHECK_CODE_BAIL(err = CompVCanvasFactory::set(&CompVCanvasFactoryGL));
#elif HAVE_SKIA
    COMPV_CHECK_CODE_BAIL(err = CompVCanvasFactory::set(&CompVCanvasFactorySkia));
#endif

    /* SDL */
#if defined(HAVE_SDL_H)
    // Init SDL window factory
    COMPV_CHECK_CODE_BAIL(err = CompVWindowFactory::set(&CompVWindowFactorySDL));

    // Init SDL library
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        COMPV_DEBUG_ERROR_EX("SDL", "SDL_Init failed: %s", SDL_GetError());
        COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_E_SDL);
    }
    COMPV_DEBUG_INFO_EX("SDL", "SDL_Init succeeded");
#if 0 // TODO(dmi)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#endif
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24); // because of 'GL_DEPTH24_STENCIL8'
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8); // Skia requires 8bits stencil
#if defined(HAVE_OPENGL) && 0 // TODO(dmi)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#elif defined(HAVE_OPENGLES) && 0 // TODO(dmi)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#endif
#if 0 // TODO(dmi): allowing sotftware rendering fallback
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
#endif

    s_pSDLMainWindow = SDL_CreateWindow(
                           "Main window",
                           SDL_WINDOWPOS_UNDEFINED,
                           SDL_WINDOWPOS_UNDEFINED,
                           1,
                           1,
                           SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN
                       );
    if (!s_pSDLMainWindow) {
        COMPV_DEBUG_ERROR("SDL_CreateWindow(%d, %d, %s) failed: %s", 1, 1, "Main window", SDL_GetError());
        COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_E_SDL);
    }
    s_pSDLMainContext = SDL_GL_CreateContext(s_pSDLMainWindow);
    if (!s_pSDLMainContext) {
        COMPV_DEBUG_ERROR("SDL_GL_CreateContext() failed: %s", SDL_GetError());
        COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_E_SDL);
    }
    COMPV_CHECK_EXP_BAIL(SDL_GL_MakeCurrent(s_pSDLMainWindow, s_pSDLMainContext) != 0, COMPV_ERROR_CODE_E_SDL);
#	if defined(HAVE_GL_GLEW_H)
    COMPV_CHECK_EXP_BAIL(!CompVGLUtils::isGLContextSet(), err = COMPV_ERROR_CODE_E_GL_NO_CONTEXT);
    glewErr = glewInit();
    if (GLEW_OK != glewErr) {
        COMPV_DEBUG_ERROR_EX("GLEW", "glewInit for [drawing] module failed: %s", glewGetErrorString(glewErr));
        COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_E_GLEW);
    }
    COMPV_DEBUG_INFO_EX("GLEW", "glewInit for [drawing] module succeeded");
#	endif /* HAVE_GL_GLEW_H */
    // Gather info (init supported extensions)
    COMPV_CHECK_CODE_BAIL(err = CompVGLInfo::gather());
    // Init glew for gl module
    COMPV_CHECK_CODE_BAIL(err = CompVGL::glewInit());
#endif /* SDL */

#if (defined(HAVE_JPEGLIB_H) || defined(HAVE_SKIA))
    //extern COMPV_ERROR_CODE libjpegDecodeFile(const char* filePath, CompVMatPtrPtr mat);
    //extern COMPV_ERROR_CODE libjpegDecodeInfo(const char* filePath, CompVImageInfo& info);
    COMPV_CHECK_CODE_BAIL(err = CompVImageDecoder::setFuncPtrs(COMPV_IMAGE_FORMAT_JPEG, libjpegDecodeFile, libjpegDecodeInfo));
#else
    COMPV_DEBUG_INFO("/!\\ No jpeg decoder found");
#endif

    CompVDrawing::s_bInitialized = true;
    COMPV_DEBUG_INFO("Drawing module initialized");

bail:
    if (COMPV_ERROR_CODE_IS_NOK(err)) {
		COMPV_CHECK_CODE_NOP(CompVDrawing::deInit());
    }
    return err;
}

COMPV_ERROR_CODE CompVDrawing::deInit()
{
    COMPV_CHECK_CODE_NOP(CompVBase::deInit());
    COMPV_CHECK_CODE_NOP(CompVGL::deInit());

    CompVDrawing::breakLoop();
	CompVDrawing::s_ptrListener = NULL;
	if (CompVDrawing::s_WorkerThread) {
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Joining the worker thread...");
		COMPV_CHECK_CODE_NOP(CompVDrawing::s_WorkerThread->join(), "Failed to join the worker thread");
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Worker thread joined");
		CompVDrawing::s_WorkerThread = NULL;
	}
	CompVDrawing::s_WorkerSemaphore = NULL;

#if defined(HAVE_SDL_H)
    if (s_pSDLMainContext) {
        SDL_GL_DeleteContext(s_pSDLMainContext);
        s_pSDLMainContext = NULL;
    }
    if (s_pSDLMainWindow) {
        SDL_DestroyWindow(s_pSDLMainWindow);
        s_pSDLMainWindow = NULL;
    }
    SDL_Quit();
#endif /* HAVE_SDL_H */

    CompVDrawing::s_bInitialized = false;

    COMPV_DEBUG_INFO("Drawing module deinitialized");

    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVDrawing::runLoop(CompVRunLoopListenerPtr listener COMPV_DEFAULT(NULL))
{
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "%s(%p)", __FUNCTION__, *listener);
    COMPV_CHECK_EXP_RETURN(!CompVDrawing::isInitialized(), COMPV_ERROR_CODE_E_NOT_INITIALIZED);
    if (CompVDrawing::isLoopRunning()) {
        return COMPV_ERROR_CODE_S_OK;
    }
	CompVDrawing::s_ptrListener = listener;
#   if COMPV_OS_APPLE
    if (!pthread_main_np()) {
        COMPV_DEBUG_WARN_EX(COMPV_THIS_CLASSNAME, "MacOS: Runnin even loop outside main thread");
    }
#   endif /* COMPV_OS_APPLE */
    COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
    CompVDrawing::s_bLoopRunning = true;
    compv_thread_id_t eventLoopThreadId = CompVThread::getIdCurrent();

    // http://forum.lwjgl.org/index.php?topic=5836.0
    // Context creation and using rules:
    //There are 3 concepts you need to be aware of : (A)window / context creation(B) running the event loop that dispatches events for the window and(C) making the context current in a thread.
    //	- On Windows, (A)and(B) must happen on the same thread.It doesn't have to be the main thread. (C) can happen on any thread.
    //	- On Linux, you can have(A), (B)and(C) on any thread.
    //	- On Mac OS X, (A)and(B) must happen on the same thread, that must also be the main thread(thread 0).Again, (C)can happen on any thread.
    COMPV_DEBUG_INFO("Running event loop on thread with id = %ld", (long)eventLoopThreadId);

#if COMPV_OS_ANDROID
    COMPV_CHECK_CODE_BAIL(err = CompVDrawing::android_runLoop(AndroidApp_get()));
#elif defined(HAVE_SDL_H)
    if (CompVDrawing::s_ptrListener) {
		COMPV_CHECK_CODE_BAIL(err = CompVSemaphore::newObj(&CompVDrawing::s_WorkerSemaphore));
        COMPV_CHECK_CODE_BAIL(err = CompVThread::newObj(&CompVDrawing::s_WorkerThread, CompVDrawing::workerThread, NULL));
    }
    COMPV_CHECK_CODE_BAIL(err = CompVDrawing::sdl_runLoop());
#else
	if (CompVDrawing::s_ptrListener) {
		COMPV_CHECK_CODE_BAIL(err = CompVSemaphore::newObj(&CompVDrawing::s_WorkerSemaphore));
        COMPV_CHECK_CODE_BAIL(err = CompVThread::newObj(&CompVDrawing::s_WorkerThread, WorkerThread, userData));
    }
    while (CompVDrawing::s_bLoopRunning) {
        if (CompVDrawing::windowsCount() == 0) {
            COMPV_DEBUG_INFO("No active windows in the event loop... breaking the loop");
            goto bail;
        }
        CompVThread::sleep(1);
    }
#endif

bail:
    CompVDrawing::s_bLoopRunning = false;
	COMPV_CHECK_CODE_NOP(CompVDrawing::signalState(COMPV_RUNLOOP_STATE_LOOP_STOPPED)); // will increment the semaphore
	
	// Join the worker thread
	if (CompVDrawing::s_WorkerThread) {
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Joining the worker thread...");
		COMPV_CHECK_CODE_NOP(CompVDrawing::s_WorkerThread->join(), "Failed to join the worker thread");
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Worker thread joined");
	}
    CompVDrawing::s_WorkerThread = NULL;
    return err;
}

#if defined(HAVE_SDL_H)
#define COMPV_SDL_GET_WINDOW() \
 CompVWindowPrivPtr windPriv = static_cast<CompVWindowPriv*>(SDL_GetWindowData(SDL_GetWindowFromID(sdlEvent.window.windowID), "This")); \
 COMPV_CHECK_EXP_BAIL(!windPriv, err = COMPV_ERROR_CODE_E_SDL)

COMPV_ERROR_CODE CompVDrawing::sdl_runLoop()
{
	SDL_Event sdlEvent;
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;

	while (CompVDrawing::s_bLoopRunning) {
		if (CompVWindowRegistry::count() == 0) {
			COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "No active windows in the event loop... breaking the loop");
			goto bail;
		}

		if (SDL_WaitEvent(&sdlEvent)) {
			if (sdlEvent.type == SDL_QUIT) {
				COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Quit called");
				CompVDrawing::s_bLoopRunning = false;
			}
			else if (sdlEvent.type == SDL_WINDOWEVENT) {
				switch (sdlEvent.window.event) {
					case SDL_WINDOWEVENT_CLOSE: {
						COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "SDL_WINDOWEVENT_CLOSE");
						COMPV_SDL_GET_WINDOW();
						COMPV_CHECK_CODE_BAIL(err = windPriv->priv_updateState(COMPV_WINDOW_STATE_CLOSED));
						break;
					}
					case SDL_WINDOWEVENT_SIZE_CHANGED: {
						COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "SDL_WINDOWEVENT_SIZE_CHANGED");
						COMPV_SDL_GET_WINDOW();
						COMPV_CHECK_CODE_BAIL(err = windPriv->priv_updateSize(static_cast<size_t>(sdlEvent.window.data1), static_cast<size_t>(sdlEvent.window.data2)));
						break;
					}
					case SDL_WINDOWEVENT_FOCUS_GAINED: {
						COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "SDL_WINDOWEVENT_SIZE_CHANGED");
						COMPV_SDL_GET_WINDOW();
						COMPV_CHECK_CODE_BAIL(err = windPriv->priv_updateState(COMPV_WINDOW_STATE_FOCUS_GAINED));
						break;
					}
					case SDL_WINDOWEVENT_FOCUS_LOST: {
						COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "SDL_WINDOWEVENT_SIZE_CHANGED");
						COMPV_SDL_GET_WINDOW();
						COMPV_CHECK_CODE_BAIL(err = windPriv->priv_updateState(COMPV_WINDOW_STATE_FOCUS_GAINED));
						break;
					}
				}
			}
		}
		else {
			COMPV_CHECK_CODE_BAIL(err = CompVDrawing::breakLoop());
		}
	}

bail:
	return err;
}
#endif /* HAVE_SDL_H */

#if COMPV_OS_ANDROID

// Process the next input event
int32_t CompVDrawing::android_engine_handle_input(struct android_app* app, AInputEvent* event)
{
    CompVDrawingAndroidEngine* engine = static_cast<CompVDrawingAndroidEngine*>(app->userData);
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        engine->state.x = AMotionEvent_getX(event, 0);
        engine->state.y = AMotionEvent_getY(event, 0);
        return 1;
    }
    return 0;
}

// Process the next main command
void CompVDrawing::android_engine_handle_cmd(struct android_app* app, int32_t cmd)
{
    CompVDrawingAndroidEngine* engine = static_cast<CompVDrawingAndroidEngine*>(app->userData);
    switch (cmd) {
		case APP_CMD_SAVE_STATE:
			COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "APP_CMD_SAVE_STATE");
			// The system has asked us to save our current state.  Do so.
			if (!engine->app->savedState) {
				engine->app->savedState = malloc(sizeof(CompVDrawingAndroidSavedState)); // must not use CompVMem::malloc because
			}
			*static_cast<CompVDrawingAndroidSavedState*>(engine->app->savedState) = engine->state;
			engine->app->savedStateSize = sizeof(CompVDrawingAndroidSavedState);
			break;
		case APP_CMD_INIT_WINDOW:
			COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "APP_CMD_INIT_WINDOW");
			// The window is being shown, get it ready.
			if (engine->app->window != NULL) {
				if (CompVDrawing::s_ptrListener) {
					COMPV_CHECK_CODE_NOP(CompVSemaphore::newObj(&CompVDrawing::s_WorkerSemaphore));
					COMPV_CHECK_CODE_NOP(CompVThread::newObj(&CompVDrawing::s_WorkerThread, CompVDrawing::workerThread, app));
				}
			}
			break;
		case APP_CMD_TERM_WINDOW:
			COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "APP_CMD_TERM_WINDOW");
			// The window is being hidden or closed, clean it up.
			engine->animating = false;
			break;
		case APP_CMD_GAINED_FOCUS:
			COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "APP_CMD_GAINED_FOCUS");
			break;
		case APP_CMD_LOST_FOCUS:
			COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "APP_CMD_LOST_FOCUS");
			engine->animating = false;
			break;

		case APP_CMD_RESUME:
			COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "APP_CMD_RESUME -> onStateChanged(COMPV_RUNLOOP_STATE_ANIMATION_RESUMED)");
			COMPV_CHECK_CODE_NOP(CompVDrawing::signalState(COMPV_RUNLOOP_STATE_ANIMATION_RESUMED));
			break;
		case APP_CMD_START:
			COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "APP_CMD_START -> onStateChanged(COMPV_RUNLOOP_STATE_ANIMATION_STARTED)");
			COMPV_CHECK_CODE_NOP(CompVDrawing::signalState(COMPV_RUNLOOP_STATE_ANIMATION_STARTED));
			break;
		case APP_CMD_PAUSE:
			COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "APP_CMD_PAUSE -> onStateChanged(COMPV_RUNLOOP_STATE_ANIMATION_PAUSED)");
			COMPV_CHECK_CODE_NOP(CompVDrawing::signalState(COMPV_RUNLOOP_STATE_ANIMATION_PAUSED));
			break;
		case APP_CMD_STOP:
			COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "APP_CMD_STOP -> onStateChanged(COMPV_RUNLOOP_STATE_ANIMATION_STOPPED)");
			COMPV_CHECK_CODE_NOP(CompVDrawing::signalState(COMPV_RUNLOOP_STATE_ANIMATION_STOPPED));
			break;
    }
	COMPV_CHECK_CODE_NOP(CompVWindowRegistry::android_handle_cmd(cmd));
}

COMPV_ERROR_CODE CompVDrawing::android_runLoop(struct android_app* state)
{
    COMPV_CHECK_EXP_RETURN(!state, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
    int ident;
    int events;
    struct android_poll_source* source;

    state->userData = &CompVDrawing::s_AndroidEngine;
    state->onAppCmd = CompVDrawing::android_engine_handle_cmd;
    state->onInputEvent = CompVDrawing::android_engine_handle_input;
    CompVDrawing::s_AndroidEngine.app = state;

    CompVDrawing::s_AndroidEngine.animating = true;
    while (CompVDrawing::s_bLoopRunning) {
        if (CompVWindowRegistry::count() == 0) {
            COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "No active windows in the event loop... breaking the loop");
            goto bail;
        }
        // Read all pending events.
        // If not animating, we will block forever waiting for events.
        // If animating, we loop until all events are read, then continue
        // to draw the next frame of animation.
		// FIXME(dmi): always wait forever?
		// FIXME(dmi): High CPU usage when timeout value is equal to zero
        while ((ident = ALooper_pollAll(-1/*CompVDrawing::s_AndroidEngine.animating ? 0 : -1*/, NULL, &events, (void**)&source)) >= 0) {
            // Process this event.
            if (source != NULL) {
                source->process(state, source);
            }

            // If a sensor has data, process it now.
            //if (ident == LOOPER_ID_USER) {
            //	if (CompVDrawing::s_AndroidEngine.accelerometerSensor != NULL) {
            //		ASensorEvent event;
            //		while (ASensorEventQueue_getEvents(engine.sensorEventQueue,
            //			&event, 1) > 0) {
            //			LOGI("accelerometer: x=%f y=%f z=%f",
            //				event.acceleration.x, event.acceleration.y,
            //				event.acceleration.z);
            //		}
            //	}
            //}

            // Check if we are exiting.
            if (state->destroyRequested != 0) {
				// This code is called when the application is about to exit or the display mode is switching (portrait <-> landscape)
				COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Destroy requested");
				COMPV_CHECK_CODE_BAIL(err = CompVDrawing::breakLoop());
                goto bail;
            }
        }

        if (CompVDrawing::s_AndroidEngine.animating) { // FIXME: 'animating' not considered
            // Done with events; draw next animation frame.
            //CompVDrawing::s_AndroidEngine.state.angle += .01f;
            //if (CompVDrawing::s_AndroidEngine.state.angle > 1) {
            //	CompVDrawing::s_AndroidEngine.state.angle = 0;
            //}

            // Drawing is throttled to the screen update rate, so there
            // is no need to do timing here.
            //engine_draw_frame(&engine);
        }
    }

bail:
    return err;
}
#endif /* COMPV_OS_ANDROID */

COMPV_ERROR_CODE CompVDrawing::breakLoop()
{
	if (CompVDrawing::isInitialized()) {
		COMPV_CHECK_CODE_RETURN(CompVWindowRegistry::closeAll());
	}
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVDrawing::signalState(COMPV_RUNLOOP_STATE state)
{
	if (CompVDrawing::s_ptrListener && CompVDrawing::s_WorkerSemaphore) {
		s_vecStates.push_back(state);
		COMPV_CHECK_CODE_RETURN(CompVDrawing::s_WorkerSemaphore->increment());
	}
	return COMPV_ERROR_CODE_S_OK;
}

void* COMPV_STDCALL CompVDrawing::workerThread(void* arg)
{
	COMPV_RUNLOOP_STATE state;
	if (CompVDrawing::isLoopRunning()) {
		/* First time */ {
			COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "-> onStateChanged(COMPV_RUNLOOP_STATE_LOOP_STARTED)");
			CompVDrawing::s_ptrListener->onStateChanged(COMPV_RUNLOOP_STATE_LOOP_STARTED);
#if !COMPV_OS_ANDROID // Android: wait for Activity::onResume() or Activity::onStart
			COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "-> onStateChanged(COMPV_RUNLOOP_STATE_ANIMATION_STARTED)");
			CompVDrawing::s_ptrListener->onStateChanged(COMPV_RUNLOOP_STATE_ANIMATION_STARTED);
#endif
		} /* EndOf-FirstTime */

		while (CompVDrawing::isLoopRunning()) {
			COMPV_CHECK_CODE_BAIL(CompVDrawing::s_WorkerSemaphore->decrement());
			while (!s_vecStates.empty()) {
				state = *s_vecStates.begin();
				s_vecStates.erase(s_vecStates.begin());
				COMPV_CHECK_CODE_BAIL(CompVDrawing::s_ptrListener->onStateChanged(state));
			}
		}
	}

bail:
	return NULL;
}

COMPV_NAMESPACE_END()

