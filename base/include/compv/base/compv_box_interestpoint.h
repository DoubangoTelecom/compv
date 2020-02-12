/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_BOX_INTERESTPOINT_H_)
#define _COMPV_BASE_BOX_INTERESTPOINT_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_box.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)

COMPV_OBJECT_DECLARE_PTRS(BoxInterestPoint)

class COMPV_BASE_API CompVBoxInterestPoint : public CompVBox<CompVInterestPoint >
{
protected:
	CompVBoxInterestPoint(size_t nCapacity = 0, bool bLockable = false);
public:
	virtual ~CompVBoxInterestPoint();
	COMPV_OBJECT_GET_ID(CompVBoxInterestPoint);
	COMPV_ERROR_CODE sortByStrength(); // multithreaded sorting
	COMPV_ERROR_CODE retainBest(size_t count);
	COMPV_ERROR_CODE eraseTooCloseToBorder(size_t img_width, size_t img_height, size_t border_size);
	static COMPV_ERROR_CODE newObj(CompVBoxInterestPointPtrPtr box, size_t nCapacity = 0, bool bLockable = false);
};

void CompVInterestPointScaleAndRoundAndGetAngleSinCos(COMPV_ALIGNED(x) const float* xf, COMPV_ALIGNED(x) const float *yf, COMPV_ALIGNED(x) const float *sf, COMPV_ALIGNED(x) const float* angleInDegree, COMPV_ALIGNED(x) int32_t* xi, COMPV_ALIGNED(x) int32_t* yi, COMPV_ALIGNED(x) float* cos, COMPV_ALIGNED(x) float* sin, compv_scalar_t count);

COMPV_VS_DISABLE_WARNINGS_END()

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_BOX_INTERESTPOINT_H_ */
