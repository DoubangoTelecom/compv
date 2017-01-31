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

#define TEST_CONV				1
#define TEST_FAST				0
#define TEST_ORB				0
#define TEST_CANNY				0
#define TEST_HOUGHSTD			0
#define TEST_SCALE				0
#define TEST_PYRAMID			0


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

#if TEST_CONV
    extern COMPV_ERROR_CODE TestConv();
    COMPV_CHECK_CODE_ASSERT(TestConv());
#endif
#if TEST_FAST
    extern COMPV_ERROR_CODE TestFAST();
    COMPV_CHECK_CODE_ASSERT(TestFAST());
#endif
#if TEST_ORB
    extern COMPV_ERROR_CODE TestORB();
    COMPV_CHECK_CODE_ASSERT(TestORB());
#endif
#if TEST_CANNY
    extern COMPV_ERROR_CODE TestCanny();
    COMPV_CHECK_CODE_ASSERT(TestCanny());
#endif
#if TEST_HOUGHSTD
    extern COMPV_ERROR_CODE TestHoughStd();
    COMPV_CHECK_CODE_ASSERT(TestHoughStd());
#endif
#if TEST_SCALE
    extern COMPV_ERROR_CODE TestScale();
    COMPV_CHECK_CODE_ASSERT(TestScale());
#endif
#if TEST_PYRAMID
    extern COMPV_ERROR_CODE TestPyramid();
    COMPV_CHECK_CODE_ASSERT(TestPyramid());
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

