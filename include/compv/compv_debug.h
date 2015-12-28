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
#if !defined(_COMPV_DEBUG_H_)
#define _COMPV_DEBUG_H_

#include "compv/compv_config.h"
#include "compv/compv_debug.h"
#include "compv/compv_common.h"

COMPV_NAMESPACE_BEGIN()

typedef int(*CompVDebugFuncPtr)(const void* arg, const char* fmt, ...);

class COMPV_API CompVDebugMgr
{
private:
	CompVDebugMgr();
public:
	virtual ~CompVDebugMgr();

public:
	static void setArgData(const void*);
	static const void* getArgData();
	static void setInfoFuncPtr(CompVDebugFuncPtr);
	static CompVDebugFuncPtr getInfoFuncPtr();
	static void setWarnFuncPtr(CompVDebugFuncPtr);
	static CompVDebugFuncPtr getWarnFuncPtr();
	static void setErrorFuncPtr(CompVDebugFuncPtr);
	static CompVDebugFuncPtr getErrorFuncPtr();
	static void setFatalFuncPtr(CompVDebugFuncPtr);
	static CompVDebugFuncPtr getFatalFuncPtr();
	static COMPV_DEBUG_LEVEL getLevel();
	static void setLevel(COMPV_DEBUG_LEVEL);

private:
	static const void* s_pcArgData;
	static CompVDebugFuncPtr s_pfInfo;
	static CompVDebugFuncPtr s_pfWarn;
	static CompVDebugFuncPtr s_pfError;
	static CompVDebugFuncPtr s_pfFatal;
	static COMPV_DEBUG_LEVEL s_eLevel;
};

/* INFO */
#define COMPV_DEBUG_INFO(FMT, ...)		\
	if (CompVDebugMgr::getLevel() >= COMPV_DEBUG_LEVEL_INFO) { \
		if (CompVDebugMgr::getInfoFuncPtr()) \
			CompVDebugMgr::getInfoFuncPtr()(CompVDebugMgr::getArgData(), "*[COMPV INFO]: " FMT "\n", ##__VA_ARGS__); \
				else \
			fprintf(stderr, "*[COMPV INFO]: " FMT "\n", ##__VA_ARGS__); \
			}


/* WARN */
#define COMPV_DEBUG_WARN(FMT, ...)		\
	if (CompVDebugMgr::getLevel() >= COMPV_DEBUG_LEVEL_WARN) { \
		if (CompVDebugMgr::getWarnFuncPtr()) \
			CompVDebugMgr::getWarnFuncPtr()(CompVDebugMgr::getArgData(), "**[COMPV WARN]: function: \"%s()\" \nfile: \"%s\" \nline: \"%u\" \nMSG: " FMT "\n", __FUNCTION__,  __FILE__, __LINE__, ##__VA_ARGS__); \
				else \
			fprintf(stderr, "**[COMPV WARN]: function: \"%s()\" \nfile: \"%s\" \nline: \"%u\" \nMSG: " FMT "\n", __FUNCTION__,  __FILE__, __LINE__, ##__VA_ARGS__); \
		}

/* ERROR */
#define COMPV_DEBUG_ERROR(FMT, ...) 		\
	if (CompVDebugMgr::getLevel() >= COMPV_DEBUG_LEVEL_ERROR) { \
		if (CompVDebugMgr::getErrorFuncPtr()) \
			CompVDebugMgr::getErrorFuncPtr()(CompVDebugMgr::getArgData(), "***[COMPV ERROR]: function: \"%s()\" \nfile: \"%s\" \nline: \"%u\" \nMSG: " FMT "\n", __FUNCTION__,  __FILE__, __LINE__, ##__VA_ARGS__); \
				else \
			fprintf(stderr, "***[COMPV ERROR]: function: \"%s()\" \nfile: \"%s\" \nline: \"%u\" \nMSG: " FMT "\n", __FUNCTION__,  __FILE__, __LINE__, ##__VA_ARGS__); \
		}


/* FATAL */
#define COMPV_DEBUG_FATAL(FMT, ...) 		\
	if (CompVDebugMgr::getLevel() >= COMPV_DEBUG_LEVEL_FATAL) { \
		if (CompVDebugMgr::getFatalFuncPtr()) \
			CompVDebugMgr::getFatalFuncPtr()(CompVDebugMgr::getArgData(), "****[COMPV FATAL]: function: \"%s()\" \nfile: \"%s\" \nline: \"%u\" \nMSG: " FMT "\n", __FUNCTION__,  __FILE__, __LINE__, ##__VA_ARGS__); \
				else \
			fprintf(stderr, "****[COMPV FATAL]: function: \"%s()\" \nfile: \"%s\" \nline: \"%u\" \nMSG: " FMT "\n", __FUNCTION__,  __FILE__, __LINE__, ##__VA_ARGS__); \
		}


#define COMPV_DEBUG_INFO_EX(MODULE, FMT, ...) COMPV_DEBUG_INFO("[" MODULE "] " FMT, ##__VA_ARGS__)
#define COMPV_DEBUG_WARN_EX(MODULE, FMT, ...) COMPV_DEBUG_WARN("[" MODULE "] " FMT, ##__VA_ARGS__)
#define COMPV_DEBUG_ERROR_EX(MODULE, FMT, ...) COMPV_DEBUG_ERROR("[" MODULE "] " FMT, ##__VA_ARGS__)
#define COMPV_DEBUG_FATAL_EX(MODULE, FMT, ...) COMPV_DEBUG_FATAL("[" MODULE "] " FMT, ##__VA_ARGS__)

COMPV_NAMESPACE_END()

#endif /* _COMPV_DEBUG_H_ */
