/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/ui/compv_ui.h"
#include "compv/base/compv_base.h"

#if HAVE_GLFW
#include <GLFW/glfw3.h>
#endif /* HAVE_GLFW */

COMPV_NAMESPACE_BEGIN()

bool CompVUI::s_bInitialized = false;
bool CompVUI::s_bLoopRunning = false;
std::map<compv_window_id_t, CompVPtr<CompVWindow* > > CompVUI::m_sWindows;
CompVPtr<CompVMutex* > CompVUI::m_sWindowsMutex = NULL;
CompVPtr<CompVThread* > CompVUI::m_sWorkerThread = NULL;

#if HAVE_GLFW
static void GLFW_ErrorCallback(int error, const char* description);
#endif /* HAVE_GLFW */

CompVUI::CompVUI()
{

}

CompVUI::~CompVUI()
{

}

COMPV_ERROR_CODE CompVUI::init()
{
	/* Base */
	COMPV_CHECK_CODE_RETURN(CompVBase::init());

	if (CompVUI::s_bInitialized) {
		return COMPV_ERROR_CODE_S_OK;
	}
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	COMPV_DEBUG_INFO("Initializing UI module (v %s)...", COMPV_VERSION_STRING);

	COMPV_CHECK_CODE_BAIL(err = CompVMutex::newObj(&CompVUI::m_sWindowsMutex));

	/* GLFW */
#if HAVE_GLFW
	glfwSetErrorCallback(GLFW_ErrorCallback);
	if (!glfwInit()) {
		COMPV_DEBUG_ERROR_EX("GLFW", "glfwInit failed");
		COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_E_GLFW);
	}
	COMPV_DEBUG_INFO_EX("GLFW", "glfwInit succeeded");
#else
	COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_S_OK);
	COMPV_DEBUG_INFO("GLFW not supported on the current platform");
#endif /* HAVE_GLFW */

bail:
	if (COMPV_ERROR_CODE_IS_OK(err)) {
		/* Everything is OK */
		CompVUI::s_bInitialized = true;
		COMPV_DEBUG_INFO("UI module initialized");
		return COMPV_ERROR_CODE_S_OK;
	}
	else {
		/* Something went wrong */
#if HAVE_GLFW
		glfwTerminate();
		glfwSetErrorCallback(NULL);
#endif /* HAVE_GLFW */
	}

	return err;
}

