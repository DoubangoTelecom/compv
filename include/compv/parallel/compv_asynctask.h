/* Copyright (C) 2016 Doubango Telecom <https://www.doubango.org>
*
* This file is part of Open Source ComputerVision (a.k.a CompV) project.
* Source code hosted at https://github.com/DoubangoTelecom/compv
* Website hosted at http://compv.org
*
* CompV is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* CompV is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with CompV.
*/
#if !defined(_COMPV_PRALLEL_ASYNCTASK_H_)
#define _COMPV_PRALLEL_ASYNCTASK_H_

#include "compv/compv_config.h"
#include "compv/parallel/compv_thread.h"
#include "compv/parallel/compv_semaphore.h"
#include "compv/compv_obj.h"
#include "compv/compv_common.h"

COMPV_NAMESPACE_BEGIN()

typedef uint64_t compv_asynctoken_id_t;

typedef struct compv_asynctoken_param_xs {
	const void* pcParamPtr;
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
}
compv_asynctoken_xt;

#define COMPV_ASYNCTASK_PARAM_PTR_INVALID									((const void*)-1)
#define COMPV_ASYNCTASK_GET_PARAM(param_ptr, type)							*((type*)(param_ptr))
#define COMPV_ASYNCTASK_GET_PARAM_PTR(param_ptr, type)						((type)(param_ptr))
#define COMPV_ASYNCTASK_GET_PARAM_SCALAR(param_scalar, type)				((type)(param_scalar))
#define COMPV_ASYNCTASK_GET_PARAM_STATIC_ARRAY(param_ptr, type, w, h)		*((type (**)[w][h])(param_ptr))

#define COMPV_ASYNCTASK_SET_PARAM(param_ptr)								(const void*)(&(param_ptr))
#define COMPV_ASYNCTASK_SET_PARAM_PTR(param_ptr)							(const void*)((param_ptr))
#define COMPV_ASYNCTASK_SET_PARAM_SCALAR(param_scalar)						(const void*)((param_scalar)) // Must not be more than a pointer size, we recommend using int32_t. If you set a param using this macro then you *must* use COMPV_ASYNCTASK_GET_PARAM_SCALAR() to get it
#define COMPV_ASYNCTASK_SET_PARAM_NULL()									COMPV_ASYNCTASK_PARAM_PTR_INVALID

class COMPV_API CompVAsyncTask : public CompVObj
{
protected:
	CompVAsyncTask();
public:
	virtual ~CompVAsyncTask();
	virtual COMPV_INLINE const char* getObjectId() { return "CompVAsyncTask"; };

	COMPV_ERROR_CODE start();
	COMPV_ERROR_CODE setAffinity(vcomp_core_id_t core_id);
	COMPV_ERROR_CODE tokenTake(compv_asynctoken_id_t* pi_token);
	COMPV_ERROR_CODE tokenRelease(compv_asynctoken_id_t* pi_token);
	COMPV_ERROR_CODE tokenSetParam(compv_asynctoken_id_t token_id, int32_t param_index, const void* param_ptr, size_t param_size);
	COMPV_ERROR_CODE tokenSetParams(compv_asynctoken_id_t token_id, compv_asynctoken_f f_func, ...);
	COMPV_ERROR_CODE tokenSetParams2(compv_asynctoken_id_t token_id, compv_asynctoken_f f_func, va_list* ap);
	COMPV_ERROR_CODE execute(compv_asynctoken_id_t token_id, compv_asynctoken_f f_func, ...);
	COMPV_ERROR_CODE execute2(compv_asynctoken_id_t token_id, compv_asynctoken_f f_func, va_list* ap);
	COMPV_ERROR_CODE wait(compv_asynctoken_id_t token_id, uint64_t u_timeout = 86400000/* 1 day */);
	COMPV_ERROR_CODE stop();
	COMPV_INLINE uint64_t getTockensCount() { return m_iTokensCount; }

	static compv_asynctoken_id_t getUniqueTokenId();
	static COMPV_ERROR_CODE newObj(CompVObjWrapper<CompVAsyncTask*>* asyncTask);

private:
	static void* COMPV_STDCALL run(void *pcArg);

private:
	COMPV_DISABLE_WARNINGS_BEGIN(4251 4267)
	CompVObjWrapper<CompVThread* >m_Thread;
	CompVObjWrapper<CompVSemaphore* >m_SemRun;
	CompVObjWrapper<CompVSemaphore* >m_SemExec;
	COMPV_DISABLE_WARNINGS_END()

	bool m_bStarted;
	vcomp_core_id_t m_iCoreId;

	struct compv_asynctoken_xs tokens[COMPV_ASYNCTASK_MAX_TOKEN_COUNT];
	uint64_t m_iTokensCount; // number of active tokens
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_PRALLEL_ASYNCTASK_H_ */
