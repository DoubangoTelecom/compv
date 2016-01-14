#include <compv/compv_api.h>

using namespace compv;

static bool safe_func_exec = false;
static CompVObjWrapper<CompVMutex*> g_mutex;
static struct thread {
    const char* name;
    CompVObjWrapper<CompVThread*> thread_;
    CompVObjWrapper<CompVSemaphore*> sema_;
    CompVObjWrapper<CompVCondvar*> condvar_;
} threads[] = {
    { "foo", NULL, NULL },
    { "bar", NULL, NULL },
    { "comv", NULL, NULL },
    { "doubango", NULL, NULL },
    { "telecom", NULL, NULL }
};

static void safe_func(const struct thread* thread_)
{
    g_mutex->lock();
    COMPV_ASSERT(!safe_func_exec);
    COMPV_DEBUG_INFO("Begin safe func thread '%s' core=%ld", thread_->name, CompVThread::getCoreId());
    safe_func_exec = true;
    thread_->thread_->sleep(500);
    safe_func_exec = false;
    COMPV_DEBUG_INFO("End safe func thread '%s'", thread_->name);
    g_mutex->unlock();
}

static void* COMPV_STDCALL thread_start(void *arg)
{
    const struct thread* thread_ = (const struct thread*)arg;
    COMPV_DEBUG_INFO("Entering thread '%s'", thread_->name);
    COMPV_CHECK_CODE_ASSERT(thread_->condvar_->wait()); // wait until broadcat or signal is called
    COMPV_CHECK_CODE_ASSERT(thread_->sema_->decrement()); // wait until increment() is called
    safe_func(thread_); // execute thread-safe function
    COMPV_DEBUG_INFO("Exiting thread '%s'", thread_->name);
    return NULL;
}

bool TestThreads()
{
    // Create mutex
    COMPV_CHECK_CODE_ASSERT(CompVMutex::newObj(&g_mutex));
    // Create all semaphores and condvars
    for (size_t i = 0; i < sizeof(threads) / sizeof(threads[0]); ++i) {
        COMPV_CHECK_CODE_ASSERT(CompVSemaphore::newObj(&threads[i].sema_));
        COMPV_CHECK_CODE_ASSERT(CompVCondvar::newObj(&threads[i].condvar_));
    }

    // Create threads, they will hang as sema.dec() is called until sema.inc() is called
    for (size_t i = 0; i < sizeof(threads) / sizeof(threads[0]); ++i) {
        COMPV_CHECK_CODE_ASSERT(CompVThread::newObj(&threads[i].thread_, thread_start, (void*)&threads[i]));
    }

    // Press a key to continue
    getchar();

    // Release condvars and increment semaphores
    for (size_t i = 0; i < sizeof(threads) / sizeof(threads[0]); ++i) {
        COMPV_CHECK_CODE_ASSERT(threads[i].condvar_->broadcast());
        COMPV_CHECK_CODE_ASSERT(threads[i].sema_->increment());
    }

    getchar();

    // destroy all threads and semaphores
    for (size_t i = 0; i < sizeof(threads) / sizeof(threads[0]); ++i) {
        threads[i] = { NULL, NULL, NULL, NULL };
    }
    g_mutex = NULL;

    // Press a key to continue
    getchar();
    return true;
}