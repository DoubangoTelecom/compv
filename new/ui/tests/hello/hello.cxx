// hello.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <compv/compv_api.h>

using namespace compv;

int _tmain(int argc, _TCHAR* argv[])
{
	COMPV_ERROR_CODE err;
	COMPV_CHECK_CODE_BAIL(err = CompVInit());


bail:
	if (COMPV_ERROR_CODE_IS_NOK(err)) {
		COMPV_DEBUG_ERROR("Something went wrong!!");
	}
	getchar();
	COMPV_CHECK_CODE_ASSERT(err = CompVDeInit());
	return -1;
}

