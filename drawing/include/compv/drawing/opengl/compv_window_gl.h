/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_DRAWING_OPENGL_WINDOW_GL_H_)
#define _COMPV_DRAWING_OPENGL_WINDOW_GL_H_

#include "compv/base/compv_config.h"
#include "compv/drawing/opengl/compv_headers_gl.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/drawing/opengl/compv_surface_gl.h"
#include "compv/drawing/opengl/compv_context_gl.h"
#include "compv/drawing/compv_window.h"
#include "compv/base/compv_obj.h"
#include "compv/base/compv_common.h"
#include "compv/base/parallel/compv_mutex.h"
#include "compv/base/compv_autolock.h"

#include <vector>

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

//
//	CompVWindowGL
//
class CompVWindowGL;
typedef CompVPtr<CompVWindowGL* > CompVWindowGLPtr;
typedef CompVWindowGLPtr* CompVWindowGLPtrPtr;

class CompVWindowGL : public CompVWindowPriv, public CompVLock
{
	friend class CompVWindowGLListener;
protected:
	CompVWindowGL(size_t width, size_t height, const char* title);
public:
	virtual ~CompVWindowGL();

	bool isInitialized()const;

	// Overrides(CompVWindow)
	virtual bool isGLEnabled()const { return true; }
	virtual bool isDrawing()const { return m_bDrawing;  }
	virtual COMPV_ERROR_CODE beginDraw();
	virtual COMPV_ERROR_CODE endDraw();
	virtual size_t numSurface();
	virtual COMPV_ERROR_CODE removeAllSurfaces();
	virtual COMPV_ERROR_CODE addSurface();
	virtual COMPV_ERROR_CODE removeSurface(size_t index);
	virtual CompVSurfacePtr surface(size_t index = 0);

	// Overrides(CompVWindowPriv)
	virtual COMPV_ERROR_CODE priv_updateSize(size_t newWidth, size_t newHeight);

protected:
	virtual CompVContextGLPtr context() = 0;

private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
	bool m_bDrawing;
	std::vector<CompVSurfaceGLPtr > m_vecGLSurfaces;
	COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

#endif /* _COMPV_DRAWING_OPENGL_WINDOW_GL_H_ */
