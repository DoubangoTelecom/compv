#include "../tests/tests_common.h"

using namespace compv;

#define TAG_UNITTESTS "UnitTests"

#define UNITTEST_FEATURE_FAST	1
#define UNITTEST_CHROMA_CONV	0

#define disableSSE() (kCpuFlagSSE | kCpuFlagSSE2 | kCpuFlagSSE3 | kCpuFlagSSSE3 | kCpuFlagSSE41 | kCpuFlagSSE42 | kCpuFlagSSE4a)
#define disableAVX() (kCpuFlagAVX | kCpuFlagAVX2)
#define disableNEON() (kCpuFlagARM_NEON | kCpuFlagARM_VFPv4)
#define disableALL() kCpuFlagAll
#define disableNONE() kCpuFlagNone
#define enableALL() disableNONE()


static const bool UNITTESTS_ASM[] = { true, false };
static const bool UNITTESTS_INTRIN[] = { true, false };
static const bool UNITTESTS_FIXEDPOINT[] = { true, false };
static const int32_t UNITTESTS_MAXTHREADS[] = { COMPV_NUM_THREADS_MULTI, COMPV_NUM_THREADS_SINGLE };
static const uint64_t UNITTESTS_CPUFLAGS[] = {
	disableALL(), // cpp only
	enableALL(), // neon, avx, sse
#if COMPV_ARCH_X86
	disableSSE(), // avx only
	disableAVX(), // sse only
#endif
};

compv_main()
{
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	{
		size_t fmax;
		COMPV_CHECK_CODE_BAIL(err = compv_tests_init());

		fmax = CompVGpu::isActiveAndEnabled() ? 2 : 1; // 0 = gpu disabled, 1 = gpu enabled

		for (size_t a = 0; a < sizeof(UNITTESTS_ASM) / sizeof(UNITTESTS_ASM[0]); ++a) {
			for (size_t b = 0; b < sizeof(UNITTESTS_INTRIN) / sizeof(UNITTESTS_INTRIN[0]); ++b) {
				for (size_t c = 0; c < sizeof(UNITTESTS_FIXEDPOINT) / sizeof(UNITTESTS_FIXEDPOINT[0]); ++c) {
					for (size_t d = 0; d < sizeof(UNITTESTS_MAXTHREADS) / sizeof(UNITTESTS_MAXTHREADS[0]); ++d) {
						for (size_t e = 0; e < sizeof(UNITTESTS_CPUFLAGS) / sizeof(UNITTESTS_CPUFLAGS[0]); ++e) {
							for (size_t f = 0; f < fmax; ++f) {
								std::string unitest = std::string("asm=") + CompVBase::to_string(UNITTESTS_ASM[a])
									+ std::string(", intrin=") + CompVBase::to_string(UNITTESTS_INTRIN[b])
									+ std::string(", disabled cpuflags=") + std::string(CompVCpu::flagsAsString(UNITTESTS_CPUFLAGS[e]))
									+ std::string(", fixedpoint=") + CompVBase::to_string(UNITTESTS_FIXEDPOINT[c])
									+ std::string(", maxthreads=") + (UNITTESTS_MAXTHREADS[d] == COMPV_NUM_THREADS_MULTI ? std::string("multi") : (UNITTESTS_MAXTHREADS[d] == COMPV_NUM_THREADS_SINGLE ? std::string("single") : CompVBase::to_string(UNITTESTS_MAXTHREADS[d])))
									+ std::string(", gpgpu=") + CompVBase::to_string(f == 1);
								COMPV_DEBUG_INFO_EX(TAG_UNITTESTS, "\n******************\nUnittest options: %s\n******************\n", unitest.c_str());
								COMPV_CHECK_CODE_BAIL(err = CompVCpu::setMathFixedPointEnabled(UNITTESTS_FIXEDPOINT[c]));
								COMPV_CHECK_CODE_BAIL(err = CompVCpu::setAsmEnabled(UNITTESTS_ASM[a]));
								COMPV_CHECK_CODE_BAIL(err = CompVCpu::setIntrinsicsEnabled(UNITTESTS_INTRIN[b]));
								COMPV_CHECK_CODE_BAIL(err = CompVCpu::flagsDisable(UNITTESTS_CPUFLAGS[e]));
								COMPV_CHECK_CODE_BAIL(err = CompVParallel::multiThreadingSetMaxThreads(UNITTESTS_MAXTHREADS[d]));
								COMPV_CHECK_CODE_BAIL(err = CompVGpu::setEnabled(f == 1));
#if UNITTEST_FEATURE_FAST || !defined(COMPV_TEST_LOCAL)
								extern COMPV_ERROR_CODE unittest_feature_fast();
								COMPV_CHECK_CODE_BAIL(err = unittest_feature_fast(), "FAST detection unittest failed");
#endif
#if UNITTEST_CHROMA_CONV || !defined(COMPV_TEST_LOCAL)
								extern COMPV_ERROR_CODE unittest_chroma_conv();
								COMPV_CHECK_CODE_BAIL(err = unittest_chroma_conv(), "Chroma conversion unittest failed");
#endif
							}
						}
					}
				}
			}
		}

	bail:
		COMPV_CHECK_CODE_ASSERT(err, TAG_UNITTESTS "Something went wrong!!");
		COMPV_CHECK_CODE_ASSERT(err = compv_tests_deInit());
	}

	COMPV_DEBUG_CHECK_FOR_MEMORY_LEAKS();

	COMPV_DEBUG_INFO_EX(TAG_UNITTESTS, "************* Program ended!!! *************");

	compv_main_return(static_cast<int>(err));
}