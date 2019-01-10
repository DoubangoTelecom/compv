/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_DRAWING_SURFACE_H_)
#define _COMPV_BASE_DRAWING_SURFACE_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/drawing/compv_canvas.h"
#include "compv/base/drawing/compv_mvp.h"
#include "compv/base/drawing/compv_renderer.h"
#include "compv/base/drawing/compv_viewport.h"

COMPV_NAMESPACE_BEGIN()

typedef long compv_surface_id_t;

class CompVWindow;

COMPV_OBJECT_DECLARE_PTRS(Surface)

class COMPV_BASE_API CompVSurface : public CompVObj
{
protected:
    CompVSurface(size_t width, size_t height);
public:
    virtual ~CompVSurface();

    COMPV_INLINE compv_surface_id_t id()const {
        return m_nId;
    }
    COMPV_INLINE size_t width()const {
        return m_nWidth;
    }
    COMPV_INLINE size_t height()const {
        return m_nHeight;
    }
    COMPV_INLINE bool isActive()const {
        return m_bActive;
    }
    COMPV_INLINE CompVViewportPtr viewport()const {
        return m_ptrViewport;
    }
    virtual COMPV_ERROR_CODE activate() {
        m_bActive = true;
        return COMPV_ERROR_CODE_S_OK;
    }
    virtual COMPV_ERROR_CODE deActivate() {
        m_bActive = false;
        return COMPV_ERROR_CODE_S_OK;
    }

    virtual bool isGLEnabled()const = 0;
    virtual COMPV_ERROR_CODE setMVP(CompVMVPPtr mvp) = 0;
    virtual COMPV_ERROR_CODE setViewport(CompVViewportPtr viewport);
    virtual CompVRendererPtr renderer() = 0;
    virtual CompVCanvasPtr canvas() = 0;
	virtual CompVCanvasPtr requestCanvas(size_t width = 0, size_t height = 0) = 0;
    virtual COMPV_ERROR_CODE drawImage(const CompVMatPtr& mat, const CompVViewportPtr& viewport = nullptr) = 0;

    static COMPV_ERROR_CODE newObj(CompVSurfacePtrPtr surface, const CompVWindow* window);

protected:
    COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
    size_t m_nWidth;
    size_t m_nHeight;
    CompVViewportPtr m_ptrViewport;
    COMPV_VS_DISABLE_WARNINGS_END()

private:
    COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
    static compv_surface_id_t s_nSurfaceId;
    compv_surface_id_t m_nId;
    bool m_bActive;
    COMPV_VS_DISABLE_WARNINGS_END()

};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_DRAWING_SURFACE_H_ */
