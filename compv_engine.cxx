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
#include "compv/compv_engine.h"
#include "compv/compv_mem.h"
#include "compv/compv_cpu.h"
#include "compv/compv_debug.h"
#include "compv/time/compv_time.h"
#include "compv/image/compv_image.h"
#include "compv/features/compv_feature.h"
#include "compv/compv_mathutils.h"


COMPV_NAMESPACE_BEGIN()

bool CompVEngine::s_bInitialized = false;
bool CompVEngine::s_bInitializing = false;
#if defined(COMPV_OS_WINDOWS)
bool CompVEngine::s_bBigEndian = false;
#else
bool CompVEngine::s_bBigEndian = true;
#endif
bool CompVEngine::s_bTesting = false;
CompVObjWrapper<CompVThreadDispatcher *> CompVEngine::s_ThreadDisp = NULL;

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
#endif
    COMPV_DEBUG_INFO("sizeof(compv_scalar_t)= #%lu", sizeof(compv_scalar_t));

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

    /* Memory alignment */
    COMPV_DEBUG_INFO("Default alignment: #%d", COMPV_SIMD_ALIGNV_DEFAULT);
    COMPV_DEBUG_INFO("Best alignment: #%d", CompVMem::getBestAlignment());

	/* Memory management */
	COMPV_CHECK_CODE_BAIL(err_ = CompVMem::init());

    /* Features */
    COMPV_CHECK_CODE_BAIL(err_ = CompVFeature::init());


    COMPV_DEBUG_INFO("Engine initialized");

bail:
    s_bInitialized = COMPV_ERROR_CODE_IS_OK(err_);
	s_bInitializing = false;
    // cleanup if initialization failed
    if (!s_bInitialized) {
        s_ThreadDisp = NULL;
    }
    else {
        // The next functions are called here because they recursively call "CompVEngine::init()"
        // We call them now because "s_bInitialized" is already set to "true" and this is the way to avoid endless loops

        // ThreadDispatcher
        // maxThreads: <= 0 means choose the best one, ==1 means disable, > 1 means enable
        if (numThreads > 1 || (numThreads <= 0 && CompVCpu::getCoresCount() > 1)) {
            COMPV_CHECK_CODE_BAIL(err_ = CompVThreadDispatcher::newObj(&s_ThreadDisp, numThreads));
        }
    }
    return err_;
}

COMPV_ERROR_CODE CompVEngine::deInit()
{
    s_bInitialized = false;
    s_ThreadDisp = NULL;
    
	// TODO(dmi): deInit other modules (not an issue because there is no memory allocation)
	CompVMem::deInit();

    return COMPV_ERROR_CODE_S_OK;
}

CompVObjWrapper<CompVThreadDispatcher* > CompVEngine::getThreadDispatcher()
{
    return s_ThreadDisp;
}

COMPV_ERROR_CODE CompVEngine::multiThreadingEnable(CompVObjWrapper<CompVThreadDispatcher* > dispatcher)
{
    COMPV_CHECK_EXP_RETURN(!dispatcher, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    s_ThreadDisp = dispatcher;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVEngine::multiThreadingDisable()
{
    s_ThreadDisp = NULL;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVEngine::multiThreadingSetMaxThreads(size_t maxThreads)
{
    CompVObjWrapper<CompVThreadDispatcher *> newThreadDisp;
    COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::newObj(&newThreadDisp));
    s_ThreadDisp = newThreadDisp;// TODO(dmi): function not optimal, we destroy all threads and create new ones
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVEngine::setTestingModeEnabled(bool bTesting)
{
    COMPV_DEBUG_INFO("Engine testing mode = %s", bTesting ? "true" : "false");
    s_bTesting = bTesting;
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

COMPV_NAMESPACE_END()