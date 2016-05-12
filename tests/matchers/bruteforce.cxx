#include <compv/compv_api.h>

#include "../common.h"

#define CROSS_CHECK		false
#define KNN				1
#define NORM			COMPV_BRUTEFORCE_NORM_HAMMING

bool TestBruteForce()
{
	COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
	CompVPtr<CompVMatcher *> bruteforce;
	int32_t val32;
	bool valBool;

	COMPV_CHECK_CODE_BAIL(err_ = CompVMatcher::newObj(COMPV_BRUTEFORCE_ID, &bruteforce));

	val32 = KNN;
	COMPV_CHECK_CODE_BAIL(err_ = bruteforce->set(COMPV_BRUTEFORCE_SET_INT32_KNN, &val32, sizeof(val32)));
	val32 = NORM;
	COMPV_CHECK_CODE_BAIL(err_ = bruteforce->set(COMPV_BRUTEFORCE_SET_INT32_NORM, &val32, sizeof(val32)));
	valBool = CROSS_CHECK;
	COMPV_CHECK_CODE_BAIL(err_ = bruteforce->set(COMPV_BRUTEFORCE_SET_BOOL_CROSS_CHECK, &valBool, sizeof(valBool)));

bail:
	COMPV_CHECK_CODE_ASSERT(err_);
	return COMPV_ERROR_CODE_IS_OK(err_);
}