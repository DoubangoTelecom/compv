/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_PRALLEL_ASYNCTASK11_H_)
#define _COMPV_BASE_PRALLEL_ASYNCTASK11_H_

#include "compv/base/compv_config.h"
#if COMPV_CPP11
#include "compv/base/parallel/compv_thread.h"
#include "compv/base/parallel/compv_mutex.h"
#include "compv/base/parallel/compv_semaphore.h"
#include "compv/base/compv_obj.h"
#include "compv/base/compv_common.h"

#include <map>
#include <vector>
#include <functional>

// C++11
#if COMPV_PARALLEL_SEMA11
#	include <mutex>
#	include <condition_variable>
#	include <memory>
#endif

COMPV_NAMESPACE_BEGIN()

typedef std::pair<long/*TaskId*/, long/*ChildId*/> CompVAsyncTask11Id;
typedef std::vector<CompVAsyncTask11Id > CompVAsyncTaskIds;

#if COMPV_PARALLEL_SEMA11
class CompVSemaphore11
{
public:
    CompVSemaphore11(int count_ = 0)
        : count(count_) {}

    COMPV_INLINE COMPV_ERROR_CODE increment() {
        std::unique_lock<std::mutex> lock(mtx);
        count++;
        cv.notify_one();
        return COMPV_ERROR_CODE_S_OK;
    }

    COMPV_INLINE COMPV_ERROR_CODE decrement() {
        std::unique_lock<std::mutex> lock(mtx);
        while (count == 0) {
            cv.wait(lock);
        }
        count--;
        return COMPV_ERROR_CODE_S_OK;
    }
private:
    std::mutex mtx;
    std::condition_variable cv;
    int count;
};
#endif

COMPV_OBJECT_DECLARE_PTRS(AsyncTask11)

class COMPV_BASE_API CompVAsyncTask11 : public CompVObj
{
protected:
    CompVAsyncTask11();
public:
    virtual ~CompVAsyncTask11();
	COMPV_OBJECT_GET_ID(CompVAsyncTask11);

    COMPV_ERROR_CODE start();
    COMPV_ERROR_CODE setAffinity(compv_core_id_t core_id);
    COMPV_ERROR_CODE invoke(std::function<void()> fFunc, long* tokenId = nullptr);
    COMPV_ERROR_CODE waitOne(long childId, uint64_t u_timeout = 86400000/* 1 day */);
    COMPV_ERROR_CODE stop();
    COMPV_INLINE CompVThreadPtr thread() {
        return m_Thread;
    }
    COMPV_INLINE compv_core_id_t coreId() const {
        return m_iCoreId;
    }

    static long uniqueId();
    static COMPV_ERROR_CODE newObj(CompVAsyncTask11PtrPtr asyncTask);

private:
    static void* COMPV_STDCALL run(void *pcArg);

private:
    COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
    CompVThreadPtr m_Thread;
#if COMPV_PARALLEL_SEMA11
    std::shared_ptr<CompVSemaphore11> m_SemRun;
    std::shared_ptr<CompVSemaphore11> m_SemExec;
#else
    CompVSemaphorePtr m_SemRun;
	CompVSemaphorePtr m_SemExec;
#endif
    CompVMutexPtr m_MutexChilds;
	std::map<long, std::function<void()> > m_Childs;
    bool m_bStarted;
    compv_core_id_t m_iCoreId;
	COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* COMPV_CPP11 */

#endif /* _COMPV_BASE_PRALLEL_ASYNCTASK11_H_ */
