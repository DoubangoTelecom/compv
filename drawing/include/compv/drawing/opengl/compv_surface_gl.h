/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_DRAWING_SURFACE_GL_H_)
#define _COMPV_DRAWING_SURFACE_GL_H_

#include "compv/base/compv_config.h"
#include "compv/drawing/opengl/compv_headers_gl.h"
#if defined(HAVE_OPENGL) ||defined(HAVE_OPENGLES)
#include "compv/base/compv_common.h"
#include "compv/base/compv_obj.h"
#include "compv/drawing/compv_surface.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class CompVWindow;

class CompVSurfaceGL;
typedef CompVPtr<CompVSurfaceGL* > CompVSurfaceGLPtr;
typedef CompVSurfaceGLPtr* CompVSurfaceGLPtrPtr;

class CompVSurfaceGL : public CompVSurface
{
protected:
	CompVSurfaceGL(int width, int height);
public:
	virtual ~CompVSurfaceGL();
	virtual COMPV_INLINE const char* getObjectId() {
		return "CompVSurfaceGL";
	};

	virtual bool isGLEnabled()const { return true; };

	virtual COMPV_ERROR_CODE beginDraw();
	virtual COMPV_ERROR_CODE endDraw();
	virtual COMPV_ERROR_CODE drawImage(CompVMatPtr mat);
	virtual COMPV_ERROR_CODE drawText(const void* textPtr, size_t textLengthInBytes);

	static COMPV_ERROR_CODE newObj(CompVSurfaceGLPtrPtr glSurface, const CompVWindow* window);

private:
	COMPV_ERROR_CODE initFrameBuffer();
	COMPV_ERROR_CODE deInitFrameBuffer();

private:
	COMPV_DISABLE_WARNINGS_BEGIN(4251 4267)
	GLuint m_uNameFrameBuffer;
	GLuint m_uNameTexture;
	GLuint m_uNameDepthStencil;
	COMPV_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) ||defined(HAVE_OPENGLES) */

#endif /* _COMPV_DRAWING_SURFACE_GL_H_ */
