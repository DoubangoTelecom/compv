/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/parallel/compv_runnable.h"
#include "compv/base/compv_debug.h"

#define COMPV_THIS_CLASSNAME	"CompVRunnable"

COMPV_NAMESPACE_BEGIN()

CompVRunnable::CompVRunnable()
	: m_bRunning(false)
{

}

CompVRunnable::~CompVRunnable()
{
	if (m_ptrThread) {
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Joining the thread");
		COMPV_CHECK_CODE_NOP(m_ptrThread->join());
	}
}

COMPV_ERROR_CODE CompVRunnable::start()
{
	if (m_ptrThread && m_bRunning) {
		return COMPV_ERROR_CODE_S_OK;
	}
	if (m_ptrThread) {
		COMPV_CHECK_CODE_RETURN(m_ptrThread->join());
	}
	COMPV_ERROR_CODE err;
	m_bRunning = true;
	COMPV_CHECK_CODE_BAIL(err = CompVThread::newObj(&m_ptrThread, CompVRunnable::workerThread, this));

bail:
	if (COMPV_ERROR_CODE_IS_NOK(err)) {
		// must  not use 'm_bRunning = COMPV_ERROR_CODE_IS_NOK(err);'
		// -> between CompVThread::newObj and this line 'm_bRunning' could be set to false
		// by 'CompVRunnable::workerThread'
		m_bRunning = false;
	}
	return err;
}
	
void* COMPV_STDCALL CompVRunnable::workerThread(void* arg)
{
	CompVRunnable* runnable = reinterpret_cast<CompVRunnable*>(arg);
	COMPV_CHECK_CODE_NOP(runnable->running());
	runnable->m_bRunning = false;
	return NULL;
}

COMPV_NAMESPACE_END()