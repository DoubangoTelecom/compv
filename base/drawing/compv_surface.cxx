/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/drawing/compv_surface.h"

COMPV_NAMESPACE_BEGIN()

compv_surface_id_t CompVSurface::s_nSurfaceId = 0;

CompVSurface::CompVSurface(size_t width, size_t height)
    : CompVObj()
    , m_nWidth(width)
    , m_nHeight(height)
	, m_nId(compv_atomic_add(&CompVSurface::s_nSurfaceId, 1))
    , m_bActive(true)
{
}

CompVSurface::~CompVSurface()
{

}

COMPV_ERROR_CODE CompVSurface::setViewport(CompVViewportPtr viewport)
{
    COMPV_CHECK_EXP_RETURN(!viewport, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    m_ptrViewport = viewport;
    return COMPV_ERROR_CODE_S_OK;
}


COMPV_NAMESPACE_END()

