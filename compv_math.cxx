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
/*
Most of trig approx. are implemented using document at "documentation/trig_approximations.pdf"
*/
#include "compv/compv_math.h"

COMPV_NAMESPACE_BEGIN()

COMPV_API const float kfMathTrigPi = 3.1415926535897932384626433f; // PI
COMPV_API const float kfMathTrigPiTimes2 = 2.f * kfMathTrigPi; // PI * 2
COMPV_API const float kfMathTrigPiOver2 = kfMathTrigPi / 2.f; // PI / 2
COMPV_API const float kfMathTrigPiOver180 = kfMathTrigPi / 180.f; // PI/180
COMPV_API const float kfMathTrig180OverPi = 180.f / kfMathTrigPi; // 180/PI
COMPV_API const float kMathTrigC1 = 0.99940307f;
COMPV_API const float kMathTrigC2 = -0.49558072f;
COMPV_API const float kMathTrigC3 = 0.03679168f;

COMPV_NAMESPACE_END()
