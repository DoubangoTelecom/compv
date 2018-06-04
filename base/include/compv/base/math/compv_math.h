/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MATH_H_)
#define _COMPV_BASE_MATH_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_mat.h"

#include <cmath>

COMPV_NAMESPACE_BEGIN()

#define COMPV_MATH_LN_10							2.3025850929940456840179914546844
#define COMPV_MATH_SQRT_2							1.4142135623730950488016887242097
#define COMPV_MATH_1_OVER_SQRT_2					0.70710678118654752440084436210485
#define COMPV_MATH_SQRT_1_OVER_2					0.70710678118654752440084436210485
#define COMPV_MATH_MOD_POW2_INT32(val, div)			((val) & ((div) - 1))  // div must be power of 2 // FIXME: rename log2...?
#define COMPV_MATH_MAX(x,y)							((x) > (y) ? (x) : (y))
#define COMPV_MATH_MAX_3(x,y,z)						(COMPV_MATH_MAX(x,COMPV_MATH_MAX(y,z)))
#define COMPV_MATH_MIN(x,y)							((x) > (y) ? (y) : (x))
#define COMPV_MATH_MIN_3(x,y,z)						(COMPV_MATH_MIN(x,COMPV_MATH_MIN(y,z)))
#define COMPV_MATH_MIN_INT(x, y)					((y) ^ (((x) ^ (y)) & -((x) < (y))))
#define COMPV_MATH_MAX_INT(x, y)					((x) ^ (((x) ^ (y)) & -((x) < (y))))
#define COMPV_MATH_MIN_POSITIVE(x, y)				(((x) >= 0 && (y)>=0) ? COMPV_MATH_MIN((x), (y)) : COMPV_MATH_MAX((x), (y)))	// (G-245)
#define COMPV_MATH_ABS(x)							(std::abs((x))) // ::abs() accepts integers only, use std::abs
#define COMPV_MATH_ABS_INT32(x)						COMPV_MATH_ABS((x))//FIXME:((x) ^ ((x) >> 31)) - ((x) >> 31)
#define COMPV_MATH_MV_DIFF(mv1, mv2)				(COMPV_MATH_ABS((mv1)[0] - (mv2)[0]) + COMPV_MATH_ABS((mv1)[1] - (mv2)[1])) // mvDiff( mv1, mv2 ) = Abs( mv1[ 0 ] - mv2[ 0 ] ) + Abs( mv1[ 1 ] - mv2[ 1 ] ) (G-251)
#define COMPV_MATH_CEIL(x)							(std::ceil((x)))
#define COMPV_MATH_CLIP3(x,y,z)						((z)<(x) ? (x) : ((z)>(y) ? (y) : (z))) //!\ Do not transform to max(x, min(z, y)) as each macro (max and min) would be expanded several times
#define COMPV_MATH_CLIP3_INT(x,y,z)					MUST_NOT_USE_WOULD_BE_SLOOOOOW
#define COMPV_MATH_CLIP1Y(_x,BitDepthY)				COMPV_MATH_CLIP3(0, (1 << (BitDepthY)) - 1, (_x))
#define COMPV_MATH_CLIP1C(_x,BitDepthC)				COMPV_MATH_CLIP3(0, (1 << (BitDepthC)) - 1, (_x))
#define COMPV_MATH_CLIP2(y,z)						COMPV_MATH_CLIP3(0,(y),(z))// // Clip2(max, val) = Clip3(0, max, val)
#define COMPV_MATH_TAP6FILTER(E, F, G, H, I, J)		((E) - 5*((F) + (I)) + 20*((G) + (H)) + (J))
#define COMPV_MATH_FLOOR(x)							(std::floor(x))
#define COMPV_MATH_INVERSE_RASTER_SCAN(a,b,c,d,e)	((e)==0 ? ((a)%((d)/(b)))*(b) : ((a)/((d)/(b)))*(c))
#define COMPV_MATH_LOG(x)							std::log((x))
#define COMPV_MATH_LOG2(x)							(1/std::log(2.0)) * std::log((x))
#define COMPV_MATH_LOGA0(x)							(std::log10((x)))
#define COMPV_MATH_MEDIAN(x,y,z)					((x)+(y)+(z)-COMPV_MATH_MIN((x),COMPV_MATH_MIN((y),(z)))-COMPV_MATH_MAX((x),COMPV_MATH_MAX((y),(z))))
#define COMPV_MATH_SIGN(x)							((x)>=0 ? 1 : -1)
#define COMPV_MATH_ROUND(x)							(COMPV_MATH_SIGN((x))*COMPV_MATH_FLOOR(COMPV_MATH_ABS((x))+0.5))
#define COMPV_MATH_SQRT(x)							std::sqrt((x))
#define COMPV_MATH_COS								std::cos
#define COMPV_MATH_SIN								std::sin
#define COMPV_MATH_TAN								std::tan
#define COMPV_MATH_ASIN								std::asin
#define COMPV_MATH_ACOS								std::acos
#define COMPV_MATH_EXP								std::exp
#define COMPV_MATH_POW								std::pow
#define COMPV_MATH_ATAN2							std::atan2
#define COMPV_MATH_ATAN								std::atan
#define COMPV_MATH_ATAN2F							std::atan2f
#define COMPV_MATH_ROUNDF_2_NEAREST_INT(f,inttype)			static_cast<inttype>(static_cast<int>/*Cast to 'int' then 'inttype' is important when 'inttype' is unsigned: see comment*/((f) >= 0.0 ? ((f) + 0.5) : ((f) - 0.5))) //!\IMPORTANT: (f) is evaluated several times -> must be an 'immediate' value. Round to nearest. Also, when f < 0.f must cast to int then 'inttype', see http://www.embeddeduse.com/2013/08/25/casting-a-negative-float-to-an-unsigned-int/
#define COMPV_MATH_ROUNDFU_2_NEAREST_INT(f,inttype)			static_cast<inttype>((f) + 0.5) // fast rounding: (f) must be > 0, 0.5 will be rounded to toward the closest even number (SIMD: add 0.5, then cvt with truncation). Round to nearest.
#define COMPV_MATH_HYPOT_NAIVE(x, y)				COMPV_MATH_SQRT((x)*(x) + (y)*(y))

