#include <compv/compv_api.h>
#if COMPV_OS_WINDOWS
#include <tchar.h>
#endif

using namespace compv;

#define numThreads			COMPV_NUM_THREADS_SINGLE
#define enableIntrinsics	true
#define enableAsm			true
#define testingMode			true
#define cpuDisable			kCpuFlagNone

#define TEST_GAUSS_FILTER_DIM1		0
#define TEST_GAUSS_FILTER_DIM2		0
#define TEST_GAUSS_KER_DIM1_GEN		0
#define TEST_GAUSS_KER_DIM2_GEN		0
#define TEST_CONVLT_FLOAT			1
#define TEST_CONVLT_FXP				0


#if COMPV_OS_WINDOWS
int _tmain(int argc, _TCHAR* argv[])
#else
int main(int argc, char** argv)
#endif
{
    CompVDebugMgr::setLevel(COMPV_DEBUG_LEVEL_INFO);
    COMPV_CHECK_CODE_ASSERT(CompVEngine::init(numThreads));
    COMPV_CHECK_CODE_ASSERT(CompVEngine::setTestingModeEnabled(testingMode));
    COMPV_CHECK_CODE_ASSERT(CompVCpu::setAsmEnabled(enableAsm));
    COMPV_CHECK_CODE_ASSERT(CompVCpu::setIntrinsicsEnabled(enableIntrinsics));
    COMPV_CHECK_CODE_ASSERT(CompVCpu::flagsDisable(cpuDisable));

#if TEST_CONVLT_FLOAT
    extern COMPV_ERROR_CODE TestConvlt_float();
	COMPV_CHECK_CODE_ASSERT(TestConvlt_float());
#endif

#if TEST_CONVLT_FXP
    extern COMPV_ERROR_CODE TestConvlt_fxp();
    COMPV_CHECK_CODE_ASSERT(TestConvlt_fxp());
#endif

#if TEST_GAUSS_FILTER_DIM1
    extern COMPV_ERROR_CODE TestGaussFilter1();
    COMPV_CHECK_CODE_ASSERT(TestGaussFilter1());
#endif

#if TEST_GAUSS_FILTER_DIM2
    extern COMPV_ERROR_CODE TestGaussFilter2();
    COMPV_CHECK_CODE_ASSERT(TestGaussFilter2());
#endif

#if TEST_GAUSS_KER_DIM1_GEN
    extern COMPV_ERROR_CODE TestGaussKernDim1Gen();
    COMPV_CHECK_CODE_ASSERT(TestGaussKernDim1Gen());
#endif

#if TEST_GAUSS_KER_DIM2_GEN
    extern COMPV_ERROR_CODE TestGaussKernDim2Gen();
    COMPV_CHECK_CODE_ASSERT(TestGaussKernDim2Gen());
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

