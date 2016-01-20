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
#if !defined(_COMPV_MATH_H_)
#define _COMPV_MATH_H_

#include "compv/compv_config.h"

#include <math.h>

COMPV_NAMESPACE_BEGIN()

#define COMPV_MATH_LN_10							2.3025850929940456840179914546844
#define COMPV_MATH_PI								3.1415926535897932384626433832795
#define COMPV_MATH_SQRT_2							1.4142135623730950488016887242097
#define COMPV_MATH_1_BY_SQRT_2						0.70710678118654752440084436210485
#define COMPV_MATH_SQRT_1_BY_2						0.70710678118654752440084436210485
#define COMPV_MATH_MOD_POW2_INT32(val, div)			((val) & ((div) - 1))  // div must be power of 2 // FIXME: rename log2...?
#define COMPV_MATH_MAX(x,y)							((x) > (y) ? (x) : (y))
#define COMPV_MATH_MAX_3(x,y,z)						(COMPV_MATH_MAX(x,COMPV_MATH_MAX(y,z)))
#define COMPV_MATH_MIN(x,y)							((x) > (y) ? (y) : (x))
#define COMPV_MATH_MIN_3(x,y,z)						(COMPV_MATH_MIN(x,COMPV_MATH_MIN(y,z)))
#define COMPV_MATH_MIN_POSITIVE(x, y)				(((x) >= 0 && (y)>=0) ? COMPV_MATH_MIN((x), (y)) : COMPV_MATH_MAX((x), (y)))	// (G-245)
#define COMPV_MATH_ABS(x)							(abs((x)))
#define COMPV_MATH_ABS_INT32(x)						COMPV_MATH_ABS((x))//FIXME:((x) ^ ((x) >> 31)) - ((x) >> 31)
#define COMPV_MATH_MV_DIFF(mv1, mv2)				(COMPV_MATH_ABS((mv1)[0] - (mv2)[0]) + COMPV_MATH_ABS((mv1)[1] - (mv2)[1])) // mvDiff( mv1, mv2 ) = Abs( mv1[ 0 ] - mv2[ 0 ] ) + Abs( mv1[ 1 ] - mv2[ 1 ] ) (G-251)
#define COMPV_MATH_CEIL(x)							(ceil((x)))
#define COMPV_MATH_CLIP3(x,y,z)						((z)<(x) ? (x) : ((z)>(y) ? (y) : (z)))
#define COMPV_MATH_CLIP1Y(_x,BitDepthY)				COMPV_MATH_CLIP3(0, (1 << (BitDepthY)) - 1, (_x))
#define COMPV_MATH_CLIP1C(_x,BitDepthC)				COMPV_MATH_CLIP3(0, (1 << (BitDepthC)) - 1, (_x))
#define COMPV_MATH_CLIP2(y,z)						COMPV_MATH_CLIP3(0,(y),(z))// // Clip2(max, val) = Clip3(0, max, val)
#define COMPV_MATH_TAP6FILTER(E, F, G, H, I, J)		((E) - 5*((F) + (I)) + 20*((G) + (H)) + (J))
#define COMPV_MATH_FLOOR(x)							(floor(x))
#define COMPV_MATH_INVERSE_RASTER_SCAN(a,b,c,d,e)	((e)==0 ? ((a)%((d)/(b)))*(b) : ((a)/((d)/(b)))*(c))
#define COMPV_MATH_LOG(x)							log((x))
#define COMPV_MATH_LOG2(x)							(1/log(2.0)) * log((x))
#define COMPV_MATH_LOGA0(x)							(log10((x)))
#define COMPV_MATH_MEDIAN(x,y,z)					((x)+(y)+(z)-COMPV_MATH_MIN((x),COMPV_MATH_MIN((y),(z)))-COMPV_MATH_MAX((x),COMPV_MATH_MAX((y),(z))))
#define COMPV_MATH_SIGN(x)							((x)>=0 ? 1 : -1)
#define COMPV_MATH_ROUND(x)							(COMPV_MATH_SIGN((x))*COMPV_MATH_FLOOR(COMPV_MATH_ABS((x))+0.5))
#define COMPV_MATH_SQRT(x)							sqrt((x))
#define COMPV_MATH_COS								cos
#define COMPV_MATH_SIN								sin
#define COMPV_MATH_TAN								tan
#define COMPV_MATH_ASIN								asin
#define COMPV_MATH_EXP								exp
#define COMPV_MATH_POW								pow

COMPV_NAMESPACE_END()

#endif /* _COMPV_MATH_H_ */