/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_PRALLEL_THREADDISP_H_)
#define _COMPV_BASE_PRALLEL_THREADDISP_H_

#include "compv/base/compv_config.h"
#if !COMPV_PARALLEL_THREADDISP11
#include "compv/base/compv_obj.h"
#include "compv/base/compv_common.h"
#include "compv/base/parallel/compv_asynctask.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_BASE_API CompVThreadDispatcher : public CompVObj
{
protected:
    CompVThreadDispatcher(int32_t numThreads);
public:
    virtual ~CompVThreadDispatcher();
    virtual COMPV_INLINE const char* getObjectId() {
        return "CompVThreadDispatcher";
    };
    COMPV_INLINE int32_t getThreadsCount() {
        return m_nTasksCount;
    }

    COMPV_ERROR_CODE execute(uint32_t threadIdx, compv_asynctoken_id_t tokenId, compv_asynctoken_f f_func, ...);
    COMPV_ERROR_CODE wait(uint32_t threadIdx, compv_asynctoken_id_t tokenId, uint64_t u_timeout = 86400000/* 1 day */);
    COMPV_ERROR_CODE getIdleTime(uint32_t threadIdx, compv_asynctoken_id_t tokenId, uint64_t* timeIdle);
    uint32_t getThreadIdxForNextToCurrentCore();
    uint32_t getThreadIdxCurrent();
    bool isMotherOfTheCurrentThread();
    int32_t guessNumThreadsDividingAcrossY(int xcount, int ycount, int minSamplesPerThread);

    static COMPV_ERROR_CODE newObj(CompVPtr<CompVThreadDispatcher*>* disp, int32_t numThreads = -1);

private:
    CompVPtr<CompVAsyncTask *>* m_pTasks;
    int32_t m_nTasksCount;
    bool m_bValid;
};

COMPV_NAMESPACE_END()

#endif /* COMPV_PARALLEL_THREADDISP11 */

#endif /* _COMPV_BASE_PRALLEL_THREADDISP_H_ */
