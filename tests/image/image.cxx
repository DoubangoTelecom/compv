#include <compv/compv_api.h>
#include <tchar.h>

using namespace compv;

#define numThreads			COMPV_NUM_THREADS_SINGLE
#define enableIntrinsics	true
#define enableAsm			true

#define TEST_RGBA 1

#if TEST_RGBA
extern bool TestRgba();
#endif

int _tmain(int argc, _TCHAR* argv[])
{
	CompVDebugMgr::setLevel(COMPV_DEBUG_LEVEL_INFO);
	COMPV_CHECK_CODE_ASSERT(CompVEngine::init(numThreads));
	COMPV_CHECK_CODE_ASSERT(CompVCpu::setAsmEnabled(enableAsm));
	COMPV_CHECK_CODE_ASSERT(CompVCpu::setIntrinsicsEnabled(enableIntrinsics));
	COMPV_CHECK_CODE_ASSERT(CompVCpu::flagsDisable(kCpuFlagNone));

#if TEST_RGBA
	TestRgba();
#endif

	getchar();
	return 0;
}

