/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/compv_engine.h"
#include "compv/compv_mem.h"
#include "compv/compv_cpu.h"
#include "compv/compv_debug.h"
#include "compv/math/compv_math_utils.h"
#include "compv/time/compv_time.h"
#include "compv/image/compv_image.h"
#include "compv/features/compv_feature.h"
#include "compv/matchers/compv_matcher.h"


COMPV_NAMESPACE_BEGIN()

bool CompVEngine::s_bInitialized = false;
bool CompVEngine::s_bInitializing = false;
#if defined(COMPV_OS_WINDOWS)
bool CompVEngine::s_bBigEndian = false;
#else
bool CompVEngine::s_bBigEndian = true;
#endif
bool CompVEngine::s_bTesting = false;
bool CompVEngine::s_bMathTrigFast = true;
bool CompVEngine::s_bMathFixedPoint = true;
CompVPtr<CompVThreadDispatcher *> CompVEngine::s_ThreadDisp = NULL;
CompVPtr<CompVThreadDispatcher11 *> CompVEngine::s_ThreadDisp11 = NULL;

CompVEngine::CompVEngine()
{

}

CompVEngine:: ~CompVEngine()
{

}

COMPV_ERROR_CODE CompVEngine::init(int32_t numThreads /*= -1*/)
{
    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;

    if (s_bInitialized || s_bInitializing) {
        return COMPV_ERROR_CODE_S_OK;
    }

    s_bInitializing = true;

    COMPV_DEBUG_INFO("Initializing engine (v %s)...", COMPV_VERSION_STRING);

    // Make sure sizeof(compv_scalar_t) is correct
#if defined(COMPV_ASM) || defined(COMPV_INTRINSIC)
    if (sizeof(compv_scalar_t) != sizeof(void*)) {
        COMPV_DEBUG_ERROR("sizeof(compv_scalar_t)= #%lu not equal to sizeof(void*)= #%lu", sizeof(compv_scalar_t), sizeof(void*));
        return COMPV_ERROR_CODE_E_SYSTEM;
    }
	// https://en.wikipedia.org/wiki/Single-precision_floating-point_format
	if (sizeof(compv_float32_t) != 4) {
		COMPV_DEBUG_ERROR("sizeof(compv_float32_t)= #%lu not equal to 4", sizeof(compv_float32_t));
		return COMPV_ERROR_CODE_E_SYSTEM;
	}
	// https://en.wikipedia.org/wiki/Double-precision_floating-point_format
	if (sizeof(compv_float64_t) != 8) {
		COMPV_DEBUG_ERROR("sizeof(compv_float64_t)= #%lu not equal to 8", sizeof(compv_float64_t));
		return COMPV_ERROR_CODE_E_SYSTEM;
	}
#endif
    COMPV_DEBUG_INFO("sizeof(compv_scalar_t)= #%lu", sizeof(compv_scalar_t));
    COMPV_DEBUG_INFO("sizeof(float)= #%lu", sizeof(float));

    // endianness
    // https://developer.apple.com/library/mac/documentation/Darwin/Conceptual/64bitPorting/MakingCode64-BitClean/MakingCode64-BitClean.html
#if TARGET_RT_LITTLE_ENDIAN
    s_bBigEndian = false;
#elif TARGET_RT_BIG_ENDIAN
    s_bBigEndian = true;
#else
    static const short kWord = 0x4321;
    s_bBigEndian = ((*(int8_t *)&kWord) != 0x21);
#	if defined(COMPV_OS_WINDOWS)
    if (s_bBigEndian) {
        COMPV_DEBUG_WARN("Big endian on Windows machine. Is it right?");
    }
#	endif
#endif

    // rand()
    srand((unsigned int)CompVTime::getNowMills());

#if defined(HAVE_GL_GLEW_H)
    GLenum glewErr = glewInit();
    if (GLEW_OK != glewErr) {
        COMPV_DEBUG_ERROR("Initializing GLEW failed (%d): %s", glewErr, glewGetErrorString(glewErr));
        COMPV_CHECK_CODE_RETURN(err_ = COMPV_ERROR_CODE_E_GLEW);
    }
#endif

    /* Image handlers initialization */
    COMPV_CHECK_CODE_BAIL(err_ = CompVImageDecoder::init());

    /* CPU features initialization */
    COMPV_CHECK_CODE_BAIL(err_ = CompVCpu::init());
    COMPV_DEBUG_INFO("CPU features: %s", CompVCpu::getFlagsAsString(CompVCpu::getFlags()));
    COMPV_DEBUG_INFO("CPU cores: #%d", CompVCpu::getCoresCount());
    COMPV_DEBUG_INFO("CPU cache1: line size: #%dB, size :#%dKB", CompVCpu::getCache1LineSize(), CompVCpu::getCache1Size()>>10);
#if defined(COMPV_ARCH_X86)
    // even if we are on X64 CPU it's possible that we're running a 32-bit binary
#	if defined(COMPV_ARCH_X64)
    COMPV_DEBUG_INFO("Binary type: X86_64");
#	else
    COMPV_DEBUG_INFO("Binary type: X86_32");
    if (CompVCpu::isSupported(kCpuFlagX64)) {
        COMPV_DEBUG_INFO("/!\\Using 32bits binaries on 64bits machine: optimization issues");
    }
#	endif
#endif
#if defined(COMPV_INTRINSIC)
    COMPV_DEBUG_INFO("Intrinsic enabled");
#endif
#if defined(COMPV_ASM)
    COMPV_DEBUG_INFO("Assembler enabled");
#endif
#if defined __INTEL_COMPILER
    COMPV_DEBUG_INFO("Using Intel compiler");
#endif
    // https://msdn.microsoft.com/en-us/library/jj620901.aspx
#if defined(__AVX2__)
    COMPV_DEBUG_INFO("Code built with option /arch:AVX2");
#elif defined(__AVX__)
    COMPV_DEBUG_INFO("Code built with option /arch:AVX");
#endif

    /* Math functions: Must be after CPU initialization */
    COMPV_CHECK_CODE_BAIL(err_ = CompVMathUtils::init());
    COMPV_DEBUG_INFO("Math Fast Trig.: %s", CompVEngine::isMathTrigFast() ? "true" : "fast");
    COMPV_DEBUG_INFO("Math Fixed Point: %s", CompVEngine::isMathFixedPoint() ? "true" : "fast");

    /* Memory alignment */
    COMPV_DEBUG_INFO("Default alignment: #%d", COMPV_SIMD_ALIGNV_DEFAULT);
    COMPV_DEBUG_INFO("Best alignment: #%d", CompVMem::getBestAlignment());

    /* Memory management */
    COMPV_CHECK_CODE_BAIL(err_ = CompVMem::init());

    /* Features */
    COMPV_CHECK_CODE_BAIL(err_ = CompVFeature::init());

    /* Matchers */
    COMPV_CHECK_CODE_BAIL(err_ = CompVMatcher::init());

    COMPV_DEBUG_INFO("Engine initialized");

bail:
    s_bInitialized = COMPV_ERROR_CODE_IS_OK(err_);
    s_bInitializing = false;
    // cleanup if initialization failed
    if (!s_bInitialized) {
        s_ThreadDisp = NULL;
		s_ThreadDisp11 = NULL;
    }
    else {
        // The next functions are called here because they recursively call "CompVEngine::init()"
        // We call them now because "s_bInitialized" is already set to "true" and this is the way to avoid endless loops

        // ThreadDispatcher
        // maxThreads: <= 0 means choose the best one, ==1 means disable, > 1 means enable
        if (numThreads > 1 || (numThreads <= 0 && CompVCpu::getCoresCount() > 1)) {
            COMPV_CHECK_CODE_BAIL(err_ = CompVThreadDispatcher::newObj(&s_ThreadDisp, numThreads));
			COMPV_CHECK_CODE_BAIL(err_ = CompVThreadDispatcher11::newObj(&s_ThreadDisp11, numThreads));
        }
    }
    return err_;
}

