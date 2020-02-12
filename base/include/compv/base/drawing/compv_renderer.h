/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_DRAWING_RENDERER_H_)
#define _COMPV_BASE_DRAWING_RENDERER_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_obj.h"
#include "compv/base/compv_mat.h"
#include "compv/base/drawing/compv_canvas.h"
#include "compv/base/drawing/compv_viewport.h"

COMPV_NAMESPACE_BEGIN()

typedef long compv_renderer_id_t;

COMPV_OBJECT_DECLARE_PTRS(Renderer)

class COMPV_BASE_API CompVRenderer : public CompVObj
{
protected:
    CompVRenderer(COMPV_SUBTYPE ePixelFormat);
public:
    virtual ~CompVRenderer();

    COMPV_INLINE COMPV_SUBTYPE pixelFormat()const {
        return m_ePixelFormat;
    }
    COMPV_INLINE compv_renderer_id_t id()const {
        return m_nId;
    }

    virtual bool isGLEnabled()const = 0;
    virtual COMPV_ERROR_CODE drawImage(const CompVMatPtr& mat, const CompVViewportPtr& viewport = nullptr) = 0;
    virtual CompVCanvasPtr canvas() = 0;

private:
    COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
	compv_renderer_id_t m_nId;
    COMPV_SUBTYPE m_ePixelFormat;
    static compv_renderer_id_t s_nRendererId;
    COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_DRAWING_RENDERER_H_ */