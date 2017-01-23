/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_PRALLEL_THREADDISP_H_)
#define _COMPV_BASE_PRALLEL_THREADDISP_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_obj.h"
#include "compv/base/compv_common.h"
#include "compv/base/parallel/compv_asynctask11.h"
#include "compv/base/parallel/compv_asynctask10.h"

COMPV_NAMESPACE_BEGIN()

COMPV_OBJECT_DECLARE_PTRS(ThreadDispatcher)

class COMPV_BASE_API CompVThreadDispatcher : public CompVObj
{
protected:
	CompVThreadDispatcher(int32_t numThreads);
public:
	virtual ~CompVThreadDispatcher();
	COMPV_OBJECT_GET_ID(CompVThreadDispatcher);
	COMPV_INLINE int32_t threadsCount() { return m_nTasksCount; }
	COMPV_INLINE int32_t tasksCount() { return m_nTasksCount; }

#if COMPV_CPP11
	virtual COMPV_ERROR_CODE invoke(std::function<void()> fFunc, CompVAsyncTaskIds& taskIds) = 0;
	virtual COMPV_ERROR_CODE wait(const CompVAsyncTaskIds& taskIds, uint64_t u_timeout = 86400000/* 1 day */) = 0;
	virtual COMPV_ERROR_CODE waitOne(const CompVAsyncTask11Id& taskId, uint64_t u_timeout = 86400000/* 1 day */) = 0;
#else
	virtual COMPV_ERROR_CODE execute(uint32_t threadIdx, compv_asynctoken_id_t tokenId, compv_asynctoken_f f_func, ...) = 0;
	virtual COMPV_ERROR_CODE wait(uint32_t threadIdx, compv_asynctoken_id_t tokenId, uint64_t u_timeout = 86400000/* 1 day */) = 0;
#endif
	virtual bool isMotherOfTheCurrentThread() = 0;
	
	static size_t guessNumThreadsDividingAcrossY(size_t xcount, size_t ycount, size_t maxThreads, size_t minSamplesPerThread);

	static COMPV_ERROR_CODE newObj(CompVThreadDispatcherPtrPtr disp, int32_t numThreads = -1);

private:
	int32_t m_nTasksCount;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_PRALLEL_THREADDISP_H_ */
