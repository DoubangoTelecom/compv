#include <compv/compv_api.h>
#include <tchar.h>

using namespace compv;

#define numThreads			COMPV_NUM_THREADS_SINGLE
#define enableIntrinsics	true
#define enableAsm			true
#define testingMode			false
#define cpuDisable			kCpuFlagAVX

#define TEST_COPY			1


int _tmain(int argc, _TCHAR* argv[])
{
	CompVDebugMgr::setLevel(COMPV_DEBUG_LEVEL_INFO);
	COMPV_CHECK_CODE_ASSERT(CompVEngine::init(numThreads));
	COMPV_CHECK_CODE_ASSERT(CompVEngine::setTestingModeEnabled(testingMode));
	COMPV_CHECK_CODE_ASSERT(CompVCpu::setAsmEnabled(enableAsm));
	COMPV_CHECK_CODE_ASSERT(CompVCpu::setIntrinsicsEnabled(enableIntrinsics));
	COMPV_CHECK_CODE_ASSERT(CompVCpu::flagsDisable(cpuDisable));

#if TEST_COPY
	extern bool TestCopy();
	COMPV_ASSERT(TestCopy());
#endif

	// deInit the engine
	COMPV_CHECK_CODE_ASSERT(compv::CompVEngine::deInit());

	// Make sure we freed all allocated memory
	COMPV_ASSERT(compv::CompVMem::isEmpty());
	// Make sure we freed all allocated objects
	COMPV_ASSERT(compv::CompVObj::isEmpty());

	getchar();
	return 0;
}
