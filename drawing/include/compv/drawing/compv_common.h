/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_DRAWING_COMMON_H_)
#define _COMPV_DRAWING_COMMON_H_

#include "compv/drawing/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/gl/compv_common.h"

COMPV_NAMESPACE_BEGIN()

struct CompVDrawingVec3f {
public:
	union {
		struct { float x, y, z; };
		struct { float r, g, b; };
		struct { float s, t, p; };
	};
	CompVDrawingVec3f(float x_, float y_, float z_) : x(x_), y(y_), z(z_) { }
};

struct CompVDrawingRect {
public:
	int left;
	int top;
	int right;
	int bottom;
	CompVDrawingRect(int left_ = 0, int top_ = 0, int right_ = 0, int bottom_ = 0): left(left_), top(top_), right(right_), bottom(bottom_) {  }
	static CompVDrawingRect makeFromWidthHeight(int x, int y, int width, int height) { return CompVDrawingRect(x, y, x + width, y + height); }
};

struct CompVDrawingRatio {	
public:
	int numerator;
	int denominator;
	CompVDrawingRatio(int numerator_ = 1, int denominator_ = 1) : numerator(numerator_), denominator(denominator_) { }
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_DRAWING_CANVAS_H_ */