#if !defined(M_PI)
#	define M_PI 3.1415926535897932384626433832795
#endif
#define COMPV_MATH_PI	(M_PI)
#define COMPV_MATH_DEGREE_TO_RADIAN(deg_)			(((deg_) * COMPV_MATH_PI) / 180.0)
#define COMPV_MATH_DEGREE_TO_RADIAN_FLOAT(deg_)		(float)((deg_) * kfMathTrigPiOver180)
#define COMPV_MATH_RADIAN_TO_DEGREE(rad_)			(((rad_) * 180.0) / COMPV_MATH_PI)
#define COMPV_MATH_RADIAN_TO_DEGREE_FLOAT(rad_)		(float)((rad_) * kfMathTrig180OverPi)

extern COMPV_BASE_API const float kfMathTrigPi; // PI
extern COMPV_BASE_API const float kfMathTrigPiTimes2; // PI*2
extern COMPV_BASE_API const float kfMathTrigPiOver2; // PI/2
extern COMPV_BASE_API const float kfMathTrigPiOver180; // PI/180 (used to convert from degree to radian)
extern COMPV_BASE_API const float kfMathTrig180OverPi; // 180/PI (used to convert from radian to degree)

extern COMPV_BASE_API const float kMathTrigC1;
extern COMPV_BASE_API const float kMathTrigC2;
extern COMPV_BASE_API const float kMathTrigC3;

extern COMPV_BASE_API const double kMathTrigAtan2Eps;
extern COMPV_BASE_API const float kMathTrigAtan2P1;
extern COMPV_BASE_API const float kMathTrigAtan2P3;
extern COMPV_BASE_API const float kMathTrigAtan2P5;
extern COMPV_BASE_API const float kMathTrigAtan2P7;

class COMPV_BASE_API CompVMath
{
public:
	static COMPV_ERROR_CODE sub(const CompVMatPtr& A, const CompVMatPtr& B, CompVMatPtrPtr R);
	static COMPV_ERROR_CODE mulAB(const CompVMatPtr &A, const CompVMatPtr &B, CompVMatPtrPtr R);
	static COMPV_ERROR_CODE mulABt(const CompVMatPtr &A, const CompVMatPtr &B, CompVMatPtrPtr R);
	static COMPV_ERROR_CODE mulAtA(const CompVMatPtr &A, CompVMatPtrPtr R);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_MATH_H_ */
