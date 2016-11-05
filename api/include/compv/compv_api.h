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
#include <compv/base/compv_mat.h>
#include <compv/base/compv_mem.h>
#include <compv/base/compv_obj.h>

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
