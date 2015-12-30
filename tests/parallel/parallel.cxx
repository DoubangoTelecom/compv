#include <compv/compv_api.h>
#include <tchar.h>

extern bool TestThreads();
extern bool TestAsyncTasks0();
extern bool TestAsyncTasks1();

#define TEST_THREADS		0
#define TEST_ASYNCTASKS		1

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

	return 0;
}

