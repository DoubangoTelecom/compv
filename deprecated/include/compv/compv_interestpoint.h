/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_INTERESTPOINT_H_)
#define _COMPV_INTERESTPOINT_H_

#include "compv/compv_config.h"
#include "compv/compv_box.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_API CompVBoxInterestPoint : public CompVBox<CompVInterestPoint >
{
protected:
    CompVBoxInterestPoint(size_t nCapacity = 0, bool bLockable = false);
public:
    virtual ~CompVBoxInterestPoint();
    COMPV_ERROR_CODE sortByStrength(); // multithreaded sorting
    COMPV_ERROR_CODE retainBest(size_t count);
    COMPV_ERROR_CODE eraseTooCloseToBorder(int32_t img_width, int32_t img_height, int32_t border_size);
    static COMPV_ERROR_CODE newObj(CompVPtr<CompVBoxInterestPoint* >* box, size_t nCapacity = 0, bool bLockable = false);
};

void CompVInterestPointScaleAndRoundAndGetAngleSinCos(COMPV_ALIGNED(x) const float* xf, COMPV_ALIGNED(x) const float *yf, COMPV_ALIGNED(x) const float *sf, COMPV_ALIGNED(x) const float* angleInDegree, COMPV_ALIGNED(x) int32_t* xi, COMPV_ALIGNED(x) int32_t* yi, COMPV_ALIGNED(x) float* cos, COMPV_ALIGNED(x) float* sin, compv_scalar_t count);

COMPV_NAMESPACE_END()

#endif /* _COMPV_INTERESTPOINT_H_ */
