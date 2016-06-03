/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_MATHUTILS_H_)
#define _COMPV_MATHUTILS_H_

#include "compv/compv_config.h"
#include "compv/compv_common.h"
#include "compv/math/compv_math.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_API CompVMathUtils
{
public:
    static COMPV_ERROR_CODE init();
    static COMPV_INLINE int32_t clamp(int32_t min, int32_t val, int32_t max) { // do not use macro, otherwise 'val' which coulbe a function will be evaluated several times
        // Important: Do not use CLIP3_INT here: very slow on Windows (tested on Win8 core i7 using VS2013)
        return COMPV_MATH_CLIP3(min, max, val);
    }
    static COMPV_INLINE uint8_t clampPixel8(int16_t val) {
        // Important: Do not use CLIP3_INT here: very slow on Windows (tested on Win8 core i7 using VS2013)
        // Also, asm_cmov_clip2() is sloow -> To be checked
        return (uint8_t)COMPV_MATH_CLIP3(0, 255, val);
    }
    static COMPV_INLINE compv_scalar_t maxVal(compv_scalar_t x, compv_scalar_t y) {
        return maxValFunc(x, y);
    }
    static COMPV_INLINE compv_scalar_t minVal(compv_scalar_t x, compv_scalar_t y) {
        return minValFunc(x, y);
    }
    static COMPV_INLINE int32_t minArrayI32(const int32_t* array, compv_scalar_t count) {
        return minArrayI32Func(array, count);
    }
    static COMPV_INLINE compv_scalar_t clip3(compv_scalar_t min, compv_scalar_t max, compv_scalar_t val) {
        return clip3Func(min, max, val);
    }
    static COMPV_INLINE compv_scalar_t clip2(compv_scalar_t max, compv_scalar_t val) {
        return clip2Func(max, val);
    }


	template <typename T>
	static COMPV_INLINE T hypot(T x, T y) {
#if 1
		return hypot(x, y);
#elif 0
		// https://en.wikipedia.org/wiki/Hypot
		// Without overflow / underflow
		T t;
		x = COMPV_MATH_ABS(x);
		y = COMPV_MATH_ABS(y);
		t = COMPV_MATH_MIN(x, y);
		x = COMPV_MATH_MAX(x, y);
		t = t / x;
		return x * COMPV_MATH_SQRT(1 + t*t);
#endif
	}
	template <typename T>
	static COMPV_INLINE T hypot_naive(T x, T y) {
		return COMPV_MATH_SQRT(x*x + y*y);
	}
private:
    static bool s_Initialized;
    static compv_scalar_t(*maxValFunc)(compv_scalar_t a, compv_scalar_t b);
    static compv_scalar_t(*minValFunc)(compv_scalar_t a, compv_scalar_t b);
    static int32_t(*minArrayI32Func)(const int32_t* array, compv_scalar_t count);
    static compv_scalar_t(*clip3Func)(compv_scalar_t min, compv_scalar_t max, compv_scalar_t val);
    static compv_scalar_t(*clip2Func)(compv_scalar_t max, compv_scalar_t val);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_MATHUTILS_H_ */
