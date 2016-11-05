/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_PRALLEL_ASYNCTASK11_H_)
#define _COMPV_BASE_PRALLEL_ASYNCTASK11_H_

#include "compv/base/compv_config.h"
#if COMPV_PARALLEL_THREADDISP11
#include "compv/base/parallel/compv_thread.h"
#include "compv/base/parallel/compv_mutex.h"
#include "compv/base/parallel/compv_semaphore.h"
#include "compv/base/compv_obj.h"
#include "compv/base/compv_common.h"

#include <vector>
#include <functional>

// C++11
#if COMPV_PARALLEL_SEMA11
#	include <mutex>
#	include <condition_variable>
#	include <memory>
#endif

COMPV_NAMESPACE_BEGIN()

#if !defined (COMPV_ASYNCTASK11_CHAIN_ENABLED)
#	define COMPV_ASYNCTASK11_CHAIN_ENABLED	0
#endif /* COMPV_ASYNCTASK11_CHAIN_ENABLED */

// Maximum number of mt functions you can have on the callstack for each thread
// For example, if A, B, C, D... are mt functions and have a single thread then, A->B->C->D forms 4 tokens.
// This number is per thread wich means the total tokens will be "COMPV_ASYNCTASK11_MAX_TOKEN_COUNT * numThreads"
// This means we can have up to 64 mt functions on our callstack for each thread
#if !defined(COMPV_ASYNCTASK11_MAX_TOKEN_COUNT)
#	if COMPV_ASYNCTASK11_CHAIN_ENABLED
#		define COMPV_ASYNCTASK11_MAX_TOKEN_COUNT				16
#	else
#		define COMPV_ASYNCTASK11_MAX_TOKEN_COUNT				1
#	endif
#endif /* COMPV_ASYNCTASK11_MAX_TOKEN_COUNT */

struct CompVAsyncTaskId {
	uint64_t uTaskId;
	uint64_t uTokenId;
public:
	CompVAsyncTaskId() : uTaskId(0), uTokenId(0) { }
	CompVAsyncTaskId(uint64_t taskId, uint64_t tokenId) : uTaskId(taskId), uTokenId(tokenId) { }
};
typedef std::vector<CompVAsyncTaskId> CompVAsyncTaskIds;

struct CompVAsyncToken {
	bool bExecute;
	std::function<COMPV_ERROR_CODE()> fFunc;
public:
	void init() {
		fFunc = nullptr;
		bExecute = false;
	}
};

#if COMPV_PARALLEL_SEMA11
class CompVSemaphore11 {
public:
	CompVSemaphore11(int count_ = 0)
		: count(count_) {}

	COMPV_INLINE COMPV_ERROR_CODE increment(){
		std::unique_lock<std::mutex> lock(mtx);
		count++;
		cv.notify_one();
		return COMPV_ERROR_CODE_S_OK;
	}

	COMPV_INLINE COMPV_ERROR_CODE decrement() {
		std::unique_lock<std::mutex> lock(mtx);
		while (count == 0){
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

class COMPV_BASE_API CompVAsyncTask11 : public CompVObj
{
protected:
	CompVAsyncTask11();
public:
	virtual ~CompVAsyncTask11();
	virtual COMPV_INLINE const char* getObjectId() {
		return "CompVAsyncTask11";
	};

	COMPV_ERROR_CODE start();
	COMPV_ERROR_CODE setAffinity(compv_core_id_t core_id);
	COMPV_ERROR_CODE invoke(std::function<COMPV_ERROR_CODE()> fFunc, uint64_t *tokenId = NULL);
	COMPV_ERROR_CODE waitAll(uint64_t u_timeout = 86400000/* 1 day */);
	COMPV_ERROR_CODE waitOne(uint64_t tokenId, uint64_t u_timeout = 86400000/* 1 day */);
	COMPV_ERROR_CODE stop();
	COMPV_INLINE CompVPtr<CompVThread* > getThread() {
		return m_Thread;
	}
	COMPV_INLINE compv_core_id_t getCoreId() {
		return m_iCoreId;
	}

	static uint64_t getUniqueTokenId();
	static COMPV_ERROR_CODE newObj(CompVPtr<CompVAsyncTask11*>* asyncTask);

private:
	static void* COMPV_STDCALL run(void *pcArg);

private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
	CompVPtr<CompVThread* >m_Thread;
#if COMPV_PARALLEL_SEMA11
	std::shared_ptr<CompVSemaphore11> m_SemRun;
	std::shared_ptr<CompVSemaphore11> m_SemExec;
#else
	CompVPtr<CompVSemaphore* >m_SemRun;
	CompVPtr<CompVSemaphore* >m_SemExec;
#endif
#if COMPV_ASYNCTASK11_CHAIN_ENABLED
	CompVPtr<CompVMutex* >m_MutexTokens;
#endif
	CompVAsyncToken m_Tokens[COMPV_ASYNCTASK11_MAX_TOKEN_COUNT];
	COMPV_VS_DISABLE_WARNINGS_END()

	bool m_bStarted;
	compv_core_id_t m_iCoreId;
};

typedef CompVPtr<CompVAsyncTask11* > CompVAsyncTask11Ptr;

COMPV_NAMESPACE_END()

#endif /* COMPV_PARALLEL_THREADDISP11 */

#endif /* _COMPV_BASE_PRALLEL_ASYNCTASK11_H_ */
