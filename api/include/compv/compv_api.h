/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_API_H_)
#define _COMPV_API_H_ //!\\ Must not change this name, used as guard in private header files

/* Module: Base */
#include <compv/base/compv_base.h>
#include <compv/base/compv_buffer.h>
#include <compv/base/compv_debug.h>
#include <compv/base/compv_fileutils.h>
#include <compv/base/compv_mat.h>
#include <compv/base/compv_mem.h>
#include <compv/base/compv_obj.h>

#include <compv/base/android/compv_android_native_activity.h>

#include <compv/base/image/compv_image.h>
#include <compv/base/image/compv_image_decoder.h>

#include <compv/base/parallel/compv_asynctask.h>
#include <compv/base/parallel/compv_asynctask11.h>
#include <compv/base/parallel/compv_condvar.h>
#include <compv/base/parallel/compv_mutex.h>
#include <compv/base/parallel/compv_semaphore.h>
#include <compv/base/parallel/compv_thread.h>
#include <compv/base/parallel/compv_threaddisp.h>
#include <compv/base/parallel/compv_threaddisp11.h>

/* Module: Drawing */
#include <compv/drawing/compv_canvas.h>
#include <compv/drawing/compv_drawing.h>
#include <compv/drawing/compv_mvp.h>
#include <compv/drawing/compv_surface.h>

/* Android native activity entry point */

#if COMPV_OS_ANDROID && !defined(COMPV_ANDROID_NATIVE_ACTIVITY_ENTRY_POINT)
#	define COMPV_ANDROID_NATIVE_ACTIVITY_ENTRY_POINT
void ANativeActivity_onCreate(ANativeActivity* activity, void* savedState, size_t savedStateSize) { 
	ANativeActivity_onCreatePriv(activity, savedState, savedStateSize); 
}
#endif // COMPV_OS_ANDROID

/* main definition */
#if COMPV_OS_WINDOWS
#	include <tchar.h>
#	define compv_main() int _tmain(int argc, _TCHAR* argv[])
#	define compv_main_return(code) return (code)
#elif COMPV_OS_ANDROID
#	define compv_main() void android_main(struct android_app* state)
#	define compv_main_return(code) 
#else
#	define compv_main() int main(int argc, char** argv)
#	define compv_main_return(code) return (code)
#endif

COMPV_NAMESPACE_BEGIN()
COMPV_GCC_DISABLE_WARNINGS_BEGIN("-Wunused-function")

static COMPV_ERROR_CODE CompVInit()
{
	COMPV_CHECK_CODE_RETURN(CompVBase::init());
	COMPV_CHECK_CODE_RETURN(CompVDrawing::init());
	return COMPV_ERROR_CODE_S_OK;
}

static COMPV_ERROR_CODE CompVDeInit()
{
	COMPV_CHECK_CODE_ASSERT(CompVBase::deInit());
	COMPV_CHECK_CODE_RETURN(CompVDrawing::deInit());
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_GCC_DISABLE_WARNINGS_END()
COMPV_NAMESPACE_END()

#endif /* _COMPV_API_H_ */
