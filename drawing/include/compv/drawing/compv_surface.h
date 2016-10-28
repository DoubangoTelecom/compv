/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_DRAWING_SURFACE_H_)
#define _COMPV_DRAWING_SURFACE_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_obj.h"
#include "compv/drawing/compv_renderer.h"
#include "compv/drawing/compv_canvas.h"

#include <string>

COMPV_NAMESPACE_BEGIN()

typedef long compv_surface_id_t;

class CompVSurface;
typedef CompVPtr<CompVSurface* > CompVSurfacePtr;
typedef CompVSurfacePtr* CompVSurfacePtrPtr;

class COMPV_DRAWING_API CompVSurface : public CompVObj
{
protected:
	CompVSurface();
public:
	virtual ~CompVSurface();
	virtual COMPV_INLINE const char* getObjectId() {
		return "CompVSurface";
	};

	COMPV_INLINE compv_surface_id_t getId() { return m_nId; }

	static COMPV_ERROR_CODE newObj(CompVSurfacePtrPtr surface);

protected:
	COMPV_INLINE CompVRendererPtr getRenderer() { return m_ptrRenderer; }
	COMPV_INLINE CompVCanvasPtr getCanvas() { return m_ptrCanvas; }

private:
	COMPV_DISABLE_WARNINGS_BEGIN(4251 4267)
	static compv_surface_id_t s_nSurfaceId;
	compv_surface_id_t m_nId;
	CompVRendererPtr m_ptrRenderer;
	CompVCanvasPtr m_ptrCanvas;
	COMPV_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_DRAWING_SURFACE_H_ */
