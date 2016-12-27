/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_PRALLEL_THREADDISP_NATIVE_H_)
#define _COMPV_BASE_PRALLEL_THREADDISP_NATIVE_H_

#include "compv/base/compv_config.h"
#if COMPV_CPP11
#include "compv/base/compv_obj.h"
#include "compv/base/compv_common.h"
#include "compv/base/parallel/compv_threaddisp.h"
#include "compv/base/parallel/compv_asynctask11.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

COMPV_OBJECT_DECLARE_PTRS(ThreadDispatcherNative)

class CompVThreadDispatcherNative : public CompVThreadDispatcher
{
protected:
	CompVThreadDispatcherNative(int32_t numThreads);
public:
	virtual ~CompVThreadDispatcherNative();
	COMPV_OBJECT_GET_ID(CompVThreadDispatcherNative);

	virtual COMPV_ERROR_CODE invoke(std::function<COMPV_ERROR_CODE()> fFunc, CompVAsyncTask11Ids& taskIds) override /*Overrides(CompVThreadDispatcher)*/;
	virtual COMPV_ERROR_CODE wait(const CompVAsyncTask11Ids& taskIds, uint64_t u_timeout = 86400000/* 1 day */) override /*Overrides(CompVThreadDispatcher)*/;
	virtual COMPV_ERROR_CODE waitOne(const CompVAsyncTask11Id& taskId, uint64_t u_timeout = 86400000/* 1 day */) override /*Overrides(CompVThreadDispatcher)*/;
	virtual COMPV_ERROR_CODE waitAll(uint64_t u_timeout = 86400000/* 1 day */);
	virtual uint32_t threadIdxCurrent();
	virtual bool isMotherOfTheCurrentThread() override /*Overrides(CompVThreadDispatcher)*/;

	static COMPV_ERROR_CODE newObj(CompVThreadDispatcherNativePtrPtr disp, int32_t numThreads);

private:
	long nextTaskIdx();

private:
	CompVAsyncTask11PtrPtr m_pTasks;
	bool m_bValid;
};

COMPV_NAMESPACE_END()

#endif /* COMPV_CPP11 */

#endif /* _COMPV_BASE_PRALLEL_THREADDISP_NATIVE_H_ */
