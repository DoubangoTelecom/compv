/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_PRALLEL_THREADDISP10_H_)
#define _COMPV_BASE_PRALLEL_THREADDISP10_H_

#include "compv/base/compv_config.h"
#if !COMPV_CPP11
#include "compv/base/compv_obj.h"
#include "compv/base/compv_common.h"
#include "compv/base/parallel/compv_threaddisp.h"
#include "compv/base/parallel/compv_asynctask10.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

COMPV_OBJECT_DECLARE_PTRS(ThreadDispatcher10)

class CompVThreadDispatcher10 : public CompVThreadDispatcher
{
protected:
    CompVThreadDispatcher10(int32_t numThreads);
public:
    virtual ~CompVThreadDispatcher10();
	COMPV_OBJECT_GET_ID(CompVThreadDispatcher10);

	virtual COMPV_ERROR_CODE execute(uint32_t threadIdx, compv_asynctoken_id_t tokenId, compv_asynctoken_f f_func, ...) override /*Overrides(CompVThreadDispatcher)*/;
	virtual COMPV_ERROR_CODE wait(uint32_t threadIdx, compv_asynctoken_id_t tokenId, uint64_t u_timeout = 86400000/* 1 day */) override /*Overrides(CompVThreadDispatcher)*/;
	virtual COMPV_ERROR_CODE getIdleTime(uint32_t threadIdx, compv_asynctoken_id_t tokenId, uint64_t* timeIdle);
	virtual uint32_t getThreadIdxForNextToCurrentCore();
	virtual uint32_t getThreadIdxCurrent();
	virtual bool isMotherOfTheCurrentThread() override /*Overrides(CompVThreadDispatcher)*/;

    static COMPV_ERROR_CODE newObj(CompVThreadDispatcher10PtrPtr disp, int32_t numThreads);

private:
    CompVAsyncTask10PtrPtr m_ppTasks;
    bool m_bValid;
};

COMPV_NAMESPACE_END()

#endif /* !COMPV_CPP11 */

#endif /* _COMPV_BASE_PRALLEL_THREADDISP10_H_ */