COMPV_ERROR_CODE CompVEngine::deInit()
{
    s_bInitialized = false;
    s_ThreadDisp = NULL;
	s_ThreadDisp11 = NULL;

    // TODO(dmi): deInit other modules (not an issue because there is no memory allocation)
    CompVMem::deInit();

    return COMPV_ERROR_CODE_S_OK;
}

CompVPtr<CompVThreadDispatcher* > CompVEngine::getThreadDispatcher()
{
    return s_ThreadDisp;
}

COMPV_ERROR_CODE CompVEngine::multiThreadingEnable(CompVPtr<CompVThreadDispatcher* > dispatcher)
{
    COMPV_CHECK_EXP_RETURN(!dispatcher, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    s_ThreadDisp = dispatcher;
    return COMPV_ERROR_CODE_S_OK;
}

CompVPtr<CompVThreadDispatcher11* > CompVEngine::getThreadDispatcher11()
{
	return s_ThreadDisp11;
}

COMPV_ERROR_CODE CompVEngine::multiThreadingEnable11(CompVPtr<CompVThreadDispatcher11* > dispatcher)
{
	COMPV_CHECK_EXP_RETURN(!dispatcher, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	s_ThreadDisp11 = dispatcher;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVEngine::multiThreadingDisable()
{
    s_ThreadDisp = NULL;
	s_ThreadDisp11 = NULL;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVEngine::multiThreadingSetMaxThreads(size_t maxThreads)
{
    CompVPtr<CompVThreadDispatcher *> newThreadDisp;
    COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::newObj(&newThreadDisp));
    s_ThreadDisp = newThreadDisp;// TODO(dmi): function not optimal, we destroy all threads and create new ones

	CompVPtr<CompVThreadDispatcher11 *> newThreadDisp11;
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher11::newObj(&newThreadDisp11));
	s_ThreadDisp11 = newThreadDisp11;// TODO(dmi): function not optimal, we destroy all threads and create new ones

    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVEngine::setTestingModeEnabled(bool bTesting)
{
    COMPV_DEBUG_INFO("Engine testing mode = %s", bTesting ? "true" : "false");
    s_bTesting = bTesting;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVEngine::setMathTrigFastEnabled(bool bMathTrigFast)
{
    COMPV_DEBUG_INFO("Engine math trig. fast = %s", bMathTrigFast ? "true" : "false");
    s_bMathTrigFast = bMathTrigFast;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVEngine::setMathFixedPointEnabled(bool bMathFixedPoint)
{
    COMPV_DEBUG_INFO("Engine math trig. fast = %s", bMathFixedPoint ? "true" : "false");
    s_bMathFixedPoint= bMathFixedPoint;
    return COMPV_ERROR_CODE_S_OK;
}

bool CompVEngine::isMultiThreadingEnabled()
{
    return !!s_ThreadDisp;
}

bool CompVEngine::isInitialized()
{
    return s_bInitialized;
}

bool CompVEngine::isInitializing()
{
    return s_bInitializing;
}

bool CompVEngine::isBigEndian()
{
    return s_bBigEndian;
}

bool CompVEngine::isTestingMode()
{
    return s_bTesting;
}

bool CompVEngine::isMathTrigFast()
{
    return s_bMathTrigFast;
}

bool CompVEngine::isMathFixedPoint()
{
    return s_bMathFixedPoint;
}

COMPV_NAMESPACE_END()
