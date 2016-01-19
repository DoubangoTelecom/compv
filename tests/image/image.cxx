#include <compv/compv_api.h>
#include <tchar.h>

using namespace compv;

#define numThreads			COMPV_NUM_THREADS_SINGLE
#define enableIntrinsics	true
#define enableAsm			true

#define TEST_CONV 1

#if TEST_CONV
extern bool TestConv();
#endif

int _tmain(int argc, _TCHAR* argv[])
{
    CompVDebugMgr::setLevel(COMPV_DEBUG_LEVEL_INFO);
    COMPV_CHECK_CODE_ASSERT(CompVEngine::init(numThreads));
    COMPV_CHECK_CODE_ASSERT(CompVCpu::setAsmEnabled(enableAsm));
    COMPV_CHECK_CODE_ASSERT(CompVCpu::setIntrinsicsEnabled(enableIntrinsics));
    COMPV_CHECK_CODE_ASSERT(CompVCpu::flagsDisable(kCpuFlagNone));

#if TEST_CONV
	TestConv();
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

