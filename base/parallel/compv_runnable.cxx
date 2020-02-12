/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/parallel/compv_runnable.h"
#include "compv/base/compv_base.h"
#include "compv/base/compv_debug.h"

#define COMPV_THIS_CLASSNAME	"CompVRunnable"

COMPV_NAMESPACE_BEGIN()

CompVRunnable::CompVRunnable(CompVRunnableCallbackOnRunning cbOnRunning)
	: m_bRunning(false)
	, m_cbOnRunning(cbOnRunning)
{

}

CompVRunnable::~CompVRunnable()
{
	COMPV_CHECK_CODE_NOP(stop());
}

COMPV_ERROR_CODE CompVRunnable::start()
{
	if (isRunning()) {
		return COMPV_ERROR_CODE_S_OK;
	}
	COMPV_CHECK_EXP_RETURN(m_ptrThread, COMPV_ERROR_CODE_E_INVALID_STATE, "Thread must be null");
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

COMPV_ERROR_CODE CompVRunnable::stop()
{
	m_bRunning = false;
	if (m_ptrThread) {
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Joining the thread");
		COMPV_CHECK_CODE_RETURN(m_ptrThread->join());
		m_ptrThread = nullptr;
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVRunnable::newObj(CompVRunnablePtrPtr runnable, CompVRunnableCallbackOnRunning cbOnRunning)
{
	COMPV_CHECK_EXP_RETURN(!CompVBase::isInitialized(), COMPV_ERROR_CODE_E_NOT_INITIALIZED);
	COMPV_CHECK_EXP_RETURN(!runnable || !cbOnRunning, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVRunnablePtr runnable_ = new CompVRunnable(cbOnRunning);
	COMPV_CHECK_EXP_RETURN(!runnable_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	*runnable = runnable_;
	return COMPV_ERROR_CODE_S_OK;
}
	
void* COMPV_STDCALL CompVRunnable::workerThread(void* arg)
{
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Entering worker thread...");
	CompVRunnable* This = reinterpret_cast<CompVRunnable*>(arg);
	This->m_bRunning = true;
	COMPV_CHECK_CODE_NOP(This->m_cbOnRunning());
	This->m_bRunning = false;
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Exiting worker thread...");
	return NULL;
}

COMPV_NAMESPACE_END()
