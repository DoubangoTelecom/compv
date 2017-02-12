#include "../tests/tests_common.h"

#define TAG_TEST			"UnitTestMathTransform"
#define ERR_MAX				4.7683715820312500e-07

template <typename T>
static COMPV_ERROR_CODE __math_transform_homogeneousToCartesian2D()
{
	static const struct compv_unittest_svd {
		size_t numpoints;
		const char* md5;
	}
#if COMPV_ARCH_X86
	COMPV_UNITTEST_TRF_FLOAT64[] = {
		{ 215, "28aa351d8531f8f140e51059bf0c2428" },
		{ 4, "3e7d29ad3635479a3763cc963d66c9a0" },
	},
	COMPV_UNITTEST_TRF_FLOAT32[] = {
		{ 215, "a47746c56687d18df5fe64a9abcdf570" },
		{ 4, "8bc15e9695f0e73d4b2a5cbb1ae6da3b" },
	};
#elif COMPV_ARCH_ARM
	COMPV_UNITTEST_TRF_FLOAT64[] = {
		{ 215, "b0ddd03c8ff65a55bc0ea5df38a58253" },
		{ 4, "599b6cd76fd341efeb4a63feebfff116" },
	},
	COMPV_UNITTEST_TRF_FLOAT32[] = {
		{ 215, "e37c3cf80c8d297b315ce9b42ce6877e" },
		{ 4, "6374cc4abd785713cb1dcf06bba9dd57" },
	};
#else
	COMPV_UNITTEST_TRF_FLOAT64[] = {
		{ 215, "" },
		{ 4, "" },
	},
	COMPV_UNITTEST_TRF_FLOAT32[] = {
		{ 215, "" },
		{ 4, "" },
	};
#endif

	const compv_unittest_svd* test = NULL;
	const compv_unittest_svd* tests = std::is_same<T, compv_float32_t>::value
		? COMPV_UNITTEST_TRF_FLOAT32
		: COMPV_UNITTEST_TRF_FLOAT64;

	for (size_t i = 0; i < sizeof(COMPV_UNITTEST_TRF_FLOAT64) / sizeof(COMPV_UNITTEST_TRF_FLOAT64[i]); ++i) {
		test = &tests[i];
		COMPV_DEBUG_INFO_EX(TAG_TEST, "== Trying new test: math transform homogeneousToCartesian2D -> %zu %zu ==", sizeof(T), test->numpoints);
		CompVMatPtr src;
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<T>(&src, 3, test->numpoints));
		T* x = src->ptr<T>(0);
		T* y = src->ptr<T>(1);
		T* z = src->ptr<T>(2);
		for (signed i = 0; i < static_cast<signed>(test->numpoints); ++i) {
			x[i] = static_cast<T>(((i & 1) ? i : (-i * 0.7)) + 0.5);
			y[i] = static_cast<T>((i * 0.2) + i + 0.7);
			z[i] = static_cast<T>(i*i*0.8 + 0.8);
		}

		CompVMatPtr dst;
		COMPV_CHECK_CODE_RETURN(CompVMathTransform<T>::homogeneousToCartesian2D(src, &dst));

		//COMPV_DEBUG_INFO("MD5: %s", compv_tests_md5(dst).c_str());
		COMPV_CHECK_EXP_RETURN(std::string(test->md5).compare(compv_tests_md5(dst)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "homogeneousToCartesian2D: MD5 mismatch"); // FIXME: not correct on ARM (change)

		COMPV_DEBUG_INFO_EX(TAG_TEST, "** Test OK **");
	}	

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE unittest_math_transform_homogeneousToCartesian2D()
{
	COMPV_CHECK_CODE_RETURN((__math_transform_homogeneousToCartesian2D<compv_float64_t>()));
	COMPV_CHECK_CODE_RETURN((__math_transform_homogeneousToCartesian2D<compv_float32_t>()));

	return COMPV_ERROR_CODE_S_OK;
}
