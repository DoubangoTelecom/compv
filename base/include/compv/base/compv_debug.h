/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_DEBUG_H_)
#define _COMPV_BASE_DEBUG_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"

COMPV_NAMESPACE_BEGIN()

typedef int(*CompVDebugFuncPtr)(const void* arg, const char* fmt, ...);

class COMPV_BASE_API CompVDebugMgr
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
	if (compv::CompVDebugMgr::getLevel() >= compv::COMPV_DEBUG_LEVEL_INFO) { \
		if (compv::CompVDebugMgr::getInfoFuncPtr()) \
			compv::CompVDebugMgr::getInfoFuncPtr()(compv::CompVDebugMgr::getArgData(), "*[COMPV INFO]: " FMT "\n", ##__VA_ARGS__); \
				else \
			fprintf(stderr, "*[COMPV INFO]: " FMT "\n", ##__VA_ARGS__); \
			}


/* WARN */
#define COMPV_DEBUG_WARN(FMT, ...)		\
	if (compv::CompVDebugMgr::getLevel() >= compv::COMPV_DEBUG_LEVEL_WARN) { \
		if (compv::CompVDebugMgr::getWarnFuncPtr()) \
			compv::CompVDebugMgr::getWarnFuncPtr()(compv::CompVDebugMgr::getArgData(), "**[COMPV WARN]: function: \"%s()\" \nfile: \"%s\" \nline: \"%u\" \nMSG: " FMT "\n", __FUNCTION__,  __FILE__, __LINE__, ##__VA_ARGS__); \
				else \
			fprintf(stderr, "**[COMPV WARN]: function: \"%s()\" \nfile: \"%s\" \nline: \"%u\" \nMSG: " FMT "\n", __FUNCTION__,  __FILE__, __LINE__, ##__VA_ARGS__); \
		}

/* ERROR */
#define COMPV_DEBUG_ERROR(FMT, ...) 		\
	if (compv::CompVDebugMgr::getLevel() >= compv::COMPV_DEBUG_LEVEL_ERROR) { \
		if (compv::CompVDebugMgr::getErrorFuncPtr()) \
			compv::CompVDebugMgr::getErrorFuncPtr()(compv::CompVDebugMgr::getArgData(), "***[COMPV ERROR]: function: \"%s()\" \nfile: \"%s\" \nline: \"%u\" \nMSG: " FMT "\n", __FUNCTION__,  __FILE__, __LINE__, ##__VA_ARGS__); \
				else \
			fprintf(stderr, "***[COMPV ERROR]: function: \"%s()\" \nfile: \"%s\" \nline: \"%u\" \nMSG: " FMT "\n", __FUNCTION__,  __FILE__, __LINE__, ##__VA_ARGS__); \
		}


/* FATAL */
#define COMPV_DEBUG_FATAL(FMT, ...) 		\
	if (compv::CompVDebugMgr::getLevel() >= compv::COMPV_DEBUG_LEVEL_FATAL) { \
		if (compv::CompVDebugMgr::getFatalFuncPtr()) \
			compv::CompVDebugMgr::getFatalFuncPtr()(compv::CompVDebugMgr::getArgData(), "****[COMPV FATAL]: function: \"%s()\" \nfile: \"%s\" \nline: \"%u\" \nMSG: " FMT "\n", __FUNCTION__,  __FILE__, __LINE__, ##__VA_ARGS__); \
				else \
			fprintf(stderr, "****[COMPV FATAL]: function: \"%s()\" \nfile: \"%s\" \nline: \"%u\" \nMSG: " FMT "\n", __FUNCTION__,  __FILE__, __LINE__, ##__VA_ARGS__); \
		}


#define COMPV_DEBUG_INFO_EX(MODULE, FMT, ...) COMPV_DEBUG_INFO("[" MODULE "] " FMT, ##__VA_ARGS__)
#define COMPV_DEBUG_WARN_EX(MODULE, FMT, ...) COMPV_DEBUG_WARN("[" MODULE "] " FMT, ##__VA_ARGS__)
#define COMPV_DEBUG_ERROR_EX(MODULE, FMT, ...) COMPV_DEBUG_ERROR("[" MODULE "] " FMT, ##__VA_ARGS__)
#define COMPV_DEBUG_FATAL_EX(MODULE, FMT, ...) COMPV_DEBUG_FATAL("[" MODULE "] " FMT, ##__VA_ARGS__)

#define COMPV_DEBUG_INFO_CODE_ONCE(FMT, ...) do { static bool printed = false; if (!printed) { printed = true; COMPV_DEBUG_INFO("/!\\ Code in file '%s' in function '%s' starting at line #%u: " FMT, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__); } } while(0)

#define COMPV_DEBUG_INFO_CODE_NOT_TESTED()	COMPV_DEBUG_INFO_CODE_ONCE("Not tested")
#define COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED() COMPV_DEBUG_INFO_CODE_ONCE("Not optimized")
#define COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED_GPU() COMPV_DEBUG_INFO_CODE_ONCE("Not optimized for GPU")
#define COMPV_DEBUG_INFO_CODE_FOR_TESTING() COMPV_DEBUG_INFO_CODE_ONCE("Is for testing and must not be called")

#if !defined(__AVX__)
#	define COMPV_DEBUG_INFO_CHECK_AVX() COMPV_DEBUG_INFO_CODE_ONCE("May be slow because of AVX/SSE transition issues")
#else
#	define COMPV_DEBUG_INFO_CHECK_AVX() 
#endif
#if !defined(__AVX2__)
#	define COMPV_DEBUG_INFO_CHECK_AVX2() COMPV_DEBUG_INFO_CODE_ONCE("May be slow because of AVX/SSE transition issues")
#else
#	define COMPV_DEBUG_INFO_CHECK_AVX2() 
#endif
#if !defined(__FMA3__)
#	define COMPV_DEBUG_INFO_CHECK_FMA3() COMPV_DEBUG_INFO_CODE_ONCE("May be slow because of AVX/SSE transition issues")
#else
#	define COMPV_DEBUG_INFO_CHECK_FMA3() 
#endif
#if !defined(__SSE__)
#	define COMPV_DEBUG_INFO_CHECK_SSE() COMPV_DEBUG_INFO_CODE_ONCE("Not built with SSE support")
#else
#	define COMPV_DEBUG_INFO_CHECK_SSE() 
#endif
#if !defined(__SSE2__) && !(COMPV_ARCH_X64 && defined(_MSC_VER)) // SSE2 enabled on all x64 (Visual Studio)
#	define COMPV_DEBUG_INFO_CHECK_SSE2() COMPV_DEBUG_INFO_CODE_ONCE("Not built with SSE2 support")
#else
#	define COMPV_DEBUG_INFO_CHECK_SSE2() 
#endif
#if !defined(__SSE3__)
#	define COMPV_DEBUG_INFO_CHECK_SSE3() COMPV_DEBUG_INFO_CODE_ONCE("Not built with SSE3 support")
#else
#	define COMPV_DEBUG_INFO_CHECK_SSE3() 
#endif
#if !defined(__SSSE3__)
#	define COMPV_DEBUG_INFO_CHECK_SSSE3() COMPV_DEBUG_INFO_CODE_ONCE("Not built with SSSE3 support")
#else
#	define COMPV_DEBUG_INFO_CHECK_SSSE3() 
#endif
#if !defined(__SSE4_1__)
#	define COMPV_DEBUG_INFO_CHECK_SSE41() COMPV_DEBUG_INFO_CODE_ONCE("Not built with SSE41 support")
#else
#	define COMPV_DEBUG_INFO_CHECK_SSE41() 
#endif
#if !defined(__SSE4_2__)
#	define COMPV_DEBUG_INFO_CHECK_SSE42() COMPV_DEBUG_INFO_CODE_ONCE("Not built with SSE42 support")
#else
#	define COMPV_DEBUG_INFO_CHECK_SSE42() 
#endif
#if !defined(__ARM_NEON__)
#	define COMPV_DEBUG_INFO_CHECK_NEON() COMPV_DEBUG_INFO_CODE_ONCE("Not built with NEON support")
#else
#	define COMPV_DEBUG_INFO_CHECK_NEON() 
#endif


COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_DEBUG_H_ */
