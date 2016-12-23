/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_PRALLEL_THREADDISP11_H_)
#define _COMPV_BASE_PRALLEL_THREADDISP11_H_

#include "compv/base/compv_config.h"
#if COMPV_PARALLEL_THREADDISP11
#include "compv/base/compv_obj.h"
#include "compv/base/compv_common.h"
#include "compv/base/parallel/compv_asynctask11.h"

COMPV_NAMESPACE_BEGIN()

COMPV_OBJECT_DECLARE_PTRS(ThreadDispatcher)

class COMPV_BASE_API CompVThreadDispatcher : public CompVObj
{
protected:
    CompVThreadDispatcher(int32_t numThreads);
public:
    virtual ~CompVThreadDispatcher();
    virtual COMPV_INLINE const char* getObjectId() {
        return "CompVThreadDispatcher";
    };
    COMPV_INLINE int32_t threadsCount() { return m_nTasksCount; }

    COMPV_ERROR_CODE invoke(std::function<COMPV_ERROR_CODE()> fFunc, CompVAsyncTaskIds& taskIds);
    COMPV_ERROR_CODE wait(const CompVAsyncTaskIds& taskIds, uint64_t u_timeout = 86400000/* 1 day */);
    COMPV_ERROR_CODE waitOne(const CompVAsyncTaskId& taskId, uint64_t u_timeout = 86400000/* 1 day */);
    COMPV_ERROR_CODE waitAll(uint64_t u_timeout = 86400000/* 1 day */);
    uint32_t threadIdxCurrent();
    bool isMotherOfTheCurrentThread();
	size_t guessNumThreadsDividingAcrossY(size_t xcount, size_t ycount, size_t maxThreads, size_t minSamplesPerThread);

    static COMPV_ERROR_CODE newObj(CompVPtr<CompVThreadDispatcher*>* disp, int32_t numThreads = -1);

private:
    long nextTaskIdx();

private:
    CompVPtr<CompVAsyncTask *>* m_pTasks;
    int32_t m_nTasksCount;
    bool m_bValid;
};

COMPV_NAMESPACE_END()

#endif /* COMPV_PARALLEL_THREADDISP11 */

#endif /* _COMPV_BASE_PRALLEL_THREADDISP11_H_ */
