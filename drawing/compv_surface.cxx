/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/drawing/compv_surface.h"
#include "compv/drawing/compv_drawing.h"

COMPV_NAMESPACE_BEGIN()

compv_surface_id_t CompVSurface::s_nSurfaceId = 0;

CompVSurface::CompVSurface()
	: m_nId(compv_atomic_inc(&CompVSurface::s_nSurfaceId))
{
}

CompVSurface::~CompVSurface()
{

}

COMPV_ERROR_CODE CompVSurface::newObj(CompVSurfacePtrPtr surface)
{
	COMPV_CHECK_CODE_RETURN(CompVDrawing::init());
	COMPV_CHECK_EXP_RETURN(!surface == NULL, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	*surface = NULL;

	COMPV_CHECK_EXP_RETURN(!*surface, COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

