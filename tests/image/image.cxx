#include <compv/compv_api.h>
#include <tchar.h>

using namespace compv;

#define numThreads			COMPV_NUM_THREADS_SINGLE
#define enableIntrinsics	true
#define enableAsm			true
#define testingMode			true
#define cpuDisable				kCpuFlagAVX2

#define TEST_CONV			0
#define TEST_FAST			1
#define TEST_ORB			0
#define TEST_SCALE			0
#define TEST_PYRAMID		0


int _tmain(int argc, _TCHAR* argv[])
{
    CompVDebugMgr::setLevel(COMPV_DEBUG_LEVEL_INFO);
    COMPV_CHECK_CODE_ASSERT(CompVEngine::init(numThreads));
    COMPV_CHECK_CODE_ASSERT(CompVEngine::setTestingModeEnabled(testingMode));
    COMPV_CHECK_CODE_ASSERT(CompVCpu::setAsmEnabled(enableAsm));
    COMPV_CHECK_CODE_ASSERT(CompVCpu::setIntrinsicsEnabled(enableIntrinsics));
	COMPV_CHECK_CODE_ASSERT(CompVCpu::flagsDisable(cpuDisable));

#if TEST_CONV
    extern bool TestConv();
    COMPV_ASSERT(TestConv());
#endif
#if TEST_FAST
    extern bool TestFAST();
    COMPV_ASSERT(TestFAST());
#endif
#if TEST_ORB
    extern bool TestORB();
    COMPV_ASSERT(TestORB());
#endif
#if TEST_SCALE
    extern bool TestScale();
    COMPV_ASSERT(TestScale());
#endif
#if TEST_PYRAMID
    extern bool TestPyramid();
    COMPV_ASSERT(TestPyramid());
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

