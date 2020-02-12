/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_PRALLEL_THREAD_H_)
#define _COMPV_BASE_PRALLEL_THREAD_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_obj.h"
#include "compv/base/compv_common.h"

COMPV_NAMESPACE_BEGIN()

typedef void comp_thread_handle_t;
#if COMPV_OS_WINDOWS
typedef uintptr_t compv_thread_id_t;
#	define COMPV_THREAD_PRIORITY_LOW					THREAD_PRIORITY_LOWEST
#	define COMPV_THREAD_PRIORITY_MEDIUM					THREAD_PRIORITY_NORMAL
#	define COMPV_THREAD_PRIORITY_HIGH					THREAD_PRIORITY_HIGHEST
#	define COMPV_THREAD_PRIORITY_TIME_CRITICAL			THREAD_PRIORITY_TIME_CRITICAL
#else
extern "C" {
#	include <pthread.h>
#	include <sched.h>
#	include <unistd.h>
}
typedef pthread_t compv_thread_id_t;
#	define COMPV_THREAD_PRIORITY_LOW					sched_get_priority_min(SCHED_OTHER)
#	define COMPV_THREAD_PRIORITY_TIME_CRITICAL			sched_get_priority_max(SCHED_OTHER)
#	define COMPV_THREAD_PRIORITY_MEDIUM					((COMPV_THREAD_PRIORITY_TIME_CRITICAL - COMPV_THREAD_PRIORITY_LOW) >> 1)
#	define COMPV_THREAD_PRIORITY_HIGH					((COMPV_THREAD_PRIORITY_MEDIUM * 3) >> 1)
#endif

COMPV_OBJECT_DECLARE_PTRS(Thread)

class COMPV_BASE_API CompVThread : public CompVObj
{
protected:
    CompVThread(void *(COMPV_STDCALL *start) (void *), void *arg = NULL);
public:
    virtual ~CompVThread();
	COMPV_OBJECT_GET_ID(CompVThread);

    static void sleep(uint64_t ms);
    COMPV_ERROR_CODE setPriority(int priority);
	COMPV_INLINE compv_thread_id_t id() const { return m_Id; }
    COMPV_ERROR_CODE setAffinity(compv_core_id_t coreId);

    COMPV_ERROR_CODE join();

    static COMPV_ERROR_CODE setPriorityCurrent(int priority);
    static compv_thread_id_t idCurrent();
    static bool isEquals(compv_thread_id_t id1, compv_thread_id_t id2);
    static compv_core_id_t coreId();
    static COMPV_ERROR_CODE newObj(CompVThreadPtrPtr thread, void *(COMPV_STDCALL *start) (void *), void *arg = NULL);

private:
    comp_thread_handle_t* m_pHandle;
    compv_thread_id_t m_Id;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_PRALLEL_THREAD_H_ */
