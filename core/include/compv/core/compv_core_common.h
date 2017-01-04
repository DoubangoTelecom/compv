/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_CORE_COMMON_H_)
#define _COMPV_CORE_COMMON_H_

#include "compv/core/compv_core_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_box.h"

COMPV_NAMESPACE_BEGIN()


struct CompVInterestPoint {
	compv_float32_t x; /**< Point.x */
	compv_float32_t y; /**< Point.y */
	compv_float32_t strength; /**< Corner/edge strength/response (e.g. FAST response or Harris response) */
	compv_float32_t orient; /**< angle in degree ([0-360]) */
	int32_t level; /**< pyramid level (when image is scaled, level0 is the first one) */
	compv_float32_t size; /**< patch size (e.g. BRIEF patch size-circle diameter-) */

protected:
	COMPV_INLINE void init(compv_float32_t x_, compv_float32_t y_, compv_float32_t strength_ = -1.f, compv_float32_t orient_ = -1.f, int32_t level_ = 0, compv_float32_t size_ = 0.f) {
		x = x_, y = y_, strength = strength_, orient = orient_, level = level_, size = size_;
	}
public:
	CompVInterestPoint() {
		init(0, 0);
	}
	CompVInterestPoint(compv_float32_t x_, compv_float32_t y_, compv_float32_t strength_ = -1.f, compv_float32_t orient_ = -1.f, int32_t level_ = 0, compv_float32_t size_ = 0.f) {
		init(x_, y_, strength_, orient_, level_, size_);
	}
};

struct CompVDMatch {
	int32_t queryIdx;
	int32_t trainIdx;
	int32_t imageIdx;
	int32_t distance;
protected:
	COMPV_INLINE void init(int32_t queryIdx_, int32_t trainIdx_, int32_t distance_, int32_t imageIdx_ = 0) {
		queryIdx = queryIdx_, trainIdx = trainIdx_, distance = distance_, imageIdx = imageIdx_;
	}
public:
	CompVDMatch() {
		init(0, 0, 0, 0);
	}
	CompVDMatch(int32_t queryIdx_, int32_t trainIdx_, int32_t distance_, int32_t imageIdx_ = 0) {
		init(queryIdx_, trainIdx_, distance_, imageIdx_);
	}
};
typedef CompVBox<CompVDMatch> CompVBoxDMatch;
typedef CompVPtr<CompVBoxDMatch*> CompVBoxDMatchPtr;
typedef CompVBoxDMatchPtr* CompVBoxDMatchPtrPtr;

COMPV_NAMESPACE_END()

#endif /* _COMPV_CORE_COMMON_H_ */
