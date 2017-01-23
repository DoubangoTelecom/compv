/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_ENGINE_H_)
#define _COMPV_ENGINE_H_

#include "compv/compv_config.h"
#include "compv/compv_obj.h"
#include "compv/compv_common.h"

#include "compv/parallel/compv_threaddisp.h"
#include "compv/parallel/compv_threaddisp11.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_API CompVEngine : public CompVObj
{
protected:
    CompVEngine();
public:
    virtual ~CompVEngine();
    static COMPV_ERROR_CODE init(int32_t numThreads = -1);
    static COMPV_ERROR_CODE deInit();
    static COMPV_INLINE CompVPtr<CompVThreadDispatcher* > getThreadDispatcher() {
        return CompVEngine::s_ThreadDisp;
    }
    static COMPV_ERROR_CODE multiThreadingEnable(CompVPtr<CompVThreadDispatcher* > dispatcher);
    static COMPV_INLINE CompVPtr<CompVThreadDispatcher11* > getThreadDispatcher11() {
        return CompVEngine::s_ThreadDisp11;
    }
    static COMPV_ERROR_CODE multiThreadingEnable11(CompVPtr<CompVThreadDispatcher11* > dispatcher);
    static COMPV_ERROR_CODE multiThreadingDisable();
    static COMPV_ERROR_CODE multiThreadingSetMaxThreads(size_t maxThreads);
    static COMPV_ERROR_CODE setTestingModeEnabled(bool bTesting);
    static COMPV_ERROR_CODE setMathTrigFastEnabled(bool bMathTrigFast);
    static COMPV_ERROR_CODE setMathFixedPointEnabled(bool bMathFixedPoint);
    static bool isMultiThreadingEnabled();
    static bool isInitialized();
    static bool isInitializing();
    static bool isBigEndian();
    static bool isTestingMode();
    static bool isMathTrigFast();
    static bool isMathFixedPoint();

private:
    COMPV_DISABLE_WARNINGS_BEGIN(4251 4267)
    static CompVPtr<CompVThreadDispatcher *> s_ThreadDisp;
    static CompVPtr<CompVThreadDispatcher11 *> s_ThreadDisp11;
    static bool s_bInitialized;
    static bool s_bInitializing;
    static bool s_bBigEndian;
    static bool s_bTesting;
    static bool s_bMathTrigFast;
    static bool s_bMathFixedPoint;
    COMPV_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_ENGINE_H_ */
