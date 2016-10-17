#include <compv/compv_api.h>

#if COMPV_OS_WINDOWS
#include <tchar.h>
#endif

using namespace compv;

CompVWindowPtr window1, window2;
CompVPtr<CompVThread* > workerThread;

static void* COMPV_STDCALL WorkerThread(void* arg);

#if COMPV_OS_WINDOWS
int _tmain(int argc, _TCHAR* argv[])
#else
int main(int argc, char** argv)
#endif
{
	COMPV_ERROR_CODE err;
	
	// Init the modules
	COMPV_CHECK_CODE_BAIL(err = CompVInit());

	// Create "Hello world!" window
	COMPV_CHECK_CODE_BAIL(err = CompVWindow::newObj(&window1, 640, 480, "Hello world!"));
    COMPV_CHECK_CODE_BAIL(err = CompVWindow::newObj(&window2, 640, 480, "Hello france!"));

	COMPV_CHECK_CODE_BAIL(err = CompVThread::newObj(&workerThread, WorkerThread, NULL));

	COMPV_CHECK_CODE_BAIL(err = CompVUI::runLoop());

bail:
	if (COMPV_ERROR_CODE_IS_NOK(err)) {
		COMPV_DEBUG_ERROR("Something went wrong!!");
	}

	// Destroy window (not required)
	window1 = NULL;
    window2 = NULL;
	
	// DeInit the modules
	COMPV_CHECK_CODE_ASSERT(err = CompVDeInit());
	// Make sure we freed all allocated memory
	COMPV_ASSERT(CompVMem::isEmpty());
	// Make sure we freed all allocated objects
	COMPV_ASSERT(CompVObj::isEmpty());
    
	return 0;
}


static void* COMPV_STDCALL WorkerThread(void* arg)
{
	while (CompVUI::isLoopRunning()) {
		COMPV_CHECK_CODE_BAIL(window1->draw());
		COMPV_CHECK_CODE_BAIL(window2->draw());
	}

bail:
	return NULL;
}
