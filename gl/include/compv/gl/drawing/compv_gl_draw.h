/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_GL_DRAWING_DRAW_H_)
#define _COMPV_GL_DRAWING_DRAW_H_

#include "compv/gl/compv_gl_config.h"
#include "compv/gl/compv_gl_headers.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/gl/compv_gl_program.h"
#include "compv/gl/compv_gl_mvp.h"
#include "compv/base/compv_bind.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

#define COMPV_GL_DRAW_AUTOBIND(draw) COMPV_AUTOBIND(CompVGLDraw, (draw))

COMPV_OBJECT_DECLARE_PTRS(GLDraw)

class CompVGLDraw : public CompVObj, public CompVBind
{
protected:
	CompVGLDraw(const std::string& strProgramVertexData, const std::string& strProgramFragmentData, bool bMVP = false);
public:
	virtual ~CompVGLDraw();

	COMPV_INLINE GLuint vao()const {
		return m_uNameVAO;
	}
	COMPV_INLINE GLuint vbo()const {
		return m_uNameVBO;
	}
	COMPV_INLINE CompVGLProgramPtr program() {
		return m_ptrProgram;
	}
	COMPV_INLINE CompVGLMVPPtr mvp() {
		return m_ptrMVP;
	}
	
protected:
	virtual COMPV_ERROR_CODE setOrtho(float left, float right, float bottom, float top, float zNear, float zFar);
	virtual COMPV_ERROR_CODE scale(float sx, float sy, float sz = 1.f);
	virtual COMPV_ERROR_CODE bind() override /*Overrides(CompVBind)*/;
	virtual COMPV_ERROR_CODE unbind() override /*Overrides(CompVBind)*/;

private:
	COMPV_ERROR_CODE init();
	COMPV_ERROR_CODE deInit();

private:
	bool m_bInitialized;
	bool m_bMVP;
	GLuint m_uNameVAO;
	GLuint m_uNameVBO;
	GLuint m_uNamePrgUnifMVP;
	CompVGLProgramPtr m_ptrProgram;
	CompVGLMVPPtr m_ptrMVP;
	std::string m_strProgramVertexData;
	std::string m_strProgramFragmentData;
};

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

#endif /* _COMPV_GL_DRAWING_DRAW_H_ */
