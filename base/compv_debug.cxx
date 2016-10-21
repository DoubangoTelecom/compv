/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

const void* CompVDebugMgr::s_pcArgData = NULL;
CompVDebugFuncPtr CompVDebugMgr::s_pfInfo = NULL;
CompVDebugFuncPtr CompVDebugMgr::s_pfWarn = NULL;
CompVDebugFuncPtr CompVDebugMgr::s_pfError = NULL;
CompVDebugFuncPtr CompVDebugMgr::s_pfFatal = NULL;
#if defined(DEBUG) || defined(_DEBUG)
COMPV_DEBUG_LEVEL CompVDebugMgr::s_eLevel = COMPV_DEBUG_LEVEL_INFO;
#else
COMPV_DEBUG_LEVEL CompVDebugMgr::s_eLevel = COMPV_DEBUG_LEVEL_WARN;
#endif

CompVDebugMgr::CompVDebugMgr()
{

}

CompVDebugMgr::~CompVDebugMgr()
{

}

void CompVDebugMgr::setArgData(const void* pcArgData)
{
    CompVDebugMgr::s_pcArgData = pcArgData;
}

const void* CompVDebugMgr::getArgData()
{
    return CompVDebugMgr::s_pcArgData;
}

void CompVDebugMgr::setInfoFuncPtr(CompVDebugFuncPtr funPtr)
{
    CompVDebugMgr::s_pfInfo = funPtr;
}

CompVDebugFuncPtr CompVDebugMgr::getInfoFuncPtr()
{
    return CompVDebugMgr::s_pfInfo;
}

void CompVDebugMgr::setWarnFuncPtr(CompVDebugFuncPtr funPtr)
{
    CompVDebugMgr::s_pfWarn = funPtr;
}

CompVDebugFuncPtr CompVDebugMgr::getWarnFuncPtr()
{
    return CompVDebugMgr::s_pfWarn;
}

void CompVDebugMgr::setErrorFuncPtr(CompVDebugFuncPtr funPtr)
{
    CompVDebugMgr::s_pfError = funPtr;
}

CompVDebugFuncPtr CompVDebugMgr::getErrorFuncPtr()
{
    return CompVDebugMgr::s_pfError;
}

void CompVDebugMgr::setFatalFuncPtr(CompVDebugFuncPtr funPtr)
{
    CompVDebugMgr::s_pfFatal = funPtr;
}

CompVDebugFuncPtr CompVDebugMgr::getFatalFuncPtr()
{
    return CompVDebugMgr::s_pfFatal;
}

COMPV_DEBUG_LEVEL CompVDebugMgr::getLevel()
{
    return CompVDebugMgr::s_eLevel;
}

void CompVDebugMgr::setLevel(COMPV_DEBUG_LEVEL eLevel)
{
    CompVDebugMgr::s_eLevel = eLevel;
}

COMPV_NAMESPACE_END()
