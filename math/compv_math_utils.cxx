/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/math/compv_math_utils.h"
#include "compv/compv_cpu.h"

#include <algorithm>

COMPV_NAMESPACE_BEGIN()

bool CompVMathUtils::s_Initialized = false;

static compv_scalar_t maxVal_C(compv_scalar_t x, compv_scalar_t y);
static compv_scalar_t minVal_C(compv_scalar_t x, compv_scalar_t y);
static int32_t minArrayI32_C(const int32_t* array, compv_scalar_t count);
static compv_scalar_t clip3_C(compv_scalar_t min, compv_scalar_t max, compv_scalar_t val);
static compv_scalar_t clip2_C(compv_scalar_t max, compv_scalar_t val);

#if COMPV_ARCH_X86 && COMPV_ASM
COMPV_EXTERNC compv_scalar_t compv_mathutils_maxval_asm_x86_cmov(compv_scalar_t x, compv_scalar_t y);
COMPV_EXTERNC compv_scalar_t compv_mathutils_minval_asm_x86_cmov(compv_scalar_t x, compv_scalar_t y);
COMPV_EXTERNC compv_scalar_t compv_mathutils_clip3_asm_x86_cmov(compv_scalar_t min, compv_scalar_t max, compv_scalar_t val);
COMPV_EXTERNC compv_scalar_t compv_mathutils_clip2_asm_x86_cmov(compv_scalar_t max, compv_scalar_t val);
#endif

compv_scalar_t(*CompVMathUtils::maxValFunc)(compv_scalar_t a, compv_scalar_t b) = maxVal_C;
compv_scalar_t(*CompVMathUtils::minValFunc)(compv_scalar_t a, compv_scalar_t b) = minVal_C;
int32_t(*CompVMathUtils::minArrayI32Func)(const int32_t* array, compv_scalar_t count) = minArrayI32_C;
compv_scalar_t(*CompVMathUtils::clip3Func)(compv_scalar_t min, compv_scalar_t max, compv_scalar_t val) = clip3_C;
compv_scalar_t(*CompVMathUtils::clip2Func)(compv_scalar_t max, compv_scalar_t val) = clip2_C;

COMPV_ERROR_CODE CompVMathUtils::init()
{
    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;

    if (!CompVMathUtils::s_Initialized) {
        if (CompVCpu::isEnabled(kCpuFlagCMOV)) {
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

static int32_t minArrayI32_C(const int32_t* array, compv_scalar_t count)
{
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
    int32_t min = array[0];
    for (compv_scalar_t i = 1; i < count; ++i) {
        min = std::min(min, array[i]);
    }
    return min;
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
