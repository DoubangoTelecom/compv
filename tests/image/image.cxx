#include <compv/compv_api.h>
#include <tchar.h>

using namespace compv;

#define TEST_RGBA 1

#if TEST_RGBA
extern bool TestRgba();
#endif

int _tmain(int argc, _TCHAR* argv[])
{
#define numThreads COMPV_NUM_THREADS_SINGLE

	CompVDebugMgr::setLevel(COMPV_DEBUG_LEVEL_INFO);
	COMPV_CHECK_CODE_ASSERT(CompVEngine::init(numThreads));
	COMPV_CHECK_CODE_ASSERT(CompVCpu::flagsDisable(kCpuFlagAll));

#if TEST_RGBA
	TestRgba();
#endif

	getchar();
	return 0;
}

