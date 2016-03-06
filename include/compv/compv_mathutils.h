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
#if !defined(_COMPV_MATHUTILS_H_)
#define _COMPV_MATHUTILS_H_

#include "compv/compv_config.h"
#include "compv/compv_common.h"
#include "compv/compv_math.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_API CompVMathUtils {
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
	static COMPV_INLINE compv_scalar_t clip3(compv_scalar_t min, compv_scalar_t max, compv_scalar_t val) {
		return clip3Func(min, max, val);
	}
	static COMPV_INLINE compv_scalar_t clip2(compv_scalar_t max, compv_scalar_t val) {
		return clip2Func(max, val);
	}
private:
	static bool s_Initialized;
	static compv_scalar_t(*maxValFunc)(compv_scalar_t a, compv_scalar_t b);
	static compv_scalar_t(*minValFunc)(compv_scalar_t a, compv_scalar_t b);
	static compv_scalar_t(*clip3Func)(compv_scalar_t min, compv_scalar_t max, compv_scalar_t val);
	static compv_scalar_t(*clip2Func)(compv_scalar_t max, compv_scalar_t val);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_MATHUTILS_H_ */
