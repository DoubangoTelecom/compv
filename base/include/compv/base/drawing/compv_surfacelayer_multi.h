/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_DRAWING_SURFACE_LAYER_MULTI_H_)
#define _COMPV_BASE_DRAWING_SURFACE_LAYER_MULTI_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/drawing/compv_surfacelayer.h"

COMPV_NAMESPACE_BEGIN()

COMPV_OBJECT_DECLARE_PTRS(MultiSurfaceLayer)

class COMPV_BASE_API CompVMultiSurfaceLayer : public CompVSurfaceLayer
{
protected:
    CompVMultiSurfaceLayer();
public:
    virtual ~CompVMultiSurfaceLayer();

    virtual COMPV_ERROR_CODE addSurface(CompVSurfacePtrPtr surface, size_t width, size_t height) = 0;
    virtual COMPV_ERROR_CODE removeSurface(const CompVSurfacePtr surface) = 0;

private:
    COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)

    COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_DRAWING_SURFACE_LAYER_MULTI_H_ */
