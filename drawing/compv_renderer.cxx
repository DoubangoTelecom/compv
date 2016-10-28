/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/drawing/compv_renderer.h"
#include "compv/drawing/compv_drawing.h"

COMPV_NAMESPACE_BEGIN()

compv_window_id_t CompVRenderer::s_nRendererId = 0;

CompVRenderer::CompVRenderer()
	: m_nId(compv_atomic_inc(&CompVRenderer::s_nRendererId))
{
}

CompVRenderer::~CompVRenderer()
{
	
}

COMPV_ERROR_CODE CompVRenderer::newObj(CompVRendererPtrPtr renderer)
{
	COMPV_CHECK_CODE_RETURN(CompVDrawing::init());
	COMPV_CHECK_EXP_RETURN(!renderer == NULL, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	*renderer = NULL;

	COMPV_CHECK_EXP_RETURN(!*renderer, COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

