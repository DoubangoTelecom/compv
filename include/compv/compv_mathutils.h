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
#include "compv/compv_math.h"

COMPV_NAMESPACE_BEGIN()

class CompVMathUtils {
public:
	static COMPV_INLINE int32_t clamp(int32_t min, int32_t val, int32_t max) { // do not use macro, otherwise 'val' which coulbe a function will be evaluated several times
		return COMPV_MATH_CLIP3(min, max, val);
	}
	static COMPV_INLINE uint8_t clampPixel8(int16_t val) {
		return (uint8_t)COMPV_MATH_CLIP3(0, 255, val);
	}
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_MATHUTILS_H_ */