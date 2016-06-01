/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_MATH_TRIG_H_)
#define _COMPV_MATH_TRIG_H_

#include "compv/compv_config.h"
#include "compv/compv_common.h"
#include "compv/math/compv_math.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_API CompVMathTrig
{
public:
    static void sincos_Zero_PiOver2_P32(const float* inRad, float* outSin, float* outCos, compv_scalar_t count);
    static void sincos_Zero_PiTime2_P32(const float* inRad, float* outSin, float* outCos, compv_scalar_t count);
    static void sincos_P32(const float* inRad, float* outSin, float* outCos, compv_scalar_t count);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_MATH_TRIG_H_ */
