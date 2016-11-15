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
#include "compv/drawing/compv_canvas.h"
#include "compv/drawing/compv_mvp.h"
#include "compv/drawing/compv_common.h"

#include <string>

COMPV_NAMESPACE_BEGIN()

typedef long compv_surface_id_t;

class CompVWindow;

class CompVSurface;
typedef CompVPtr<CompVSurface* > CompVSurfacePtr;
typedef CompVSurfacePtr* CompVSurfacePtrPtr;

class COMPV_DRAWING_API CompVSurface : public CompVObj
{
protected:
	CompVSurface(int width, int height);
public:
	virtual ~CompVSurface();

	COMPV_INLINE compv_surface_id_t getId()const { return m_nId; }
	COMPV_INLINE int getWidth()const { return m_nWidth; }
	COMPV_INLINE int getHeight()const { return m_nHeight; }
	
	virtual CompVMVPPtr MVP() = 0;
	virtual bool isGLEnabled()const = 0;
	virtual COMPV_ERROR_CODE drawImage(CompVMatPtr mat) = 0;
	virtual COMPV_ERROR_CODE drawText(const void* textPtr, size_t textLengthInBytes) = 0;

	static COMPV_ERROR_CODE newObj(CompVSurfacePtrPtr surface, const CompVWindow* window);

protected:
	COMPV_INLINE CompVCanvasPtr getCanvas() { return m_ptrCanvas; } // FIXME(dmi): remove

private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
	static compv_surface_id_t s_nSurfaceId;
	compv_surface_id_t m_nId;
	CompVCanvasPtr m_ptrCanvas;  // FIXME(dmi): remove
	int m_nWidth;
	int m_nHeight;

	COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_DRAWING_SURFACE_H_ */
