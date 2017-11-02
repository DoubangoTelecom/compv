/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_DRAWING_SURFACE_LAYER_SINGLE_H_)
#define _COMPV_BASE_DRAWING_SURFACE_LAYER_SINGLE_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/drawing/compv_surfacelayer.h"

COMPV_NAMESPACE_BEGIN()

COMPV_OBJECT_DECLARE_PTRS(SingleSurfaceLayer)

class COMPV_BASE_API CompVSingleSurfaceLayer : public CompVSurfaceLayer
{
protected:
    CompVSingleSurfaceLayer();
public:
    virtual ~CompVSingleSurfaceLayer();

private:
    COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)

    COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_DRAWING_SURFACE_LAYER_SINGLE_H_ */
