/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_GL_DRAWING_DRAW_TEXTS_H_)
#define _COMPV_GL_DRAWING_DRAW_TEXTS_H_

#include "compv/gl/compv_gl_config.h"
#include "compv/gl/compv_gl_headers.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/gl/compv_gl_common.h"
#include "compv/gl/compv_gl_freetype.h"
#include "compv/gl/drawing/compv_gl_draw.h"
#include "compv/base/compv_mat.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

COMPV_OBJECT_DECLARE_PTRS(GLDrawTexts)

class CompVGLDrawTexts : public CompVGLDraw
{
protected:
	CompVGLDrawTexts();
public:
	virtual ~CompVGLDrawTexts();
	COMPV_OBJECT_GET_ID(CompVGLDrawTexts);

	COMPV_ERROR_CODE texts(const CompVStringVector& texts, const CompVPointFloat32Vector& positions, const CompVDrawingOptions* options = nullptr);

	static COMPV_ERROR_CODE newObj(CompVGLDrawTextsPtrPtr drawTexts);

private:
#if HAVE_FREETYPE
	COMPV_ERROR_CODE fillAtlas(const CompVStringVector& texts, const CompVPointFloat32Vector& positions, CompVMatPtr& ptrAtlas, CompVMatPtr& ptrBitmaps);
#endif /* HAVE_FREETYPE */

private:
	GLint m_fboWidth;
	GLint m_fboHeight;
	GLuint m_uTextureAtlas;
#if HAVE_FREETYPE
	FT_Face m_face;
#endif /* HAVE_FREETYPE */
};

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

#endif /* _COMPV_GL_DRAWING_DRAW_TEXTS_H_ */
