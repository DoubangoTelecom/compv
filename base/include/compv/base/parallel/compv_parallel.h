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

COMPV_NAMESPACE_BEGIN()

class COMPV_BASE_API CompVParallel : public CompVObj
{
protected:
    CompVParallel();
public:
    virtual ~CompVParallel();
    static COMPV_ERROR_CODE init(int32_t numThreads = -1);
    static COMPV_ERROR_CODE deInit();
    static COMPV_INLINE CompVThreadDispatcherPtr threadDispatcher() {
#if defined(_COMPV_API_H_)
		// If you use this dispatcher to run internal tasks then, 'isMotherOfTheCurrentThread' will return true which
		// means these tasks won't be multithreaded (because they are already multithreded). It'll be up to you
		// to choose how to dispatch the tasks.
		// Please create your own thread dispatcher.
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("You must not use this thread dispatcher in your app. This will disable internal multithreading and give you the power to decide how to dispatch the tasks.");
#endif
        return CompVParallel::s_ThreadDisp;
    }
    static COMPV_ERROR_CODE multiThreadingEnable(CompVThreadDispatcherPtr dispatcher);
    static COMPV_ERROR_CODE multiThreadingDisable();
    static COMPV_ERROR_CODE multiThreadingSetMaxThreads(int32_t maxThreads = -1);
	static COMPV_ERROR_CODE setIntelTbbEnabled(bool enabled);
    static bool isMultiThreadingEnabled();
	static bool isIntelTbbEnabled() { return s_bIntelTbbEnabled; }
    static bool isInitialized();
    static bool isInitializing();

private:
    COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
    static CompVThreadDispatcherPtr s_ThreadDisp;
    static bool s_bInitialized;
    static bool s_bInitializing;
	static bool s_bIntelTbbEnabled;
    COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_PARALLEL_H_ */
