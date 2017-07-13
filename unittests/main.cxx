#include "../tests/tests_common.h"

using namespace compv;

#define TAG_UNITTESTS "UnitTests"

#define UNITTEST_SCALE							1
#define UNITTEST_PYRAMID						0
#define UNITTEST_CONVOLUTION					0
#define UNITTEST_SOBEL							0
#define UNITTEST_CANNY							0
#define UNITTEST_HOUGHSHT						0
#define UNITTEST_HOUGHKHT						0

#define UNITTEST_FEATURE_FAST					0
#define UNITTEST_CHROMA_CONV					0
#define UNITTEST_BRUTEFORCE						0

#define UNITTEST_PATCH_MOMENTS					0

#define UNITTEST_MATH_MATRIX_OPS				0
#define UNITTEST_MATH_EIGEN_S					0
#define UNITTEST_MATH_SVD						0
#define UNITTEST_MATH_INVERSE					0 // Moore–Penrose pseudoinverse and Inverse3x3 (remove from the unittest, not stable and already part of homography)
#define UNITTEST_MATH_STATS_MSE_2D_HOMOG		0
#define UNITTEST_MATH_STATS_NORMALIZE_HARTLEY	0
#define UNITTEST_MATH_STATS_VARIANCE			0
#define UNITTEST_MATH_TRF_HOMOG_TO_CART			0 // homogeneousToCartesian2D()
#define UNITTEST_MATH_CALIB_HOMOGRAPHY			0
#define UNITTEST_MATH_DISTANCE_HAMMING			0
#define UNITTEST_MATH_HISTOGRAM					0

#define enableSSE2()	~(kCpuFlagSSE | kCpuFlagSSE2)
#define enableSSSE3()	~(kCpuFlagSSE3 | kCpuFlagSSSE3)
#define enableSSE4()	~(kCpuFlagSSE41 | kCpuFlagSSE42 | kCpuFlagSSE4a)
#define enableAVX()		~(kCpuFlagAVX | kCpuFlagAVX2)
#if COMPV_ARCH_X86
#define disableFMA()	(kCpuFlagFMA3 | kCpuFlagFMA4)
#else
#define disableFMA()	(kCpuFlagARM_NEON_FMA)
#endif
#define disableNEON()	(kCpuFlagARM_NEON)
#define disableALL()	kCpuFlagAll
#define disableNONE()	kCpuFlagNone
#define enableALL()		~kCpuFlagAll


