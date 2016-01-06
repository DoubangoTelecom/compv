#include <compv/compv_api.h>

using namespace compv;

static COMPV_ALIGN(32) long long_task_exec_count = 0;
static void long_task(int32_t start, int32_t end, uint8_t* ptr)
{
	COMPV_ASSERT(start <= end);
	for (int32_t i = start; i < end; ++i) {
		double d = (uint8_t)sqrt(sqrt((double)(ptr[i] * ptr[i]))) + 1;
		for (int32_t j = 0; j <= 10; ++j) {
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

bool TestThreadDisp()
{
#define THREADS_COUNT	7 // should be the number of CPUs - 1
#define WIDTH	1920
#define HEIGHT	1080
#define SIZE	(WIDTH * HEIGHT * 4)
#define TOKEN0	0
	uint8_t* data = (uint8_t*)CompVMem::mallocAligned(SIZE);
	COMPV_ASSERT(data != NULL);
	vcomp_core_id_t coreId_ = 0;
	CompVObjWrapper<CompVThreadDispatcher *> disp_;
	uint64_t timeStart, timeEnd;

	COMPV_CHECK_CODE_ASSERT(CompVThreadDispatcher::newObj(&disp_, THREADS_COUNT));

	// Start the tasks (each one has a single tocken with id = 0)
	timeStart = CompVTime::getNowMills();
#if 1 // Using async tasks
	int32_t interval = SIZE / THREADS_COUNT;
	int32_t start = 0, end = interval;
	for (int32_t treadId = 0; treadId < disp_->getThreadsCount(); ++treadId) {
		COMPV_CHECK_CODE_ASSERT(disp_->execute(treadId, treadId, task1_f,
			COMPV_ASYNCTASK_SET_PARAM_ASIS(start),
			COMPV_ASYNCTASK_SET_PARAM_ASIS(end),
			COMPV_ASYNCTASK_SET_PARAM_ASIS(data),
			COMPV_ASYNCTASK_SET_PARAM_NULL()));
		start += interval;
		end += interval;
		if (end >= SIZE) end = SIZE; // Clamp(end, SIZE)
	}
	// Wait for the end of all tasks
	for (int32_t treadId = 0; treadId < disp_->getThreadsCount(); ++treadId) {
		COMPV_CHECK_CODE_ASSERT(disp_->wait(treadId, TOKEN0));
	}
#else // not using async tasks
	long_task(0, SIZE, data);
#endif
	timeEnd = CompVTime::getNowMills();
	COMPV_DEBUG_INFO("Elapsed time =%llu, long_task_exec_count=%ld", (timeEnd - timeStart), long_task_exec_count);

	getchar();

	disp_ = NULL;

	CompVMem::freeAligned((void**)&data);

	return true;
}