/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/drawing/compv_drawing.h"
#include "compv/drawing/compv_image_libjpeg.h"
#include "compv/drawing/opengl/compv_headers_gl.h"
#include "compv/base/compv_base.h"
#include "compv/base/compv_mat.h"
#include "compv/base/image/compv_image_decoder.h"

#if defined(HAVE_SDL_H)
#include <SDL.h>
#endif /* HAVE_SDL_H */

COMPV_NAMESPACE_BEGIN()

bool CompVDrawing::s_bInitialized = false;
bool CompVDrawing::s_bLoopRunning = false;
int CompVDrawing::s_iGLVersionMajor = 0;
int CompVDrawing::s_iGLVersionMinor = 0;
std::map<compv_window_id_t, CompVPtr<CompVWindow* > > CompVDrawing::m_sWindows;
CompVPtr<CompVMutex* > CompVDrawing::s_WindowsMutex = NULL;
CompVPtr<CompVThread* > CompVDrawing::s_WorkerThread = NULL;

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

	COMPV_DEBUG_INFO("Initializing drawing module (v %s)...", COMPV_VERSION_STRING);

	COMPV_CHECK_CODE_BAIL(err = CompVBase::init());

	COMPV_CHECK_CODE_BAIL(err = CompVMutex::newObj(&CompVDrawing::s_WindowsMutex));

#if defined(HAVE_GL_GLEW_H)
	COMPV_DEBUG_INFO("GLEW version being used: %d.%d.%d", GLEW_VERSION_MAJOR, GLEW_VERSION_MINOR, GLEW_VERSION_MICRO);
#endif /* HAVE_GL_GLEW_H */

#if	defined(HAVE_OPENGLES)
	COMPV_DEBUG_INFO("OpenGL-ES implementation enabled");
#elif defined(HAVE_OPENGL)
	COMPV_DEBUG_INFO("OpenGL implementation enabled");
#endif

	/* SDL */
#if defined(HAVE_SDL_H)
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
	GLenum glewErr = glewInit();
	if (GLEW_OK != glewErr) {
		COMPV_DEBUG_ERROR_EX("GLEW", "glewInit failed: %s", glewGetErrorString(glewErr));
		COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_E_GLEW);
	}
	COMPV_DEBUG_INFO_EX("GLEW", "glewInit succeeded");
#	endif /* HAVE_GL_GLEW_H */
	COMPV_DEBUG_INFO("OpenGL version string: %s", glGetString(GL_VERSION));
	COMPV_DEBUG_INFO("OpenGL shading version string: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
	GLint major, minor;
	glGetIntegerv(GL_MAJOR_VERSION, &major), CompVDrawing::s_iGLVersionMajor = static_cast<int>(major);
	glGetIntegerv(GL_MINOR_VERSION, &minor), CompVDrawing::s_iGLVersionMinor = static_cast<int>(minor);
	COMPV_DEBUG_INFO("OpenGL parsed major and minor versions: %d.%d", CompVDrawing::s_iGLVersionMajor, CompVDrawing::s_iGLVersionMinor);
	COMPV_DEBUG_INFO("OpenGL renderer string: %s", glGetString(GL_RENDERER));
	COMPV_DEBUG_INFO("OpenGL vendor string: %s", glGetString(GL_VENDOR));
	GLint maxColorAttachments = 0;
	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxColorAttachments);
	COMPV_DEBUG_INFO("GL_MAX_DRAW_BUFFERS: %d", maxColorAttachments);
	GLint maxDrawBuffers = 0;
	glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuffers);
	COMPV_DEBUG_INFO("GL_MAX_DRAW_BUFFERS: %d", maxDrawBuffers);
	COMPV_DEBUG_INFO("OpenGL extensions string: %s", glGetString(GL_EXTENSIONS));
#endif /* SDL */

#if defined(HAVE_JPEGLIB_H) || defined(HAVE_SKIA)
	extern COMPV_ERROR_CODE libjpegDecodeFile(const char* filePath, CompVMatPtrPtr mat);
	extern COMPV_ERROR_CODE libjpegDecodeInfo(const char* filePath, CompVImageInfo& info);
	COMPV_CHECK_CODE_BAIL(err = CompVImageDecoder::setFuncPtrs(COMPV_IMAGE_FORMAT_JPEG, libjpegDecodeFile, libjpegDecodeInfo));
#else
	COMPV_DEBUG_INFO("/!\\ No jpeg decoder found");
#endif

	CompVDrawing::s_bInitialized = true;
	COMPV_DEBUG_INFO("Drawing module initialized");

bail:
	if (COMPV_ERROR_CODE_IS_NOK(err)) {
		CompVDrawing::deInit();
	}
	return err;
}

