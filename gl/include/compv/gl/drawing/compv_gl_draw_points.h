/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_GL_DRAWING_DRAW_POINTS_H_)
#define _COMPV_GL_DRAWING_DRAW_POINTS_H_

#include "compv/gl/compv_gl_config.h"
#include "compv/gl/compv_gl_headers.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/gl/drawing/compv_gl_draw.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

COMPV_OBJECT_DECLARE_PTRS(GLDrawPoints)

class CompVGLDrawPoints : public CompVGLDraw
{
protected:
	CompVGLDrawPoints();
public:
	virtual ~CompVGLDrawPoints();
	COMPV_OBJECT_GET_ID(CompVGLDrawPoints);

	COMPV_ERROR_CODE process(const GLfloat* xy, GLsizei count);

	static COMPV_ERROR_CODE newObj(CompVGLDrawPointsPtrPtr drawPoints);

private:

private:
	
};

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

#endif /* _COMPV_GL_DRAWING_DRAW_POINTS_H_ */
