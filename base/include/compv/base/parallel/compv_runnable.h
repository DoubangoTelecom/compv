/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_PARALLEL_RUNNABLE_H_)
#define _COMPV_BASE_PARALLEL_RUNNABLE_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_obj.h"
#include "compv/base/compv_common.h"

#include "compv/base/parallel/compv_thread.h"

#include <functional>

COMPV_NAMESPACE_BEGIN()

COMPV_OBJECT_DECLARE_PTRS(Runnable)

typedef std::function<COMPV_ERROR_CODE(void)>		CompVRunnableCallbackOnRunning; // Use std::bind(cbOnRunning, args...) to add more arguments to the callback

class COMPV_BASE_API CompVRunnable : public CompVObj
{
protected:
	CompVRunnable(CompVRunnableCallbackOnRunning cbOnRunning);
public:
	virtual ~CompVRunnable();
	COMPV_OBJECT_GET_ID(CompVRunnable);

	virtual COMPV_ERROR_CODE start();
	virtual COMPV_ERROR_CODE stop();
	virtual COMPV_INLINE bool isRunning() {
		return (m_bRunning && m_ptrThread);
	}

	static COMPV_ERROR_CODE newObj(CompVRunnablePtrPtr runnable, CompVRunnableCallbackOnRunning cbOnRunning);

private:
	static void* COMPV_STDCALL workerThread(void* arg);

private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
	bool m_bRunning;
	CompVThreadPtr m_ptrThread;
	CompVRunnableCallbackOnRunning m_cbOnRunning;
	COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_PARALLEL_RUNNABLE_H_ */
