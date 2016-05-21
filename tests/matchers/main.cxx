#include <compv/compv_api.h>
#if COMPV_OS_WINDOWS
#include <tchar.h>
#endif

using namespace compv;

#define numThreads				COMPV_NUM_THREADS_SINGLE
#define enableIntrinsics		true
#define enableAsm				true
#define enableMathFixedPoint	true
#define enableTestingMode		true

#define cpuDisable				kCpuFlagNone

#define TEST_HAMMING			0
#define TEST_BRUTEFORCE			1


#if COMPV_OS_WINDOWS
int _tmain(int argc, _TCHAR* argv[])
#else
int main(int argc, char** argv)
#endif
{
    CompVDebugMgr::setLevel(COMPV_DEBUG_LEVEL_INFO);
    COMPV_CHECK_CODE_ASSERT(CompVEngine::init(numThreads));
    COMPV_CHECK_CODE_ASSERT(CompVEngine::setTestingModeEnabled(enableTestingMode));
    COMPV_CHECK_CODE_ASSERT(CompVEngine::setMathFixedPointEnabled(enableMathFixedPoint));
    COMPV_CHECK_CODE_ASSERT(CompVCpu::setAsmEnabled(enableAsm));
    COMPV_CHECK_CODE_ASSERT(CompVCpu::setIntrinsicsEnabled(enableIntrinsics));
    COMPV_CHECK_CODE_ASSERT(CompVCpu::flagsDisable(cpuDisable));

#if TEST_HAMMING
    extern bool TestHamming();
    COMPV_ASSERT(TestHamming());
#endif
#if TEST_BRUTEFORCE
    extern bool TestBruteForce();
    COMPV_ASSERT(TestBruteForce());
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

