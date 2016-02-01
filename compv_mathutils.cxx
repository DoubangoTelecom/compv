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
#include "compv/compv_mathutils.h"
#include "compv/compv_cpu.h"

#include <algorithm>

COMPV_NAMESPACE_BEGIN()

bool CompVMathUtils::s_Initialized = false;

static compv_scalar_t maxVal_C(compv_scalar_t x, compv_scalar_t y);
static compv_scalar_t minVal_C(compv_scalar_t x, compv_scalar_t y);
static compv_scalar_t clip3_C(compv_scalar_t min, compv_scalar_t max, compv_scalar_t val);
static compv_scalar_t clip2_C(compv_scalar_t max, compv_scalar_t val);

#if COMPV_ARCH_X86 && COMPV_ASM
extern "C" compv_scalar_t compv_mathutils_maxval_asm_x86_cmov(compv_scalar_t x, compv_scalar_t y);
extern "C" compv_scalar_t compv_mathutils_minval_asm_x86_cmov(compv_scalar_t x, compv_scalar_t y);
extern "C" compv_scalar_t compv_mathutils_clip3_asm_x86_cmov(compv_scalar_t min, compv_scalar_t max, compv_scalar_t val);
extern "C" compv_scalar_t compv_mathutils_clip2_asm_x86_cmov(compv_scalar_t max, compv_scalar_t val);
#endif

compv_scalar_t(*CompVMathUtils::maxValFunc)(compv_scalar_t a, compv_scalar_t b) = maxVal_C;
compv_scalar_t(*CompVMathUtils::minValFunc)(compv_scalar_t a, compv_scalar_t b) = minVal_C;
compv_scalar_t(*CompVMathUtils::clip3Func)(compv_scalar_t min, compv_scalar_t max, compv_scalar_t val) = clip3_C;
compv_scalar_t(*CompVMathUtils::clip2Func)(compv_scalar_t max, compv_scalar_t val) = clip2_C;

COMPV_ERROR_CODE CompVMathUtils::init()
{
    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;

    if (!CompVMathUtils::s_Initialized) {
        if (CompVCpu::isSupported(kCpuFlagCMOV)) {
            COMPV_EXEC_IFDEF_ASM_X86(CompVMathUtils::maxValFunc = compv_mathutils_maxval_asm_x86_cmov);
            COMPV_EXEC_IFDEF_ASM_X86(CompVMathUtils::minValFunc = compv_mathutils_minval_asm_x86_cmov);
            COMPV_EXEC_IFDEF_ASM_X86(CompVMathUtils::clip3Func = compv_mathutils_clip3_asm_x86_cmov);
            COMPV_EXEC_IFDEF_ASM_X86(CompVMathUtils::clip2Func = compv_mathutils_clip2_asm_x86_cmov);
        }
        CompVMathUtils::s_Initialized = true;
    }

    return err_;
}

static compv_scalar_t maxVal_C(compv_scalar_t x, compv_scalar_t y)
{
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
    return std::max(x, y);
}

static compv_scalar_t minVal_C(compv_scalar_t x, compv_scalar_t y)
{
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
    return std::min(x, y);
}

compv_scalar_t clip3_C(compv_scalar_t min, compv_scalar_t max, compv_scalar_t val)
{
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
    return CompVMathUtils::maxVal(min, CompVMathUtils::minVal(val, max));
}

compv_scalar_t clip2_C(compv_scalar_t max, compv_scalar_t val)
{
    return clip3_C(0, max, val);
}

COMPV_NAMESPACE_END()
