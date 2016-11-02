/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_DRAWING_RENDERER_H_)
#define _COMPV_DRAWING_RENDERER_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_obj.h"
#include "compv/base/compv_mat.h"

#include <string>

COMPV_NAMESPACE_BEGIN()

typedef long compv_renderer_id_t;

class CompVSurface;

class CompVRenderer;
typedef CompVPtr<CompVRenderer* > CompVRendererPtr;
typedef CompVRendererPtr* CompVRendererPtrPtr;

class COMPV_DRAWING_API CompVRenderer : public CompVObj
{
protected:
	CompVRenderer(COMPV_PIXEL_FORMAT ePixelFormat);
public:
	virtual ~CompVRenderer();
	virtual COMPV_INLINE const char* getObjectId() {
		return "CompVRenderer";
	};
	
	COMPV_INLINE COMPV_PIXEL_FORMAT getPixelFormat() { return m_ePixelFormat; }
	COMPV_INLINE compv_renderer_id_t getId() { return m_nId; }

	virtual bool isGLEnabled()const = 0;
	virtual COMPV_ERROR_CODE render(CompVMatPtr mat) = 0; // FIXME(dmi): rename to drawImage()

	static COMPV_ERROR_CODE newObj(CompVRendererPtrPtr renderer, COMPV_PIXEL_FORMAT ePixelFormat, const CompVSurface* surface);

private:
	COMPV_DISABLE_WARNINGS_BEGIN(4251 4267)
	static compv_renderer_id_t s_nRendererId;
	compv_renderer_id_t m_nId;
	COMPV_PIXEL_FORMAT m_ePixelFormat;
	COMPV_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_DRAWING_RENDERER_H_ */