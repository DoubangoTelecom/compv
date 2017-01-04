#include "../tests/tests_common.h"

using namespace compv;

#define TAG_UNITTESTS "UnitTests"

#define UNITTEST_FEATURE_FAST	1
#define UNITTEST_CHROMA_CONV	0

#define kAsmTrue true
#define kAsmFalse false
#define kIntrinTrue true
#define kIntrinFalse false
#define kMtTrue true
#define kMtFalse false
#define kFpTrue true
#define kFpFalse false

#define disableSSE() (kCpuFlagSSE | kCpuFlagSSE2 | kCpuFlagSSE3 | kCpuFlagSSSE3 | kCpuFlagSSE41 | kCpuFlagSSE42 | kCpuFlagSSE4a)
#define disableAVX() (kCpuFlagAVX | kCpuFlagAVX2)
#define disableNEON() (kCpuFlagARM_NEON | kCpuFlagARM_VFPv4)

static const struct compv_unittest_option {
	uint64_t disabledCpuFlags;
	bool enableAsm;
	bool enableIntrin;
	bool enabledMultithreading;
	bool enableFixedPointMath;
}
COMPV_UNITTEST_OPTIONS[] =
{
	kCpuFlagAll, kAsmFalse, kIntrinFalse, kMtFalse, kFpTrue, // Pure C++, single-threaded, fxp
	kCpuFlagAll, kAsmFalse, kIntrinFalse, kMtTrue, kFpTrue, // Pure C++, multi-threaded, fxp
#if COMPV_ARCH_X86
	disableAVX(), kAsmTrue, kIntrinFalse, kMtFalse, kFpTrue, // SSE-asm, single-threaded, fxp
	disableAVX(), kAsmTrue, kIntrinFalse, kMtTrue, kFpTrue, // SSE-asm, multi-threaded, fxp
	disableAVX(), kAsmFalse, kIntrinTrue, kMtFalse, kFpTrue, // SSE-intrin, single-threaded, fxp
	disableAVX(), kAsmFalse, kIntrinTrue, kMtTrue, kFpTrue, // SSE-intrin, multi-threaded, fxp
	disableSSE(), kAsmTrue, kIntrinFalse, kMtFalse, kFpTrue, // AVX-asm, single-threaded, fxp
	disableSSE(), kAsmTrue, kIntrinFalse, kMtTrue, kFpTrue, // AVX-asm, multi-threaded, fxp
	disableSSE(), kAsmFalse, kIntrinTrue, kMtFalse, kFpTrue, // AVX-intrin, single-threaded, fxp
	disableSSE(), kAsmFalse, kIntrinTrue, kMtTrue, kFpTrue, // AVX-intrin, multi-threaded, fxp
#elif COMPV_ARCH_ARM
	kCpuFlagNone, kAsmTrue, kIntrinFalse, kMtFalse, kFpTrue, // NEON-asm, single-threaded, fxp
	kCpuFlagNone, kAsmTrue, kIntrinFalse, kMtTrue, kFpTrue, // NEON-asm, multi-threaded, fxp
	kCpuFlagNone, kAsmFalse, kIntrinTrue, kMtFalse, kFpTrue, // NEON-intrin, single-threaded, fxp
	kCpuFlagNone, kAsmFalse, kIntrinTrue, kMtTrue, kFpTrue, // NEON-intrin, multi-threaded, fxp
#endif
};
size_t COMPV_UNITTEST_OPTIONS_COUNT = sizeof(COMPV_UNITTEST_OPTIONS) / sizeof(COMPV_UNITTEST_OPTIONS[0]);

static std::string unittest_option_tostring(const compv_unittest_option* option)
{
	return
		std::string("disabledCpuFlags:") + std::string(CompVCpu::flagsAsString(option->disabledCpuFlags)) + std::string(", ")
		+ std::string("enableAsm:") + CompVBase::to_string(option->enableAsm) + std::string(", ")
		+ std::string("enableIntrin:") + CompVBase::to_string(option->enableIntrin) + std::string(", ")
		+ std::string("enabledMultithreading:") + CompVBase::to_string(option->enabledMultithreading) + std::string(", ")
		+ std::string("enableFixedPointMath:") + CompVBase::to_string(option->enableFixedPointMath) + std::string(", ");
}

compv_main()
{
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	{
		const compv_unittest_option* option;
		COMPV_CHECK_CODE_BAIL(err = compv_tests_init());

		for (size_t i = 0; i < COMPV_UNITTEST_OPTIONS_COUNT; ++i) {
			option = &COMPV_UNITTEST_OPTIONS[i];
			COMPV_DEBUG_INFO_EX(TAG_UNITTESTS, "** Unittest options: %s **\n", unittest_option_tostring(option).c_str());
			COMPV_CHECK_CODE_BAIL(err = CompVCpu::setMathFixedPointEnabled(option->enableFixedPointMath));
			COMPV_CHECK_CODE_BAIL(err = CompVCpu::setAsmEnabled(option->enableAsm));
			COMPV_CHECK_CODE_BAIL(err = CompVCpu::setIntrinsicsEnabled(option->enableIntrin));
			COMPV_CHECK_CODE_BAIL(err = CompVCpu::flagsDisable(option->disabledCpuFlags));
			if (option->enabledMultithreading) {
				COMPV_CHECK_CODE_BAIL(err = CompVParallel::multiThreadingSetMaxThreads(COMPV_NUM_THREADS_MULTI));
			}
			else {
				COMPV_CHECK_CODE_BAIL(err = CompVParallel::multiThreadingSetMaxThreads(COMPV_NUM_THREADS_SINGLE));
			}
			
#if UNITTEST_FEATURE_FAST || !defined(COMPV_TEST_LOCAL)
			extern COMPV_ERROR_CODE unittest_feature_fast();
			COMPV_CHECK_CODE_BAIL(err = unittest_feature_fast(), "FAST detection unittest failed");
#endif
#if UNITTEST_CHROMA_CONV || !defined(COMPV_TEST_LOCAL)
			extern COMPV_ERROR_CODE unittest_chroma_conv();
			COMPV_CHECK_CODE_BAIL(err = unittest_chroma_conv(), "Chroma conversion unittest failed");
#endif
		}

	bail:
		COMPV_CHECK_CODE_ASSERT(err, TAG_UNITTESTS "Something went wrong!!");
		COMPV_CHECK_CODE_ASSERT(err = compv_tests_deInit());
	}

	COMPV_DEBUG_CHECK_FOR_MEMORY_LEAKS();

	COMPV_DEBUG_INFO_EX(TAG_UNITTESTS, "************* Program ended!!! *************");

	compv_main_return(static_cast<int>(err));
}