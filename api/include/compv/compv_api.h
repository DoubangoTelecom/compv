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
#include <compv/base/compv_bind.h>
#include <compv/base/compv_buffer.h>
#include <compv/base/compv_debug.h>
#include <compv/base/compv_fileutils.h>
#include <compv/base/compv_mat.h>
#include <compv/base/compv_mem.h>
#include <compv/base/compv_obj.h>

#include <compv/base/android/compv_android_native_activity.h>

#include <compv/base/image/compv_image.h>
#include <compv/base/image/compv_image_decoder.h>

#include <compv/base/math/compv_math.h>

#include <compv/base/parallel/compv_asynctask.h>
#include <compv/base/parallel/compv_asynctask11.h>
#include <compv/base/parallel/compv_condvar.h>
#include <compv/base/parallel/compv_mutex.h>
#include <compv/base/parallel/compv_semaphore.h>
#include <compv/base/parallel/compv_thread.h>
#include <compv/base/parallel/compv_threaddisp.h>
#include <compv/base/parallel/compv_threaddisp11.h>

#include <compv/base/time/compv_time.h>

/* Module: Camera */
#include <compv/camera/compv_camera.h>

/* Module: GL */
#include <compv/gl/compv_gl.h>

/* Module: Drawing */
#include <compv/drawing/compv_drawing.h>
#include <compv/drawing/compv_drawing_factory.h>

/* Android native activity entry point */

#if COMPV_OS_ANDROID && !defined(COMPV_ANDROID_NATIVE_ACTIVITY_ENTRY_POINT)
#	define COMPV_ANDROID_NATIVE_ACTIVITY_ENTRY_POINT
void ANativeActivity_onCreate(ANativeActivity* activity, void* savedState, size_t savedStateSize)
{
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

// Optional
static COMPV_ERROR_CODE CompVInit()
{
    COMPV_CHECK_CODE_RETURN(CompVBase::init());
    COMPV_CHECK_CODE_RETURN(CompVGL::init());
    COMPV_CHECK_CODE_RETURN(CompVCamera::init());
    COMPV_CHECK_CODE_RETURN(CompVDrawing::init());
    return COMPV_ERROR_CODE_S_OK;
}

// Optional (used for debugging to make sure all memory will be freed)
static COMPV_ERROR_CODE CompVDeInit()
{
	COMPV_CHECK_CODE_RETURN(CompVDrawing::deInit());
	COMPV_CHECK_CODE_RETURN(CompVCamera::deInit());
	COMPV_CHECK_CODE_ASSERT(CompVGL::deInit());
    COMPV_CHECK_CODE_ASSERT(CompVBase::deInit()); 
    return COMPV_ERROR_CODE_S_OK;
}

#define COMP_DEBUG_CHECK_FOR_MEMORY_LEAKS() \
	/* All allocated objects and ptrs will be freed when they go out of scoop and the reference counting value reach zero.*/ \
	/* To check for memory leak we explicitly call CompVDeInit (not required) for checking. */ \
	COMPV_CHECK_CODE_ASSERT(err = CompVDeInit()); \
	/* Make sure we freed all allocated memory */ \
	COMPV_CHECK_EXP_ASSERT(!CompVMem::isEmpty(), COMPV_ERROR_CODE_E_MEMORY_LEAK, "Memory leak: you missed some pointers allocated using CompVMem::malloc"); \
	/* Make sure we freed all allocated objects */  \
	COMPV_CHECK_EXP_ASSERT(!CompVObj::isEmpty(), COMPV_ERROR_CODE_E_MEMORY_LEAK, "Memory leak: you missed some object allocated using CompVObj::newObj");

COMPV_GCC_DISABLE_WARNINGS_END()
COMPV_NAMESPACE_END()

#endif /* _COMPV_API_H_ */
