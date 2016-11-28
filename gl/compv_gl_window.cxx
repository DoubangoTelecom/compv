/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/gl/compv_gl_window.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/gl/compv_gl_utils.h"
#include "compv/gl/compv_gl_info.h"
#include "compv/gl/compv_gl_func.h"

#define COMPV_THIS_CLASS_NAME "CompVGLWindow"

COMPV_NAMESPACE_BEGIN()

// FIXME(dmi): remove layers when window is closed


//
//	CompVGLWindow
//

CompVGLWindow::CompVGLWindow(size_t width, size_t height, const char* title)
	: CompVWindowPriv(width, height, title)
	, CompVLock()
	, m_bDrawing(false)
{
}

CompVGLWindow::~CompVGLWindow()
{
	
}

bool CompVGLWindow::isInitialized()const
{
	return CompVLock::isInitialized(); // base class initialization check
}

COMPV_OVERRIDE_IMPL1("CompVWindow", bool, CompVGLWindow::isGLEnabled)() const
{
	return true;
}

COMPV_OVERRIDE_IMPL0("CompVWindow", CompVGLWindow::close)()
{
	// Closing layers require a valid GL context
	// -> set GL context
	if (context()) {
		COMPV_CHECK_CODE_RETURN(context()->makeCurrent());
	}

	// Close all layers
	for (std::map<compv_surfacelayer_id_t, CompVGLSingleSurfaceLayerPtr>::iterator it = m_mapSingleSurfaceLayers.begin(); it != m_mapSingleSurfaceLayers.end(); ++it) {
		COMPV_CHECK_CODE_NOP(it->second->close());
	}
	for (std::map<compv_surfacelayer_id_t, CompVGLMatchingSurfaceLayerPtr>::iterator it = m_mapMatchingSurfaceLayers.begin(); it != m_mapMatchingSurfaceLayers.end(); ++it) {
		COMPV_CHECK_CODE_NOP(it->second->close());
	}
	for (std::map<compv_surfacelayer_id_t, CompVGLMultiSurfaceLayerPtr>::iterator it = m_mapMultiSurfaceLayers.begin(); it != m_mapMultiSurfaceLayers.end(); ++it) {
		COMPV_CHECK_CODE_NOP(it->second->close());
	}

	// Unset context
	if (context()) {
		COMPV_CHECK_CODE_NOP(context()->unmakeCurrent());
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_OVERRIDE_IMPL0("CompVWindow", CompVGLWindow::beginDraw)()
{
	CompVAutoLock<CompVGLWindow>(this);
	COMPV_CHECK_EXP_RETURN(isClosed(), COMPV_ERROR_CODE_W_WINDOW_CLOSED);
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	COMPV_CHECK_EXP_BAIL(m_bDrawing || !context(), (err = COMPV_ERROR_CODE_E_INVALID_STATE));
	COMPV_CHECK_CODE_BAIL(err = context()->makeCurrent());
	COMPV_CHECK_CODE_BAIL(err = CompVGLInfo::gather()); // now that the context is set gather the info and set the supported extensions
	
	COMPV_glBindFramebuffer(GL_FRAMEBUFFER, kCompVGLNameSystemFrameBuffer);
	COMPV_glBindRenderbuffer(GL_RENDERBUFFER, kCompVGLNameSystemRenderBuffer);
	// TODO(dmi): 'GL_DEPTH_TEST' not working with skia:  we need to use 'glPushAttrib(GL_ALL_ATTRIB_BITS); glPopAttrib();' before/after canvas drawing
	// 'GL_DEPTH_TEST' is needed for 3D projection
	COMPV_glDisable(GL_DEPTH_TEST);
	COMPV_glDisable(GL_BLEND);
	COMPV_glViewport(0, 0, static_cast<GLsizei>(CompVWindow::width()), static_cast<GLsizei>(CompVWindow::height())); // FIXME: width and height must be dynamic
	COMPV_glClearColor(0.f, 0.f, 0.f, 1.f);
	COMPV_glClearStencil(0);
	COMPV_glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	
	// Clear surfaces
	//for (std::vector<CompVGLSurfacePtr >::iterator it = m_vecGLSurfaces.begin(); it != m_vecGLSurfaces.end(); ++it) {
	//	COMPV_CHECK_CODE_BAIL(err = (*it)->beginDraw());
	//}

	m_bDrawing = true;
bail:
	if (COMPV_ERROR_CODE_IS_NOK(err)) {
		COMPV_CHECK_CODE_NOP(context()->unmakeCurrent());
	}
	return err;
}

COMPV_OVERRIDE_IMPL0("CompVWindow", CompVGLWindow::endDraw)()
{
	CompVAutoLock<CompVGLWindow>(this);
	COMPV_CHECK_EXP_RETURN(isClosed(), COMPV_ERROR_CODE_W_WINDOW_CLOSED);
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	COMPV_CHECK_EXP_BAIL(!m_bDrawing || !context(), (err = COMPV_ERROR_CODE_E_INVALID_STATE));
	COMPV_CHECK_EXP_BAIL(!CompVGLContext::isSet(), (err = COMPV_ERROR_CODE_E_GL_NO_CONTEXT));

	// Swap (aka 'present' the final redering to the window, means switch front/back buffers)
	COMPV_CHECK_CODE_BAIL(err = context()->swapBuffers());

bail:
	m_bDrawing = false;
	COMPV_CHECK_CODE_NOP(context()->unmakeCurrent());
	return err;
}

COMPV_OVERRIDE_IMPL0("CompVWindow", CompVGLWindow::addSingleLayerSurface)(CompVSingleSurfaceLayerPtrPtr layer)
{
	CompVAutoLock<CompVGLWindow>(this);
	COMPV_CHECK_EXP_RETURN(!layer, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVGLSingleSurfaceLayerPtr layer_;
	COMPV_CHECK_CODE_RETURN(CompVGLSingleSurfaceLayer::newObj(&layer_, CompVWindow::width(), CompVWindow::height()));
	m_mapSingleSurfaceLayers[layer_->id()] = layer_;
	*layer = *layer_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_OVERRIDE_IMPL0("CompVWindow", CompVGLWindow::removeSingleLayerSurface)(const CompVSingleSurfaceLayerPtr& layer)
{
	CompVAutoLock<CompVGLWindow>(this);
	COMPV_CHECK_EXP_RETURN(!layer, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	m_mapSingleSurfaceLayers.erase(layer->id());
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_OVERRIDE_IMPL0("CompVWindow", CompVGLWindow::addMatchingLayerSurface)(CompVMatchingSurfaceLayerPtrPtr layer)
{
	CompVAutoLock<CompVGLWindow>(this);
	COMPV_CHECK_EXP_RETURN(!layer, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVGLMatchingSurfaceLayerPtr layer_;
	COMPV_CHECK_CODE_RETURN(CompVGLMatchingSurfaceLayer::newObj(&layer_, CompVWindow::width(), CompVWindow::height()));
	m_mapMatchingSurfaceLayers[layer_->id()] = layer_;
	*layer = *layer_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_OVERRIDE_IMPL0("CompVWindow", CompVGLWindow::removeMatchingLayerSurface)(const CompVMatchingSurfaceLayerPtr& layer)
{
	CompVAutoLock<CompVGLWindow>(this);
	COMPV_CHECK_EXP_RETURN(!layer, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	m_mapMatchingSurfaceLayers.erase(layer->id());
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_OVERRIDE_IMPL0("CompVWindow", CompVGLWindow::addMultiLayerSurface)(CompVMultiSurfaceLayerPtrPtr layer)
{
	CompVAutoLock<CompVGLWindow>(this);
	COMPV_CHECK_EXP_RETURN(!layer, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVGLMultiSurfaceLayerPtr layer_;
	COMPV_CHECK_CODE_RETURN(CompVGLMultiSurfaceLayer::newObj(&layer_, CompVWindow::width(), CompVWindow::height()));
	m_mapMultiSurfaceLayers[layer_->id()] = layer_;
	*layer = *layer_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_OVERRIDE_IMPL0("CompVWindow", CompVGLWindow::removeMultiLayerSurface)(const CompVMultiSurfaceLayerPtr& layer)
{
	CompVAutoLock<CompVGLWindow>(this);
	COMPV_CHECK_EXP_RETURN(!layer, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	m_mapMultiSurfaceLayers.erase(layer->id());
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_OVERRIDE_IMPL0("CompVWindowPriv", CompVGLWindow::priv_updateSize)(size_t newWidth, size_t newHeight)
{
	CompVAutoLock<CompVGLWindow>(this);
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	
	COMPV_CHECK_CODE_BAIL(err = context()->makeCurrent());

	CompVWindow::m_nWidth = newWidth;
	CompVWindow::m_nHeight = newHeight;

	// COMPV_CHECK_CODE_BAIL(err = beginDraw()); // TODO(dmi): deadlock
	//for (std::vector<CompVGLSurfacePtr >::iterator it = m_vecGLSurfaces.begin(); it != m_vecGLSurfaces.end(); ++it) {
	//	COMPV_CHECK_CODE_BAIL(err = (*it)->updateSize(newWidth, newHeight));
	//}
	for (std::map<compv_surfacelayer_id_t, CompVGLSingleSurfaceLayerPtr>::iterator it = m_mapSingleSurfaceLayers.begin(); it != m_mapSingleSurfaceLayers.end(); ++it) {
		COMPV_CHECK_CODE_BAIL(err = it->second->updateSize(newWidth, newHeight));
	}

	for (std::map<compv_surfacelayer_id_t, CompVGLMatchingSurfaceLayerPtr>::iterator it = m_mapMatchingSurfaceLayers.begin(); it != m_mapMatchingSurfaceLayers.end(); ++it) {
		COMPV_CHECK_CODE_BAIL(err = it->second->updateSize(newWidth, newHeight));
	}

	for (std::map<compv_surfacelayer_id_t, CompVGLMultiSurfaceLayerPtr>::iterator it = m_mapMultiSurfaceLayers.begin(); it != m_mapMultiSurfaceLayers.end(); ++it) {
		COMPV_CHECK_CODE_BAIL(err = it->second->updateSize(newWidth, newHeight));
	}

	//COMPV_CHECK_CODE_BAIL(err = endDraw()); // blit all surfaces using up-to-data sizes

	for (std::map<compv_windowlistener_id_t, CompVWindowListenerPtr>::iterator it = m_mapListeners.begin(); it != m_mapListeners.end(); ++it) {
		COMPV_CHECK_CODE_BAIL(err = it->second->onSizeChanged(newWidth, newHeight));
	}
	
bail:
	COMPV_CHECK_CODE_NOP(err = context()->unmakeCurrent());
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */
