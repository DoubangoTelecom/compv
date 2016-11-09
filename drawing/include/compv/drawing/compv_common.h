/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_DRAWING_COMMON_H_)
#define _COMPV_DRAWING_COMMON_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"

COMPV_NAMESPACE_BEGIN()

struct CompVDrawingViewport {
public:
	int x; int y; int width; int height;
	CompVDrawingViewport(int x_ = 0, int y_ = 0, int width_ = -1, int height_ = -1) : x(x_), y(y_), width(width_), height(height_) { }
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_DRAWING_CANVAS_H_ */