COMPV_ERROR_CODE CompVDrawing::deInit()
{
	CompVBase::deInit();

	CompVDrawing::breakLoop();
	CompVDrawing::s_WindowsMutex = NULL;
	CompVDrawing::s_WorkerThread = NULL;

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

COMPV_ERROR_CODE CompVDrawing::registerWindow(CompVPtr<CompVWindow* > window)
{
	COMPV_CHECK_EXP_RETURN(!CompVDrawing::isInitialized(), COMPV_ERROR_CODE_E_NOT_INITIALIZED);
	COMPV_CHECK_EXP_RETURN(!window, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_CODE_ASSERT(CompVDrawing::s_WindowsMutex->lock());
	CompVDrawing::m_sWindows.insert(std::pair<compv_window_id_t, CompVPtr<CompVWindow* > >(window->getId(), window));
	COMPV_CHECK_CODE_ASSERT(CompVDrawing::s_WindowsMutex->unlock());

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVDrawing::unregisterWindow(CompVPtr<CompVWindow* > window)
{
	COMPV_CHECK_EXP_RETURN(!CompVDrawing::isInitialized(), COMPV_ERROR_CODE_E_NOT_INITIALIZED);
	COMPV_CHECK_CODE_RETURN(CompVDrawing::unregisterWindow(window->getId()));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVDrawing::unregisterWindow(compv_window_id_t windowId)
{
	COMPV_CHECK_EXP_RETURN(!CompVDrawing::isInitialized(), COMPV_ERROR_CODE_E_NOT_INITIALIZED);
	COMPV_CHECK_CODE_ASSERT(CompVDrawing::s_WindowsMutex->lock());
	CompVDrawing::m_sWindows.erase(windowId);
	COMPV_CHECK_CODE_ASSERT(CompVDrawing::s_WindowsMutex->unlock());

	return COMPV_ERROR_CODE_S_OK;
}

size_t CompVDrawing::windowsCount()
{
	COMPV_CHECK_CODE_ASSERT(CompVDrawing::s_WindowsMutex->lock());
	size_t count = CompVDrawing::m_sWindows.size();
	COMPV_CHECK_CODE_ASSERT(CompVDrawing::s_WindowsMutex->unlock());
	return count;
}

COMPV_ERROR_CODE CompVDrawing::runLoop(void *(COMPV_STDCALL *WorkerThread) (void *) /*= NULL*/, void *userData /*= NULL*/)
{
	COMPV_CHECK_EXP_RETURN(!CompVDrawing::isInitialized(), COMPV_ERROR_CODE_E_NOT_INITIALIZED);
	if (CompVDrawing::isLoopRunning()) {
		return COMPV_ERROR_CODE_S_OK;
	}
#   if COMPV_OS_APPLE
	if (!pthread_main_np()) {
		COMPV_DEBUG_WARN("MacOS: Runnin even loop outside main thread");
	}
#   endif /* COMPV_OS_APPLE */
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	CompVDrawing::s_bLoopRunning = true;
	compv_thread_id_t eventLoopThreadId = CompVThread::getIdCurrent();

	// Run worker thread (s_bLoopRunning must be equal to true)
	if (WorkerThread) {
		COMPV_CHECK_CODE_BAIL(err = CompVThread::newObj(&CompVDrawing::s_WorkerThread, WorkerThread, userData));
	}

	// http://forum.lwjgl.org/index.php?topic=5836.0
	// Context creation and using rules:
	//There are 3 concepts you need to be aware of : (A)window / context creation(B) running the event loop that dispatches events for the window and(C) making the context current in a thread.
	//	- On Windows, (A)and(B) must happen on the same thread.It doesn't have to be the main thread. (C) can happen on any thread.
	//	- On Linux, you can have(A), (B)and(C) on any thread.
	//	- On Mac OS X, (A)and(B) must happen on the same thread, that must also be the main thread(thread 0).Again, (C)can happen on any thread.
	COMPV_DEBUG_INFO("Running event loop on thread with id = %ld", (long)eventLoopThreadId);

#if defined(HAVE_SDL_H)
	SDL_Event sdlEvent;
#endif

	while (CompVDrawing::s_bLoopRunning) {
		if (CompVDrawing::windowsCount() == 0) {
			COMPV_DEBUG_INFO("No active windows in the event loop... breaking the loop");
			goto bail;
		}
#if defined(HAVE_SDL_H)
		if (SDL_WaitEvent(&sdlEvent)) {
			if (sdlEvent.type == SDL_QUIT) {
				COMPV_DEBUG_INFO_EX("UI", "Quit called");
				CompVDrawing::s_bLoopRunning = false;
			}
			else if (sdlEvent.type == SDL_WINDOWEVENT && sdlEvent.window.event == SDL_WINDOWEVENT_CLOSE) {
				SDL_Window* window = SDL_GetWindowFromID(sdlEvent.window.windowID);
				if (window) {
					CompVWindowPtr wind = static_cast<CompVWindow*>(SDL_GetWindowData(window, "This"));
					if (wind) {
						COMPV_CHECK_CODE_ASSERT(wind->close());
					}
				}
			}
		}
		else {
			CompVDrawing::s_bLoopRunning = false;
		}
#else
		CompVThread::sleep(1);
#endif /* HAVE_SDL_H */
	}

bail:
	CompVDrawing::s_bLoopRunning = false;
	CompVDrawing::s_WorkerThread = NULL;
	return err;
}

COMPV_ERROR_CODE CompVDrawing::breakLoop()
{
	COMPV_CHECK_EXP_RETURN(!CompVDrawing::isInitialized(), COMPV_ERROR_CODE_E_NOT_INITIALIZED);
	COMPV_CHECK_CODE_ASSERT(CompVDrawing::s_WindowsMutex->lock());
	std::map<compv_window_id_t, CompVPtr<CompVWindow* > >::iterator it;
	for (it = CompVDrawing::m_sWindows.begin(); it != CompVDrawing::m_sWindows.end(); ++it) {
		CompVPtr<CompVWindow* > window = it->second;
		COMPV_CHECK_CODE_ASSERT(window->close());
	}
	CompVDrawing::m_sWindows.clear();
	COMPV_CHECK_CODE_ASSERT(CompVDrawing::s_WindowsMutex->unlock());

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

