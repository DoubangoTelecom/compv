/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_DRAWING_SURFACE_LAYER_SINGLE_H_)
#define _COMPV_DRAWING_SURFACE_LAYER_SINGLE_H_

#include "compv/drawing/compv_config.h"
#include "compv/drawing/compv_common.h"
#include "compv/base/compv_obj.h"
#include "compv/drawing/compv_surfacelayer.h"

COMPV_NAMESPACE_BEGIN()

class CompVSingleSurfaceLayer;
typedef CompVPtr<CompVSingleSurfaceLayer* > CompVSingleSurfaceLayerPtr;
typedef CompVSingleSurfaceLayerPtr* CompVSingleSurfaceLayerPtrPtr;

class COMPV_DRAWING_API CompVSingleSurfaceLayer : public CompVSurfaceLayer
{
protected:
	CompVSingleSurfaceLayer();
public:
	virtual ~CompVSingleSurfaceLayer();

	// FIXME: cannot drawText to surface(), requires renderer -> rename 'CompVSingleSurfaceLayer' and create a multiSurface
	virtual CompVSurfacePtr surface() = 0;

private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)

		COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_DRAWING_SURFACE_LAYER_SINGLE_H_ */
