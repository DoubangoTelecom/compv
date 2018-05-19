/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
/*
Most of trig approx. are implemented using document at "documentation/trig_approximations.pdf"
*/
#include "compv/base/math/compv_math.h"

COMPV_NAMESPACE_BEGIN()

COMPV_BASE_API const float kfMathTrigPi = 3.1415926535897932384626433f; // PI
COMPV_BASE_API const float kfMathTrigPiTimes2 = 2.f * kfMathTrigPi; // PI * 2
COMPV_BASE_API const float kfMathTrigPiOver2 = kfMathTrigPi / 2.f; // PI / 2
COMPV_BASE_API const float kfMathTrigPiOver180 = kfMathTrigPi / 180.f; // PI/180
COMPV_BASE_API const float kfMathTrig180OverPi = 180.f / kfMathTrigPi; // 180/PI

COMPV_BASE_API const float kMathTrigC1 = 0.99940307f;
COMPV_BASE_API const float kMathTrigC2 = -0.49558072f;
COMPV_BASE_API const float kMathTrigC3 = 0.03679168f;

COMPV_BASE_API const double kMathTrigAtan2Eps = 2.2204460492503131e-016;
COMPV_BASE_API const float kMathTrigAtan2P1 = 57.2836266f;
COMPV_BASE_API const float kMathTrigAtan2P3 = -18.6674461f;
COMPV_BASE_API const float kMathTrigAtan2P5 = 8.91400051f;
COMPV_BASE_API const float kMathTrigAtan2P7 = -2.53972459f;

COMPV_NAMESPACE_END()
