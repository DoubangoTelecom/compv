/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/drawing/opengl/compv_renderer_gl.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/drawing/compv_drawing.h"
#include "compv/drawing/opengl/compv_utils_gl.h"
#include "compv/drawing/opengl/compv_renderer_gl_rgb.h"

COMPV_NAMESPACE_BEGIN()

CompVRendererGL::CompVRendererGL(COMPV_PIXEL_FORMAT ePixelFormat)
	: CompVRenderer(ePixelFormat)
	, m_uVertexBuffer(0)
	, m_uIndiceBuffer(0)
{
	
}

CompVRendererGL::~CompVRendererGL()
{
	COMPV_CHECK_CODE_ASSERT(deInitBuffers());
}

COMPV_ERROR_CODE CompVRendererGL::initBuffers()
{
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::haveCurrentContext(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);
	// TODO(dmi): use GLUtils
	if (!m_uVertexBuffer) {
		glGenBuffers(1, &m_uVertexBuffer);
		if (!m_uVertexBuffer) {
			std::string err;
			COMPV_CHECK_CODE_BAIL(CompVUtilsGL::getLastError(&err));
			if (!err.empty()) {
				COMPV_DEBUG_ERROR("Failed to create vertex buffer: %s", err.c_str());
				COMPV_CHECK_CODE_BAIL(COMPV_ERROR_CODE_E_GL);
			}
		}
		glBindBuffer(GL_ARRAY_BUFFER, m_uVertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(CompVGLTexture2DVertices), CompVGLTexture2DVertices, GL_STATIC_DRAW);
	}
	if (!m_uIndiceBuffer) {
		glGenBuffers(1, &m_uIndiceBuffer);
		if (!m_uIndiceBuffer) {
			std::string err;
			COMPV_CHECK_CODE_BAIL(CompVUtilsGL::getLastError(&err));
			if (!err.empty()) {
				COMPV_DEBUG_ERROR("Failed to create index buffer: %s", err.c_str());
				COMPV_CHECK_CODE_BAIL(COMPV_ERROR_CODE_E_GL);
			}
		}
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uIndiceBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(CompVGLTexture2DIndices), CompVGLTexture2DIndices, GL_STATIC_DRAW);
	}
	return COMPV_ERROR_CODE_S_OK;

bail:
	COMPV_CHECK_CODE_RETURN(deInitBuffers());
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVRendererGL::deInitBuffers()
{
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::haveCurrentContext(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);
	if (m_uVertexBuffer) {
		glDeleteBuffers(1, &m_uVertexBuffer);
		m_uVertexBuffer = 0;
	}
	if (m_uIndiceBuffer) {
		glDeleteBuffers(1, &m_uIndiceBuffer);
		m_uIndiceBuffer = 0;
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVRendererGL::newObj(CompVRendererGLPtrPtr glRenderer, COMPV_PIXEL_FORMAT ePixelFormat, const CompVSurface* surface)
{
	COMPV_CHECK_CODE_RETURN(CompVDrawing::init());
	COMPV_CHECK_EXP_RETURN(!glRenderer || !surface, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_EXP_RETURN(!surface->isGLEnabled(), COMPV_ERROR_CODE_E_INVALID_PARAMETER); // Make sure both the window and surface have an GL context attached
	
	CompVRendererGLPtr glRenderer_;
	switch (ePixelFormat) {
	case COMPV_PIXEL_FORMAT_R8G8B8:
	{
		CompVRendererGLRgbPtr glRgbRenderer_;
		COMPV_CHECK_CODE_RETURN(CompVRendererGLRgb::newObj(&glRgbRenderer_, ePixelFormat, surface));
		glRenderer_ = *glRgbRenderer_;
		break;
	}
	default:
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT);
		break;
	}
	COMPV_CHECK_CODE_RETURN(glRenderer_->initBuffers());
	COMPV_CHECK_EXP_RETURN(!(*glRenderer = glRenderer_), COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */
