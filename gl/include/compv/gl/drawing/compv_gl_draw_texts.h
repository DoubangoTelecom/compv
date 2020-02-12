/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
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

#include <map>

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

struct CompVFreeTypeChar {
	GLuint left;
	GLuint top;
	GLuint width;
	GLuint height;
	GLuint advance_x;
	GLuint advance_y;
	uint8_t* mem = nullptr;
public:
	virtual ~CompVFreeTypeChar() {
		CompVMem::free(reinterpret_cast<void**>(&mem));
	}
};
typedef std::map<unsigned long, CompVFreeTypeChar> CompVFreeTypeCache;

COMPV_OBJECT_DECLARE_PTRS(GLDrawTexts)

class CompVGLDrawTexts : public CompVGLDraw
{
protected:
	CompVGLDrawTexts();
public:
	virtual ~CompVGLDrawTexts();
	COMPV_OBJECT_GET_ID(CompVGLDrawTexts);

	COMPV_ERROR_CODE texts(const CompVVecString& texts, const CompVPointFloat32Vector& positions, const CompVDrawingOptions* options = nullptr);

	static COMPV_ERROR_CODE newObj(CompVGLDrawTextsPtrPtr drawTexts);

private:
#if HAVE_FREETYPE
	COMPV_ERROR_CODE freeTypeAddChar(unsigned long charcode);
	COMPV_ERROR_CODE freeTypeCreateFace(const std::string fontFullPath, size_t fontSize);
	COMPV_ERROR_CODE freeTypeFillAtlas(const bool bUtf8, const CompVVecString& texts, const CompVPointFloat32Vector& positions, CompVMatPtr& ptrAtlas, CompVMatPtr& ptrBoxes, size_t& numChars);
#endif /* HAVE_FREETYPE */

private:
#if HAVE_FREETYPE
	GLint m_fboWidth;
	GLint m_fboHeight;
	GLuint m_uTextureAtlas;
	std::string m_fontFullPath;
	size_t m_fontSize;
	CompVBufferPtr m_ptrFaceBuffer;
	CompVFreeTypeCache m_freeTypeCache;
	FT_Face m_face;
#endif /* HAVE_FREETYPE */
};

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

#endif /* _COMPV_GL_DRAWING_DRAW_TEXTS_H_ */
