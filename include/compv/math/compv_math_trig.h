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
