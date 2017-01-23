#include <compv/compv_api.h>
#if COMPV_OS_WINDOWS
#include <tchar.h>
#endif

extern bool TestThreads();
extern bool TestAsyncTasks0();
extern bool TestAsyncTasks1();
extern bool TestThreadDisp();

#define TEST_THREADS		0
#define TEST_ASYNCTASKS		0
#define TEST_THREADDISP		1

#if COMPV_OS_WINDOWS
int _tmain(int argc, _TCHAR* argv[])
#else
int main(int argc, char** argv)
#endif
{
    compv::CompVDebugMgr::setLevel(compv::COMPV_DEBUG_LEVEL_INFO);
    COMPV_CHECK_CODE_ASSERT(compv::CompVEngine::init());
#if TEST_THREADS
    COMPV_ASSERT(TestThreads());
#endif

#if TEST_ASYNCTASKS
    COMPV_ASSERT(TestAsyncTasks0());
    COMPV_ASSERT(TestAsyncTasks1());
#endif

#if TEST_THREADDISP
    COMPV_ASSERT(TestThreadDisp());
#endif

    COMPV_CHECK_CODE_ASSERT(compv::CompVEngine::deInit());

    // Make sure we freed all allocated memory
    COMPV_ASSERT(compv::CompVMem::isEmpty());
    // Make sure we freed all allocated objects
    COMPV_ASSERT(compv::CompVObj::isEmpty());

    return 0;
}

