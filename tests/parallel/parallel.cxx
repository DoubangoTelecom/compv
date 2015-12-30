#include <compv/compv_api.h>
#include <tchar.h>

extern bool TestThreads();
extern bool TestAsyncTasks0();
extern bool TestAsyncTasks1();
extern bool TestThreadDisp();

#define TEST_THREADS		0
#define TEST_ASYNCTASKS		0
#define TEST_THREADDISP		1

int _tmain(int argc, _TCHAR* argv[])
{
	compv::CompVDebugMgr::setLevel(compv::COMPV_DEBUG_LEVEL_INFO);
#if TEST_THREADS
	COMPV_ASSERT(TestThreads());
#endif

#if TEST_ASYNCTASKS
	//COMPV_ASSERT(TestAsyncTasks0());
	COMPV_ASSERT(TestAsyncTasks1());
#endif

#if TEST_THREADDISP
	COMPV_ASSERT(TestThreadDisp());
#endif

	return 0;
}