static const bool UNITTESTS_ASM[] = { true, false };
static const bool UNITTESTS_INTRIN[] = { true, false };
static const bool UNITTESTS_FIXEDPOINT[] = { true, false };
static const int32_t UNITTESTS_MAXTHREADS[] = { COMPV_NUM_THREADS_MULTI, COMPV_NUM_THREADS_SINGLE };
static const uint64_t UNITTESTS_CPUFLAGS_DISABLED[] = {
	disableALL(), // cpp only
	enableALL(), // neon, avx, sse, fma (normal case)
#if COMPV_ARCH_X86
	enableSSE2(), // SSE, SSE2
	enableSSSE3(), // SSE3, SSSE3
	enableSSE4(), // SSE4.1, SSE4.2, SSE4.a
	enableAVX(), // AVX, AVX2
#endif
	disableFMA(), // all without fma
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
						for (size_t e = 0; e < sizeof(UNITTESTS_CPUFLAGS_DISABLED) / sizeof(UNITTESTS_CPUFLAGS_DISABLED[0]); ++e) {
							for (size_t f = 0; f < fmax; ++f) {
								std::string unitest = std::string("asm=") + CompVBase::to_string(UNITTESTS_ASM[a])
									+ std::string(", intrin=") + CompVBase::to_string(UNITTESTS_INTRIN[b])
									+ std::string(", enabled cpuflags=") + std::string(CompVCpu::flagsAsString(~UNITTESTS_CPUFLAGS_DISABLED[e]))
									+ std::string(", fixedpoint=") + CompVBase::to_string(UNITTESTS_FIXEDPOINT[c])
									+ std::string(", maxthreads=") + (UNITTESTS_MAXTHREADS[d] == COMPV_NUM_THREADS_MULTI ? std::string("multi") : (UNITTESTS_MAXTHREADS[d] == COMPV_NUM_THREADS_SINGLE ? std::string("single") : CompVBase::to_string(UNITTESTS_MAXTHREADS[d])))
									+ std::string(", gpgpu=") + CompVBase::to_string(f == 1);
								COMPV_DEBUG_INFO_EX(TAG_UNITTESTS, "\n******************\nUnittest options: %s\n******************\n", unitest.c_str());
								COMPV_CHECK_CODE_BAIL(err = CompVCpu::setMathFixedPointEnabled(UNITTESTS_FIXEDPOINT[c]));
								COMPV_CHECK_CODE_BAIL(err = CompVCpu::setAsmEnabled(UNITTESTS_ASM[a]));
								COMPV_CHECK_CODE_BAIL(err = CompVCpu::setIntrinsicsEnabled(UNITTESTS_INTRIN[b]));
								COMPV_CHECK_CODE_BAIL(err = CompVCpu::flagsDisable(UNITTESTS_CPUFLAGS_DISABLED[e]));
								COMPV_CHECK_CODE_BAIL(err = CompVParallel::multiThreadingSetMaxThreads(UNITTESTS_MAXTHREADS[d]));
								COMPV_CHECK_CODE_BAIL(err = CompVGpu::setEnabled(f == 1));
#if UNITTEST_SCALE || !defined(COMPV_TEST_LOCAL)
								extern COMPV_ERROR_CODE unittest_scale();
								COMPV_CHECK_CODE_BAIL(err = unittest_scale(), "Image scale unittest failed");
#endif
#if UNITTEST_PYRAMID || !defined(COMPV_TEST_LOCAL)
								extern COMPV_ERROR_CODE unittest_pyramid();
								COMPV_CHECK_CODE_BAIL(err = unittest_pyramid(), "Image pyramid unittest failed");
#endif
#if UNITTEST_CONVOLUTION || !defined(COMPV_TEST_LOCAL)
								extern COMPV_ERROR_CODE unittest_convlt();
								COMPV_CHECK_CODE_BAIL(err = unittest_convlt(), "Image convolution unittest failed");
#endif
#if UNITTEST_SOBEL || !defined(COMPV_TEST_LOCAL)
								extern COMPV_ERROR_CODE unittest_sobel();
								COMPV_CHECK_CODE_BAIL(err = unittest_sobel(), "Sobel unittest failed");
#endif
#if UNITTEST_CANNY || !defined(COMPV_TEST_LOCAL)
								extern COMPV_ERROR_CODE unittest_canny();
								COMPV_CHECK_CODE_BAIL(err = unittest_canny(), "Canny unittest failed");
#endif
#if UNITTEST_HOUGHSHT || !defined(COMPV_TEST_LOCAL)
								extern COMPV_ERROR_CODE unittest_houghsht();
								COMPV_CHECK_CODE_BAIL(err = unittest_houghsht(), "Houghsht unittest failed");
#endif
#if UNITTEST_HOUGHKHT || !defined(COMPV_TEST_LOCAL)
								extern COMPV_ERROR_CODE unittest_houghkht();
								COMPV_CHECK_CODE_BAIL(err = unittest_houghkht(), "Houghkht unittest failed");
#endif						
								
								
#if UNITTEST_FEATURE_FAST || !defined(COMPV_TEST_LOCAL)
								extern COMPV_ERROR_CODE unittest_feature_fast();
								COMPV_CHECK_CODE_BAIL(err = unittest_feature_fast(), "FAST detection unittest failed");
#endif
#if UNITTEST_CHROMA_CONV || !defined(COMPV_TEST_LOCAL)
								extern COMPV_ERROR_CODE unittest_chroma_conv();
								COMPV_CHECK_CODE_BAIL(err = unittest_chroma_conv(), "Chroma conversion unittest failed");
#endif
#if UNITTEST_BRUTEFORCE || !defined(COMPV_TEST_LOCAL)
								extern COMPV_ERROR_CODE unittest_mathes_bruteforce();
								COMPV_CHECK_CODE_BAIL(err = unittest_mathes_bruteforce(), "Bruteforce unittest failed");
#endif

#if UNITTEST_PATCH_MOMENTS || !defined(COMPV_TEST_LOCAL)
								extern COMPV_ERROR_CODE compv_unittest_patch_moments0110();
								COMPV_CHECK_CODE_BAIL(err = compv_unittest_patch_moments0110(), "Patch moments unittest failed");
#endif

#if UNITTEST_MATH_MATRIX_OPS || !defined(COMPV_TEST_LOCAL)
								extern COMPV_ERROR_CODE unittest_math_matrix_ops();
								COMPV_CHECK_CODE_BAIL(err = unittest_math_matrix_ops(), "Math matrix ops unittest failed");
#endif
#if UNITTEST_MATH_EIGEN_S || !defined(COMPV_TEST_LOCAL)
								extern COMPV_ERROR_CODE unittest_math_eigenS();
								COMPV_CHECK_CODE_BAIL(err = unittest_math_eigenS(), "Math eigenS unittest failed");
#endif
#if UNITTEST_MATH_SVD || !defined(COMPV_TEST_LOCAL)
								extern COMPV_ERROR_CODE unittest_math_svd();
								COMPV_CHECK_CODE_BAIL(err = unittest_math_svd(), "Math svd unittest failed");
#endif
#if UNITTEST_MATH_INVERSE || !defined(COMPV_TEST_LOCAL)
								extern COMPV_ERROR_CODE unittest_math_inverse();
								COMPV_CHECK_CODE_BAIL(err = unittest_math_inverse(), "Math inverse unittest failed");
#endif
#if UNITTEST_MATH_STATS_MSE_2D_HOMOG || !defined(COMPV_TEST_LOCAL)
								extern COMPV_ERROR_CODE unittest_math_stats_mse2D_homogeneous();
								COMPV_CHECK_CODE_BAIL(err = unittest_math_stats_mse2D_homogeneous(), "Math stats MSE 2D homo unittest failed");
#endif
#if UNITTEST_MATH_STATS_NORMALIZE_HARTLEY || !defined(COMPV_TEST_LOCAL)
								extern COMPV_ERROR_CODE unittest_math_stats_normalize2D_hartley();
								COMPV_CHECK_CODE_BAIL(err = unittest_math_stats_normalize2D_hartley(), "Math stats norm 2D hartley unittest failed");
#endif
#if UNITTEST_MATH_STATS_VARIANCE || !defined(COMPV_TEST_LOCAL)
								extern COMPV_ERROR_CODE unittest_math_stats_variance();
								COMPV_CHECK_CODE_BAIL(err = unittest_math_stats_variance(), "Math stats variance unittest failed");
#endif
#if UNITTEST_MATH_TRF_HOMOG_TO_CART || !defined(COMPV_TEST_LOCAL)
								extern COMPV_ERROR_CODE unittest_math_transform_homogeneousToCartesian2D();
								COMPV_CHECK_CODE_BAIL(err = unittest_math_transform_homogeneousToCartesian2D(), "Math transform homogeneousToCartesian2D unittest failed");
#endif

#if UNITTEST_MATH_CALIB_HOMOGRAPHY || !defined(COMPV_TEST_LOCAL)
								extern COMPV_ERROR_CODE unittest_math_calib_homography();
								COMPV_CHECK_CODE_BAIL(err = unittest_math_calib_homography(), "Math calib homography unittest failed");
#endif	
#if UNITTEST_MATH_DISTANCE_HAMMING || !defined(COMPV_TEST_LOCAL)
								extern COMPV_ERROR_CODE unittest_math_distance_hamming();
								COMPV_CHECK_CODE_BAIL(err = unittest_math_distance_hamming(), "Math hamming distance unittest failed");
#endif
#if UNITTEST_MATH_HISTOGRAM || !defined(COMPV_TEST_LOCAL)
								extern COMPV_ERROR_CODE unittest_histogram();
								COMPV_CHECK_CODE_BAIL(err = unittest_histogram(), "Math histogram unittest failed");
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