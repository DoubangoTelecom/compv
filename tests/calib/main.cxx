#include <compv/compv_api.h>
#include <tchar.h>

#define numThreads			COMPV_NUM_THREADS_SINGLE

#define enableIntrinsics	true
#define enableAsm			true
#define testingMode			true
#define mathTrigFast		true
#define cpuDisable			kCpuFlagAVX

using namespace compv;

#define TEST_HOMOGRAPHY				1

#if COMPV_OS_WINDOWS
int _tmain(int argc, _TCHAR* argv[])
#else
int main()
#endif
{
    CompVDebugMgr::setLevel(COMPV_DEBUG_LEVEL_INFO);
    COMPV_CHECK_CODE_ASSERT(CompVEngine::init(numThreads));
    COMPV_CHECK_CODE_ASSERT(CompVEngine::setTestingModeEnabled(testingMode));
    COMPV_CHECK_CODE_ASSERT(CompVEngine::setMathTrigFastEnabled(mathTrigFast));
    COMPV_CHECK_CODE_ASSERT(CompVCpu::setAsmEnabled(enableAsm));
    COMPV_CHECK_CODE_ASSERT(CompVCpu::setIntrinsicsEnabled(enableIntrinsics));
	COMPV_CHECK_CODE_ASSERT(CompVCpu::flagsDisable(cpuDisable));

#if TEST_HOMOGRAPHY
	extern COMPV_ERROR_CODE TestHomography();
	COMPV_CHECK_CODE_ASSERT(TestHomography());
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

