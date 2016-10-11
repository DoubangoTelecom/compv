/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_PARALLEL_H_)
#define _COMPV_BASE_PARALLEL_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_obj.h"
#include "compv/base/compv_common.h"

#include "compv/base/parallel/compv_threaddisp.h"
#include "compv/base/parallel/compv_threaddisp11.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_BASE_API CompVParallel : public CompVObj
{
protected:
	CompVParallel();
public:
	virtual ~CompVParallel();
	static COMPV_ERROR_CODE init(int32_t numThreads = -1);
	static COMPV_ERROR_CODE deInit();
#if COMPV_PARALLEL_THREADDISP11
	static COMPV_INLINE CompVPtr<CompVThreadDispatcher11* > getThreadDispatcher11() { return CompVParallel::s_ThreadDisp11; }
	static COMPV_ERROR_CODE multiThreadingEnable11(CompVPtr<CompVThreadDispatcher11* > dispatcher);
#else
	static COMPV_INLINE CompVPtr<CompVThreadDispatcher* > getThreadDispatcher() { return CompVParallel::s_ThreadDisp; }
	static COMPV_ERROR_CODE multiThreadingEnable(CompVPtr<CompVThreadDispatcher* > dispatcher);
#endif	
	static COMPV_ERROR_CODE multiThreadingDisable();
	static COMPV_ERROR_CODE multiThreadingSetMaxThreads(size_t maxThreads);
	static bool isMultiThreadingEnabled();
	static bool isInitialized();
	static bool isInitializing();

private:
	COMPV_DISABLE_WARNINGS_BEGIN(4251 4267)
#if COMPV_PARALLEL_THREADDISP11
	static CompVPtr<CompVThreadDispatcher11 *> s_ThreadDisp11;
#else
	static CompVPtr<CompVThreadDispatcher *> s_ThreadDisp;
#endif	
	static bool s_bInitialized;
	static bool s_bInitializing;
	COMPV_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_PARALLEL_H_ */
