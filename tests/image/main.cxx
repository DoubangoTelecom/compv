#include "../tests_common.h"

using namespace compv;

#define TAG_TEST_IMAGE "TestImage"

#define TEST_CHROMA_CONV			1

/* Entry point function */
compv_main()
{
	{
		COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
		COMPV_CHECK_CODE_BAIL(err = tests_init());

#if TEST_CHROMA_CONV
		extern COMPV_ERROR_CODE chroma_conv();
		COMPV_CHECK_CODE_BAIL(err = chroma_conv(), "Chroma conversion test failed");
#endif

	bail:
		COMPV_CHECK_CODE_ASSERT(err, "Something went wrong!!");
		COMPV_CHECK_CODE_ASSERT(err = tests_deInit());
	}

	COMPV_DEBUG_CHECK_FOR_MEMORY_LEAKS();

	compv_main_return(0);
}