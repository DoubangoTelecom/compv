/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/ui/compv_ui.h"
#include "compv/base/compv_base.h"

#if HAVE_GLFW
#include <GLFW/glfw3.h>
#endif /* HAVE_GLFW */

COMPV_NAMESPACE_BEGIN()

bool CompVUI::s_bInitialized = false;

#if HAVE_GLFW
static void GLFW_ErrorCallback(int error, const char* description);
#endif /* HAVE_GLFW */

CompVUI::CompVUI()
{

}

CompVUI::~CompVUI()
{

}

COMPV_ERROR_CODE CompVUI::init()
{
	/* Base */
	COMPV_CHECK_CODE_RETURN(CompVBase::init());

	if (CompVUI::s_bInitialized) {
		return COMPV_ERROR_CODE_S_OK;
	}
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	COMPV_DEBUG_INFO("Initializing UI module (v %s)...", COMPV_VERSION_STRING);

	/* GLFW */
#if HAVE_GLFW
	glfwSetErrorCallback(GLFW_ErrorCallback);
	if (!glfwInit()) {
		COMPV_DEBUG_ERROR_EX("GLFW", "glfwInit failed");
		COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_E_GLFW);
	}
	COMPV_DEBUG_INFO_EX("GLFW", "glfwInit succeeded");
#else
	COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_S_OK);
	COMPV_DEBUG_INFO("GLFW not supported on the current platform");
#endif /* HAVE_GLFW */

bail:
	if (COMPV_ERROR_CODE_IS_OK(err)) {
		/* Everything is OK */
		CompVUI::s_bInitialized = true;
		COMPV_DEBUG_INFO("UI module initialized");
		return COMPV_ERROR_CODE_S_OK;
	}
	else {
		/* Something went wrong */
#if HAVE_GLFW
		glfwTerminate();
		glfwSetErrorCallback(NULL);
#endif /* HAVE_GLFW */
	}

	return err;
}

COMPV_ERROR_CODE CompVUI::deInit()
{
	if (!CompVUI::s_bInitialized) {
		return COMPV_ERROR_CODE_S_OK;
	}

	COMPV_DEBUG_INFO("DeInitializing UI module (v %s)...", COMPV_VERSION_STRING);
#if HAVE_GLFW
	glfwTerminate();
	glfwSetErrorCallback(NULL);
#endif /* HAVE_GLFW */

	/* Base */
	CompVBase::deInit();

	COMPV_DEBUG_INFO("UI module deinitialized");

	return COMPV_ERROR_CODE_S_OK;
}

#if HAVE_GLFW
static void GLFW_ErrorCallback(int error, const char* description)
{
	COMPV_DEBUG_ERROR_EX("GLFW", "code: %d, description: %s", error, description);
}
#endif /* HAVE_GLFW */

COMPV_NAMESPACE_END()

