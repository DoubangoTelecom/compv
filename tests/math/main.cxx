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
#define TEST_EIGEN				0
#define TEST_SVD				0
#define TEST_PSI				1 // Moore–Penrose pseudoinverse

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
	extern COMPV_ERROR_CODE TestMax();
	COMPV_CHECK_CODE_ASSERT(TestMax());
#endif
#if TEST_MIN
	extern COMPV_ERROR_CODE TestMin();
    COMPV_ASSERT(TestMin());
#endif
#if TEST_CLIP3
	extern COMPV_ERROR_CODE TestClip3();
	COMPV_CHECK_CODE_ASSERT(TestClip3());
#endif
#if TEST_CLIP2
	extern COMPV_ERROR_CODE TestClip2();
	COMPV_CHECK_CODE_ASSERT(TestClip2());
#endif
#if TEST_SINCOS_P32
	extern COMPV_ERROR_CODE TestSinCosP32(bool thetaWithinZeroPiTimes2 = false);
	COMPV_CHECK_CODE_ASSERT(TestSinCosP32(false));
#endif
#if TEST_SINCOS_2PI_P32
	extern COMPV_ERROR_CODE TestSinCosP32(bool thetaWithinZeroPiTimes2 = false);
	COMPV_CHECK_CODE_ASSERT(TestSinCosP32(true));
#endif
#if TEST_EIGEN
	extern COMPV_ERROR_CODE TestEigen();
    COMPV_CHECK_CODE_ASSERT(TestEigen());
#endif
#if TEST_SVD
	extern COMPV_ERROR_CODE TestSVD();
	COMPV_CHECK_CODE_ASSERT(TestSVD());
#endif
#if TEST_PSI
	extern COMPV_ERROR_CODE TestPseudoInverse();
	COMPV_CHECK_CODE_ASSERT(TestPseudoInverse());
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