COMPV_ERROR_CODE CompVUI::registerWindow(CompVPtr<CompVWindow* > window)
{
	COMPV_CHECK_EXP_RETURN(!isInitialized(), COMPV_ERROR_CODE_E_NOT_INITIALIZED);
	COMPV_CHECK_EXP_RETURN(!window, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	COMPV_CHECK_CODE_ASSERT(CompVUI::m_sWindowsMutex->lock());
	CompVUI::m_sWindows.insert(std::pair<compv_window_id_t, CompVPtr<CompVWindow* > >(window->getId(), window));
	COMPV_CHECK_CODE_ASSERT(CompVUI::m_sWindowsMutex->unlock());

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVUI::unregisterWindow(CompVPtr<CompVWindow* > window)
{
	COMPV_CHECK_CODE_RETURN(CompVUI::unregisterWindow(window->getId()));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVUI::unregisterWindow(compv_window_id_t windowId)
{
	COMPV_CHECK_EXP_RETURN(!isInitialized(), COMPV_ERROR_CODE_E_NOT_INITIALIZED);

	COMPV_CHECK_CODE_ASSERT(CompVUI::m_sWindowsMutex->lock());
	CompVUI::m_sWindows.erase(windowId);
	COMPV_CHECK_CODE_ASSERT(CompVUI::m_sWindowsMutex->unlock());

	return COMPV_ERROR_CODE_S_OK;
}

size_t CompVUI::windowsCount()
{
	COMPV_CHECK_CODE_ASSERT(CompVUI::m_sWindowsMutex->lock());
	size_t count = CompVUI::m_sWindows.size();
	COMPV_CHECK_CODE_ASSERT(CompVUI::m_sWindowsMutex->unlock());
	return count;
}

COMPV_ERROR_CODE CompVUI::runLoop(void *(COMPV_STDCALL *WorkerThread) (void *) /*= NULL*/, void *userData /*= NULL*/)
{
	COMPV_CHECK_EXP_RETURN(!isInitialized(), COMPV_ERROR_CODE_E_NOT_INITIALIZED);
	if (CompVUI::isLoopRunning()) {
		return COMPV_ERROR_CODE_S_OK;
	}
#   if COMPV_OS_APPLE
    if (!pthread_main_np()) {
        COMPV_DEBUG_WARN("MacOS: Runnin even loop outside main thread");
    }
#   endif /* COMPV_OS_APPLE */
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	CompVUI::s_bLoopRunning = true;
	compv_thread_id_t eventLoopThreadId = CompVThread::getIdCurrent();

	// Run worker thread (s_bLoopRunning must be equal to true)
	if (WorkerThread) {
		COMPV_CHECK_CODE_BAIL(err = CompVThread::newObj(&CompVUI::m_sWorkerThread, WorkerThread, userData));
	}
    
    // http://forum.lwjgl.org/index.php?topic=5836.0
	// Context creation and using rules:
	//There are 3 concepts you need to be aware of : (A)window / context creation(B) running the event loop that dispatches events for the window and(C) making the context current in a thread.
	//	- On Windows, (A)and(B) must happen on the same thread.It doesn't have to be the main thread. (C) can happen on any thread.
	//	- On Linux, you can have(A), (B)and(C) on any thread.
	//	- On Mac OS X, (A)and(B) must happen on the same thread, that must also be the main thread(thread 0).Again, (C)can happen on any thread.
    COMPV_DEBUG_INFO("Running event loop on thread with id = %ld", (long)eventLoopThreadId);

	while (CompVUI::s_bLoopRunning) {
        if (CompVUI::windowsCount() == 0) {
            COMPV_DEBUG_INFO("No active windows in the event loop... breaking the loop")
            goto bail;
        }
#if HAVE_GLFW
		glfwWaitEvents();
#else
		CompVThread::sleep(1);
#endif /* HAVE_GLFW */
	}

bail:
	CompVUI::s_bLoopRunning = false;
	CompVUI::m_sWorkerThread = NULL;
	return err;
}

COMPV_ERROR_CODE CompVUI::breakLoop()
{
	COMPV_CHECK_EXP_RETURN(!isInitialized(), COMPV_ERROR_CODE_E_NOT_INITIALIZED);
	
#if HAVE_GLFW
	// Setting "s_bLoopRunning" will break the loop which means we'll no longer poll the events.
	// Instead of doing this, just close all the windows. Closing the windows will break the loop 
	// and set 's_bLoopRunning' value to false
#else
	 // Required to break the sleep loop
	 CompVUI::s_bLoopRunning = false;
#endif
    
    COMPV_CHECK_CODE_ASSERT(CompVUI::m_sWindowsMutex->lock());
    std::map<compv_window_id_t, CompVPtr<CompVWindow* > >::iterator it;
    for (it = CompVUI::m_sWindows.begin(); it != CompVUI::m_sWindows.end(); ++it) {
        CompVPtr<CompVWindow* > window = it->second;
        COMPV_CHECK_CODE_ASSERT(window->close());
    }
    CompVUI::m_sWindows.clear();
    COMPV_CHECK_CODE_ASSERT(CompVUI::m_sWindowsMutex->unlock());

	CompVUI::m_sWorkerThread = NULL;
    
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVUI::deInit()
{
	if (!CompVUI::s_bInitialized) {
		return COMPV_ERROR_CODE_S_OK;
	}

	COMPV_DEBUG_INFO("DeInitializing UI module (v %s)...", COMPV_VERSION_STRING);
#if HAVE_GLFW
	glfwTerminate();
	glfwSetErrorCallback(NULL);
#endif /* HAVE_GLFW */

	/* Base */
	CompVBase::deInit();

    CompVUI::breakLoop();
    
	CompVUI::m_sWindows.clear();
	CompVUI::m_sWindowsMutex = NULL;
	CompVUI::m_sWorkerThread = NULL;

	COMPV_DEBUG_INFO("UI module deinitialized");

	CompVUI::s_bInitialized = false;

	return COMPV_ERROR_CODE_S_OK;
}

#if HAVE_GLFW
static void GLFW_ErrorCallback(int error, const char* description)
{
	COMPV_DEBUG_ERROR_EX("GLFW", "code: %d, description: %s", error, description);
}
#endif /* HAVE_GLFW */

COMPV_NAMESPACE_END()

