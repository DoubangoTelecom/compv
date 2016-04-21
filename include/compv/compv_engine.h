/* Copyright (C) 2016 Doubango Telecom <https://www.doubango.org>
*
* This file is part of Open Source ComputerVision (a.k.a CompV) project.
* Source code hosted at https://github.com/DoubangoTelecom/compv
* Website hosted at http://compv.org
*
* CompV is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* CompV is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with CompV.
*/
#if !defined(_COMPV_ENGINE_H_)
#define _COMPV_ENGINE_H_

#include "compv/compv_config.h"
#include "compv/compv_obj.h"
#include "compv/compv_common.h"

#include "compv/parallel/compv_threaddisp.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_API CompVEngine : public CompVObj
{
protected:
    CompVEngine();
public:
    virtual ~CompVEngine();
    static COMPV_ERROR_CODE init(int32_t numThreads = -1);
    static COMPV_ERROR_CODE deInit();
    static CompVObjWrapper<CompVThreadDispatcher* >& getThreadDispatcher();
    static COMPV_ERROR_CODE multiThreadingEnable(CompVObjWrapper<CompVThreadDispatcher* > dispatcher);
    static COMPV_ERROR_CODE multiThreadingDisable();
    static COMPV_ERROR_CODE multiThreadingSetMaxThreads(size_t maxThreads);
    static COMPV_ERROR_CODE setTestingModeEnabled(bool bTesting);
    static bool isMultiThreadingEnabled();
    static bool isInitialized();
    static bool isBigEndian();
    static bool isTestingMode();

private:
    COMPV_DISABLE_WARNINGS_BEGIN(4251 4267)
    static CompVObjWrapper<CompVThreadDispatcher *> s_ThreadDisp;
    static bool s_bInitialized;
    static bool s_bBigEndian;
    static bool s_bTesting;
    COMPV_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_ENGINE_H_ */
