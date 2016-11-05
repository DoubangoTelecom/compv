/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_PRALLEL_ASYNCTASK_H_)
#define _COMPV_BASE_PRALLEL_ASYNCTASK_H_

#include "compv/base/compv_config.h"
#if !COMPV_PARALLEL_THREADDISP11
#include "compv/base/parallel/compv_thread.h"
#include "compv/base/parallel/compv_semaphore.h"
#include "compv/base/compv_obj.h"
#include "compv/base/compv_common.h"

COMPV_NAMESPACE_BEGIN()

typedef uint64_t compv_asynctoken_id_t;

typedef struct compv_asynctoken_param_xs {
    uintptr_t pcParamPtr;
    size_t uParamSize;
}
compv_asynctoken_param_xt;

typedef COMPV_ERROR_CODE(*compv_asynctoken_f)(const struct compv_asynctoken_param_xs* pc_params);

#if !defined(COMPV_ASYNCTOKEN_ID_INVALID)
#	define COMPV_ASYNCTOKEN_ID_INVALID				-1
#endif /* COMPV_ASYNCTOKEN_ID_INVALID */

#if !defined(COMPV_ASYNCTASK_MAX_TOKEN_COUNT)
#define COMPV_ASYNCTASK_MAX_TOKEN_COUNT				(COMPV_TOKENIDX_MAX < 8 ? 8 : COMPV_TOKENIDX_MAX) // 8 is the minimum we want for anonymous IDs or for testing
#endif /* HL_ASYNCTASK_MAX_TOKEN_COUNT */

#if !defined(COMPV_ASYNCTASK_MAX_TOKEN_PARAMS_COUNT)
#define COMPV_ASYNCTASK_MAX_TOKEN_PARAMS_COUNT		16
#endif /* COMPV_ASYNCTASK_MAX_TOKEN_PARAMS_COUNT */

#define COMPV_ASYNCTOKEN_ID_IS_VALID(_id_) ((_id_) != COMPV_ASYNCTOKEN_ID_INVALID && (_id_) < COMPV_ASYNCTASK_MAX_TOKEN_COUNT)
#define COMPV_ASYNCTASK_PARAM_INDEX_IS_VALID(_id_) ((_id_) >=0 && (_id_) < COMPV_ASYNCTASK_MAX_TOKEN_PARAMS_COUNT)

typedef struct compv_asynctoken_xs {
    bool bTaken;
    bool bExecuting;
    bool bExecute;
    compv_asynctoken_f fFunc;
    compv_asynctoken_param_xt params[COMPV_ASYNCTASK_MAX_TOKEN_PARAMS_COUNT];
    int32_t iParamsCount; // number of active params
    uint64_t uTimeSchedStart;
    uint64_t uTimeSchedStop;
    uint64_t uTimeFuncExecStart;
    uint64_t uTimeFuncExecStop;
public:
    compv_asynctoken_xs() {
        bTaken = bExecuting = bExecute = false;
        iParamsCount = 0;
        fFunc = NULL;
    }
}
compv_asynctoken_xt;

#define COMPV_ASYNCTASK_PARAM_PTR_INVALID									((uintptr_t)-1)
#define COMPV_ASYNCTASK_GET_PARAM(param_ptr, type)							*((type*)(param_ptr))
#define COMPV_ASYNCTASK_GET_PARAM_ASIS(param_asis, type)					((type)((uintptr_t)((param_asis))))
#define COMPV_ASYNCTASK_GET_PARAM_REFARRAY2(param_ptr, type, w, h)			*((type (**)[w][h])(param_ptr))
#define COMPV_ASYNCTASK_GET_PARAM_REFARRAY1(param_ptr, type, w)				*((type (*)[w])(param_ptr))

#define COMPV_ASYNCTASK_SET_PARAM(param_ptr)								(uintptr_t)(&(param_ptr))
#define COMPV_ASYNCTASK_SET_PARAM_ASIS(param_asis)							((uintptr_t)((param_asis))) // Must not be more than a pointer size, we recommend using uintptr_t. If you set a param using this macro then you *must* use COMPV_ASYNCTASK_GET_PARAM_ASIS() to get it
#define COMPV_ASYNCTASK_SET_PARAM_ASISS(...)								__VA_ARGS__

#define COMPV_ASYNCTASK_SET_PARAM_NULL()									COMPV_ASYNCTASK_PARAM_PTR_INVALID

class COMPV_BASE_API CompVAsyncTask : public CompVObj
{
protected:
    CompVAsyncTask();
public:
    virtual ~CompVAsyncTask();
    virtual COMPV_INLINE const char* getObjectId() {
        return "CompVAsyncTask";
    };

    COMPV_ERROR_CODE start();
    COMPV_ERROR_CODE setAffinity(compv_core_id_t core_id);
    COMPV_ERROR_CODE tokenTake(compv_asynctoken_id_t* pi_token);
    COMPV_ERROR_CODE tokenRelease(compv_asynctoken_id_t* pi_token);
    COMPV_ERROR_CODE tokenSetParam(compv_asynctoken_id_t token_id, int32_t param_index, uintptr_t param_ptr, size_t param_size);
    COMPV_ERROR_CODE tokenSetParams(compv_asynctoken_id_t token_id, compv_asynctoken_f f_func, ...);
    COMPV_ERROR_CODE tokenSetParams2(compv_asynctoken_id_t token_id, compv_asynctoken_f f_func, va_list* ap);
    COMPV_ERROR_CODE tokenGetIdleTime(compv_asynctoken_id_t token_id, uint64_t* timeIdle);
    COMPV_ERROR_CODE execute(compv_asynctoken_id_t token_id, compv_asynctoken_f f_func, ...);
    COMPV_ERROR_CODE execute2(compv_asynctoken_id_t token_id, compv_asynctoken_f f_func, va_list* ap);
    COMPV_ERROR_CODE wait(compv_asynctoken_id_t token_id, uint64_t u_timeout = 86400000/* 1 day */);
    COMPV_ERROR_CODE stop();
    COMPV_INLINE uint64_t getTockensCount() {
        return m_iTokensCount;
    }
    COMPV_INLINE CompVPtr<CompVThread* > getThread() {
        return m_Thread;
    }
    COMPV_INLINE compv_core_id_t getCoreId() {
        return m_iCoreId;
    }

    static compv_asynctoken_id_t getUniqueTokenId();
    static COMPV_ERROR_CODE newObj(CompVPtr<CompVAsyncTask*>* asyncTask);

private:
    static void* COMPV_STDCALL run(void *pcArg);

private:
    COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
    CompVPtr<CompVThread* >m_Thread;
    CompVPtr<CompVSemaphore* >m_SemRun;
    CompVPtr<CompVSemaphore* >m_SemExec;
    struct compv_asynctoken_xs tokens[COMPV_ASYNCTASK_MAX_TOKEN_COUNT];
    COMPV_VS_DISABLE_WARNINGS_END()

    bool m_bStarted;
    compv_core_id_t m_iCoreId;

    uint64_t m_iTokensCount; // number of active tokens
};

COMPV_NAMESPACE_END()

#endif /* COMPV_PARALLEL_THREADDISP11 */

#endif /* _COMPV_BASE_PRALLEL_ASYNCTASK_H_ */
