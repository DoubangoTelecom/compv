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

class CompVWindowGL;
typedef CompVPtr<CompVWindowGL* > CompVWindowGLPtr;
typedef CompVWindowGLPtr* CompVWindowGLPtrPtr;

class CompVWindowGL : public CompVWindow, public CompVLock
{
protected:
	CompVWindowGL(int width, int height, const char* title);
public:
	virtual ~CompVWindowGL();
	virtual COMPV_INLINE const char* getObjectId() {
		return "CompVWindowGL";
	};

	bool isInitialized()const;

	// Overrides(CompVWindow)
	virtual bool isDrawing()const { return m_bDrawing;  }
	virtual COMPV_ERROR_CODE beginDraw();
	virtual COMPV_ERROR_CODE endDraw();
	virtual size_t numSurface();
	virtual COMPV_ERROR_CODE removeAllSurfaces();
	virtual COMPV_ERROR_CODE addSurface();
	virtual COMPV_ERROR_CODE removeSurface(size_t index);
	virtual CompVSurfacePtr surface(size_t index = 0);

protected:
	virtual COMPV_ERROR_CODE makeGLContextCurrent() = 0;
	virtual COMPV_ERROR_CODE unmakeGLContextCurrent() = 0;
	virtual COMPV_ERROR_CODE swapGLBuffers() = 0;

private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
	bool m_bDrawing;
	std::vector<CompVSurfaceGLPtr > m_vecGLSurfaces;
	COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

#endif /* _COMPV_DRAWING_OPENGL_WINDOW_GL_H_ */
