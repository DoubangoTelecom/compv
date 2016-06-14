/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_PRALLEL_ASYNCTASK11_H_)
#define _COMPV_PRALLEL_ASYNCTASK11_H_

#include "compv/compv_config.h"
#include "compv/parallel/compv_thread.h"
#include "compv/parallel/compv_semaphore.h"
#include "compv/compv_obj.h"
#include "compv/compv_common.h"

#include <map>

COMPV_NAMESPACE_BEGIN()

struct CompVAsyncToken;

class COMPV_API CompVAsyncTask11 : public CompVObj
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
	COMPV_DISABLE_WARNINGS_BEGIN(4251 4267)
	CompVPtr<CompVThread* >m_Thread;
	CompVPtr<CompVSemaphore* >m_SemRun;
	CompVPtr<CompVSemaphore* >m_SemExec;
	std::map<uint64_t, CompVAsyncToken> m_Tokens;
	COMPV_DISABLE_WARNINGS_END()

	bool m_bStarted;
	compv_core_id_t m_iCoreId;
};

struct CompVAsyncToken {
	uint64_t uId;
	std::function<COMPV_ERROR_CODE()> fFunc;
public:
	CompVAsyncToken(std::function<COMPV_ERROR_CODE()> f) {
		fFunc = f;
		uId = CompVAsyncTask11::getUniqueTokenId();
	}
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_PRALLEL_ASYNCTASK11_H_ */
