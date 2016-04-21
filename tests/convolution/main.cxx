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

#define TEST_GAUSS_FILTER_DIM1		1
#define TEST_GAUSS_FILTER_DIM2		0
#define TEST_GAUSS_KER_DIM1_GEN		0
#define TEST_GAUSS_KER_DIM2_GEN		0
#define TEST_CONVO					0


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

#if TEST_CONVO
    extern bool TestConvo();
    COMPV_ASSERT(TestConvo());
#endif

#if TEST_GAUSS_FILTER_DIM1
    extern bool TestGaussFilter1();
    COMPV_ASSERT(TestGaussFilter1());
#endif

#if TEST_GAUSS_FILTER_DIM2
    extern bool TestGaussFilter2();
    COMPV_ASSERT(TestGaussFilter2());
#endif

#if TEST_GAUSS_KER_DIM1_GEN
    extern bool TestGaussKernDim1Gen();
    COMPV_ASSERT(TestGaussKernDim1Gen());
#endif

#if TEST_GAUSS_KER_DIM2_GEN
    extern bool TestGaussKernDim2Gen();
    COMPV_ASSERT(TestGaussKernDim2Gen());
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

