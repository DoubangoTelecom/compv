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
#include "compv/compv_debug.h"

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
