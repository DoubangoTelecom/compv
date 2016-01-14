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

class myClass
{
public:
    myClass() {}
    virtual ~myClass() {}
    COMPV_ERROR_CODE func1(const char* argCharPtr) {
        COMPV_DEBUG_INFO("func1(argCharPtr=%s)", argCharPtr);
        return COMPV_ERROR_CODE_S_OK;
    }
};

static COMPV_ERROR_CODE task0_f(const struct compv_asynctoken_param_xs* pc_params)
{
    compv_asynctoken_id_t token_ = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[0].pcParamPtr, compv_asynctoken_id_t);
    if (token_ == token0_) {
        int32_t argInt32 = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[1].pcParamPtr, int32_t);
        int64_t argInt64 = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[2].pcParamPtr, int64_t);
        const char* argCharPtr = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[3].pcParamPtr, const char*);
        CompVObjWrapper<CompVAsyncTask *>* argTask = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[4].pcParamPtr, CompVObjWrapper<CompVAsyncTask *>*);
        return func0(argInt32, argInt64, argCharPtr, *argTask);
    }
    else if (token_ == token1_) {
        myClass& class_ = COMPV_ASYNCTASK_GET_PARAM(pc_params[1].pcParamPtr, myClass);
        const char* argCharPtr = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[2].pcParamPtr, const char*);
        return class_.func1(argCharPtr);
    }

    COMPV_ASSERT(false);
    return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
}

bool TestAsyncTasks0()
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
    COMPV_CHECK_CODE_ASSERT(task_->execute(token0_, task0_f,
                                           COMPV_ASYNCTASK_SET_PARAM_ASIS(token0_),
                                           COMPV_ASYNCTASK_SET_PARAM_ASIS(argInt32),
                                           COMPV_ASYNCTASK_SET_PARAM_ASIS(argInt64),
                                           COMPV_ASYNCTASK_SET_PARAM_ASIS(argCharPtr0),
                                           COMPV_ASYNCTASK_SET_PARAM_ASIS(argTask),
                                           COMPV_ASYNCTASK_SET_PARAM_NULL()));

    // execute task1
    const char* argCharPtr1 = "Test asyncTask (token1)";
    COMPV_CHECK_CODE_ASSERT(task_->execute(token1_, task0_f,
                                           COMPV_ASYNCTASK_SET_PARAM_ASIS(token1_),
                                           COMPV_ASYNCTASK_SET_PARAM(obj_),
                                           COMPV_ASYNCTASK_SET_PARAM_ASIS(argCharPtr1),
                                           COMPV_ASYNCTASK_SET_PARAM_NULL()));

    COMPV_CHECK_CODE_ASSERT(task_->wait(token0_));
    COMPV_CHECK_CODE_ASSERT(task_->wait(token1_));

    getchar();

    task_ = NULL;

    getchar();

    return true;
}

static COMPV_ALIGN(32) long long_task_exec_count = 0;
static void long_task(int32_t start, int32_t end, uint8_t* ptr)
{
    COMPV_ASSERT(start <= end);
    for (int32_t i = start; i < end; ++i) {
        double d = (uint8_t)sqrt(sqrt((double)(ptr[i] * ptr[i]))) + 1;
        for (int32_t j = 0; j <= 100; ++j) {
            d += (uint8_t)tan((double)ptr[i]) * rand() * cos((double)ptr[i]) * sin((double)ptr[i]);
        }
        ptr[i] = (uint8_t)d;
        compv_atomic_inc(&long_task_exec_count);
    }
}

static COMPV_ERROR_CODE task1_f(const struct compv_asynctoken_param_xs* pc_params)
{
    int32_t start = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[0].pcParamPtr, int32_t);
    int32_t end = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[1].pcParamPtr, int32_t);
    uint8_t* data = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[2].pcParamPtr, uint8_t*);
    long_task(start, end, data);
    return COMPV_ERROR_CODE_S_OK;
}

bool TestAsyncTasks1()
{
#define TASKS_COUNT	7 // should be the number of CPUs - 1
#define WIDTH	1920
#define HEIGHT	1080
#define SIZE	(WIDTH * HEIGHT * 4)
#define TOKEN0	0
    uint8_t* data = (uint8_t*)CompVMem::mallocAligned(SIZE);
    COMPV_ASSERT(data != NULL);
    vcomp_core_id_t coreId_ = 0;
    CompVObjWrapper<CompVAsyncTask *> tasks_[TASKS_COUNT];
    uint64_t timeStart, timeEnd;

    // Create and start asyncTasks
    for (size_t i = 0; i < sizeof(tasks_) / sizeof(tasks_[0]); ++i) {
        COMPV_CHECK_CODE_ASSERT(CompVAsyncTask::newObj(&tasks_[i]));
        COMPV_CHECK_CODE_ASSERT(tasks_[i]->setAffinity(CompVCpu::getValidCoreId(coreId_++)));
        COMPV_CHECK_CODE_ASSERT(tasks_[i]->start());
    }
    // Start the tasks (each one has a single tocken with id = 0)
    timeStart = CompVTime::getNowMills();
#if 1 // Using async tasks
    int32_t interval = SIZE / TASKS_COUNT;
    int32_t start = 0, end = interval;
    for (size_t i = 0; i < sizeof(tasks_) / sizeof(tasks_[0]); ++i) {
        COMPV_CHECK_CODE_ASSERT(tasks_[i]->execute(TOKEN0, task1_f,
                                COMPV_ASYNCTASK_SET_PARAM_ASIS(start),
                                COMPV_ASYNCTASK_SET_PARAM_ASIS(end),
                                COMPV_ASYNCTASK_SET_PARAM_ASIS(data),
                                COMPV_ASYNCTASK_SET_PARAM_NULL()));
        start += interval;
        end += interval;
        if (end >= SIZE) {
            end = SIZE;    // Clamp(end, SIZE)
        }
    }
    // Wait for the end of all tasks
    for (size_t i = 0; i < sizeof(tasks_) / sizeof(tasks_[0]); ++i) {
        COMPV_CHECK_CODE_ASSERT(tasks_[i]->wait(TOKEN0));
    }
#else // not using async tasks
    long_task(0, SIZE, data);
#endif
    timeEnd = CompVTime::getNowMills();
    COMPV_DEBUG_INFO("Elapsed time =%llu, long_task_exec_count=%ld", (timeEnd - timeStart), long_task_exec_count);

    getchar();

    CompVMem::freeAligned((void**)&data);

    return true;
}