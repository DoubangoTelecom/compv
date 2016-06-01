#include <compv/compv_api.h>
#include <tchar.h>

#define numThreads			COMPV_NUM_THREADS_SINGLE

#define enableIntrinsics	true
#define enableAsm			true
#define testingMode			true
#define mathTrigFast		true

using namespace compv;

#define TEST_MAX				0
#define TEST_MIN				0
#define TEST_CLIP3				0
#define TEST_CLIP2				0
#define TEST_SINCOS_P32			0 // 3.2 precision, theta within IR
#define TEST_SINCOS_2PI_P32		0 // 3.2 precision, theta within [0, 2*PI]
#define TEST_EIGEN				1

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
    COMPV_CHECK_CODE_ASSERT(CompVCpu::flagsDisable(kCpuFlagNone));

#if TEST_MAX
    extern bool TestMax();
    COMPV_ASSERT(TestMax());
#endif
#if TEST_MIN
    extern bool TestMin();
    COMPV_ASSERT(TestMin());
#endif
#if TEST_CLIP3
    extern bool TestClip3();
    COMPV_ASSERT(TestClip3());
#endif
#if TEST_CLIP2
    extern bool TestClip2();
    COMPV_ASSERT(TestClip2());
#endif
#if TEST_SINCOS_P32
    extern bool TestSinCosP32(bool thetaWithinZeroPiTimes2 = false);
    COMPV_ASSERT(TestSinCosP32(false));
#endif
#if TEST_SINCOS_2PI_P32
    extern bool TestSinCosP32(bool thetaWithinZeroPiTimes2 = false);
    COMPV_ASSERT(TestSinCosP32(true));
#endif
#if TEST_EIGEN
    extern bool TestEigen();
    COMPV_ASSERT(TestEigen());
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

