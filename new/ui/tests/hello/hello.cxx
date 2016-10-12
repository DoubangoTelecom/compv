// hello.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <compv/compv_api.h>

using namespace compv;

int _tmain(int argc, _TCHAR* argv[])
{
	COMPV_ERROR_CODE err;
	CompVWindowPtr window;

	// Init the modules
	COMPV_CHECK_CODE_BAIL(err = CompVInit());

	// Create "Hello world!" window
	COMPV_CHECK_CODE_BAIL(err = CompVWindow::newObj(&window, 640, 480, "Hello world!"));

bail:
	if (COMPV_ERROR_CODE_IS_NOK(err)) {
		COMPV_DEBUG_ERROR("Something went wrong!!");
	}
	
	getchar();

	// Destroy window (not required)
	window = NULL;
	
	// DeInit the modules
	COMPV_CHECK_CODE_ASSERT(err = CompVDeInit());
	// Make sure we freed all allocated memory
	COMPV_ASSERT(CompVMem::isEmpty());
	// Make sure we freed all allocated objects
	COMPV_ASSERT(CompVObj::isEmpty());
	return -1;
}

