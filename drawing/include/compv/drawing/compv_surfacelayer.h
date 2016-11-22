/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_DRAWING_SURFACE_LAYER_H_)
#define _COMPV_DRAWING_SURFACE_LAYER_H_

#include "compv/drawing/compv_config.h"
#include "compv/drawing/compv_common.h"
#include "compv/base/compv_obj.h"
#include "compv/drawing/compv_surface.h"

COMPV_NAMESPACE_BEGIN()

typedef long compv_surfacelayer_id_t;

class CompVSurfaceLayer;
typedef CompVPtr<CompVSurfaceLayer* > CompVSurfaceLayerPtr;
typedef CompVSurfaceLayerPtr* CompVSurfaceLayerPtrPtr;

class COMPV_DRAWING_API CompVSurfaceLayer : public CompVObj
{
protected:
	CompVSurfaceLayer();
public:
	virtual ~CompVSurfaceLayer();
	COMPV_INLINE compv_surfacelayer_id_t id()const { return m_nId; }

	virtual COMPV_ERROR_CODE blit() = 0;

private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
	static compv_surfacelayer_id_t s_nSurfaceLayerId;
	compv_surfacelayer_id_t m_nId;
	COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_DRAWING_SURFACE_LAYER_H_ */
