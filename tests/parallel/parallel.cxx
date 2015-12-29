#include <compv/compv_api.h>
#include <tchar.h>

using namespace compv;

static struct thread {
	const char* name;
	CompVObjWrapper<CompVThread*> thread_;
	CompVObjWrapper<CompVSemaphore*> sema_;
} threads[] = {
	{ "foo", NULL, NULL },
	{ "bar", NULL, NULL },
	{ "comv", NULL, NULL },
	{ "doubango", NULL, NULL },
	{ "telecom", NULL, NULL }
};

static void* COMPV_STDCALL thread_start(void *arg)
{
	const struct thread* thread_ = (const struct thread*)arg;
	COMPV_DEBUG_INFO("Entering thread '%s'", thread_->name);
	COMPV_CHECK_CODE_ASSERT(thread_->sema_->decrement()); // wait until increment() is called
	COMPV_DEBUG_INFO("Exiting thread '%s'", thread_->name);
	return NULL;
}

int _tmain(int argc, _TCHAR* argv[])
{
	// Create all semaphores
	for (size_t i = 0; i < sizeof(threads) / sizeof(threads[0]); ++i) {
		COMPV_CHECK_CODE_ASSERT(CompVSemaphore::newObj(&threads[i].sema_));
	}

	// Create threads, they will hang as sema.dec() is called until sema.inc() is called
	for (size_t i = 0; i < sizeof(threads) / sizeof(threads[0]); ++i) {
		COMPV_CHECK_CODE_ASSERT(CompVThread::newObj(&threads[i].thread_, thread_start, (void*)&threads[i]));
	}

	// Press a key to continue
	getchar();
	
	// Increment semaphores
	for (size_t i = 0; i < sizeof(threads) / sizeof(threads[0]); ++i) {
		COMPV_CHECK_CODE_ASSERT(threads[i].sema_->increment());
	}

	getchar();

	// destroy all threads and semaphores
	for (size_t i = 0; i < sizeof(threads) / sizeof(threads[0]); ++i) {
		threads[i] = { NULL, NULL, NULL };
	}

	// Press a key to continue
	getchar();

	return 0;
}

