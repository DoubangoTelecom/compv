/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
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

COMPV_NAMESPACE_BEGIN()

class COMPV_BASE_API CompVRunnable
{
protected:
	CompVRunnable();
public:
	virtual ~CompVRunnable();

	virtual COMPV_ERROR_CODE start();
	virtual COMPV_ERROR_CODE running() = 0;
	virtual COMPV_INLINE bool isRunning() {
		return (m_bRunning && m_ptrThread);
	}

private:
	static void* COMPV_STDCALL workerThread(void* arg);

private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
	bool m_bRunning;
	CompVThreadPtr m_ptrThread;
	COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_PARALLEL_RUNNABLE_H_ */
