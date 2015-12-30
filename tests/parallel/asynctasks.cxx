#include <compv/compv_api.h>

using namespace compv;

compv_asynctoken_id_t token0_;
compv_asynctoken_id_t token1_;

static COMPV_ERROR_CODE func0(int32_t argInt32, int64_t argInt64, const char* argCharPtr, CompVObjWrapper<CompVAsyncTask *> argTask)
{
	COMPV_DEBUG_INFO("func0(argInt32=%d, argInt64=%lld, argCharPtr=%s)", argInt32, argInt64, argCharPtr);
	COMPV_ASSERT(argTask);
	return COMPV_ERROR_CODE_S_OK;
}

class myClass {
public:
	myClass() {}
	virtual ~myClass() {}
	COMPV_ERROR_CODE func1(const char* argCharPtr)
	{
		COMPV_DEBUG_INFO("func1(argCharPtr=%s)", argCharPtr);
		return COMPV_ERROR_CODE_S_OK;
	}
};

static COMPV_ERROR_CODE task(const struct compv_asynctoken_param_xs* pc_params)
{
	compv_asynctoken_id_t token_ = COMPV_ASYNCTASK_GET_PARAM(pc_params[0].pcParamPtr, compv_asynctoken_id_t);
	if (token_ == token0_) {
		int32_t argInt32 = COMPV_ASYNCTASK_GET_PARAM(pc_params[1].pcParamPtr, int32_t);
		int64_t argInt64 = COMPV_ASYNCTASK_GET_PARAM(pc_params[2].pcParamPtr, int64_t);
		const char* argCharPtr = COMPV_ASYNCTASK_GET_PARAM(pc_params[3].pcParamPtr, const char*);
		CompVObjWrapper<CompVAsyncTask *>* argTask = COMPV_ASYNCTASK_GET_PARAM(pc_params[4].pcParamPtr, CompVObjWrapper<CompVAsyncTask *>*);
		return func0(argInt32, argInt64, argCharPtr, *argTask);
	}
	else if (token_ == token1_) {
		myClass& class_ = COMPV_ASYNCTASK_GET_PARAM(pc_params[1].pcParamPtr, myClass);
		const char* argCharPtr = COMPV_ASYNCTASK_GET_PARAM(pc_params[2].pcParamPtr, const char*);
		return class_.func1(argCharPtr);
	}
	
	COMPV_ASSERT(false);
	return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
}

bool TestAsyncTasks()
{
	myClass obj_;
	const vcomp_core_id_t coreId_ = CompVCpu::getValidCoreId((vcomp_core_id_t)rand());
	token0_ = CompVAsyncTask::getUniqueTokenId();
	token1_ = CompVAsyncTask::getUniqueTokenId();
	CompVObjWrapper<CompVAsyncTask *> task_;

	COMPV_CHECK_CODE_ASSERT(CompVAsyncTask::newObj(&task_));
	COMPV_CHECK_CODE_ASSERT(task_->setAffinity(coreId_));
	COMPV_CHECK_CODE_ASSERT(task_->start());

	// execute task0
	int32_t argInt32 = 123456789;
	int64_t argInt64 = 987654321;
	const char* argCharPtr0 = "Test asyncTask (token0)";
	CompVObjWrapper<CompVAsyncTask *>* argTask = &task_;
	COMPV_CHECK_CODE_ASSERT(task_->execute(token0_, task,
		COMPV_ASYNCTASK_SET_PARAM_VAL(token0_),
		COMPV_ASYNCTASK_SET_PARAM_VAL(argInt32),
		COMPV_ASYNCTASK_SET_PARAM_VAL(argInt64),
		COMPV_ASYNCTASK_SET_PARAM_VAL(argCharPtr0),
		COMPV_ASYNCTASK_SET_PARAM_VAL(argTask),
		COMPV_ASYNCTASK_SET_PARAM_NULL()));

	// execute task1
	const char* argCharPtr1 = "Test asyncTask (token1)";
	COMPV_CHECK_CODE_ASSERT(task_->execute(token1_, task,
		COMPV_ASYNCTASK_SET_PARAM_VAL(token1_),
		COMPV_ASYNCTASK_SET_PARAM_VAL(obj_),
		COMPV_ASYNCTASK_SET_PARAM_VAL(argCharPtr1),
		COMPV_ASYNCTASK_SET_PARAM_NULL()));

	COMPV_CHECK_CODE_ASSERT(task_->wait(token0_));
	COMPV_CHECK_CODE_ASSERT(task_->wait(token1_));

	getchar();

	task_ = NULL;

	getchar();

	return true;
}
