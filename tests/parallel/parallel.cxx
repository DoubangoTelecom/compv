#include <compv/compv_api.h>
#include <tchar.h>

extern bool TestThreads();
extern bool TestAsyncTasks();

#define TEST_THREADS		0
#define TEST_ASYNCTASKS		1

int _tmain(int argc, _TCHAR* argv[])
{
#if TEST_THREADS
	COMPV_ASSERT(TestThreads());
#endif

#if TEST_ASYNCTASKS
	COMPV_ASSERT(TestAsyncTasks());
#endif

	return 0;
}

