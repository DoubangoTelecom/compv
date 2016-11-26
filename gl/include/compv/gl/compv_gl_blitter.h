/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_GL_BLITTER_H_)
#define _COMPV_GL_BLITTER_H_

#include "compv/gl/compv_config.h"
#include "compv/gl/compv_gl_headers.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/base/compv_obj.h"
#include "compv/base/drawing/compv_mvp.h"
#include "compv/gl/compv_gl_common.h"
#include "compv/gl/compv_gl_vao.h"
#include "compv/gl/compv_gl_program.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class COMPV_GL_API CompVGLBlitter
{
protected:
	CompVGLBlitter();
public:
	virtual ~CompVGLBlitter();

	COMPV_INLINE size_t width()const { return m_nWidth; }
	COMPV_INLINE size_t height()const { return m_nHeight; }
	COMPV_INLINE size_t stride()const { return m_nStride; }
	COMPV_INLINE bool isToScreen()const { return m_bToScreen; }
	COMPV_INLINE GLuint indicesCount()const { return sizeof(kCompVGLTexture2DIndices) / sizeof(kCompVGLTexture2DIndices[0]); }
	COMPV_INLINE CompVGLProgramPtr program() { return m_ptrProgram; }
	
	
	virtual COMPV_ERROR_CODE bind();
	virtual COMPV_ERROR_CODE unbind();
	virtual COMPV_ERROR_CODE setMVP(CompVMVPPtr mvp);
	virtual COMPV_ERROR_CODE setSize(size_t width, size_t height, size_t stride);

protected:
	virtual COMPV_ERROR_CODE init(size_t width, size_t height, size_t stride, const std::string& prgVertexData, const std::string& prgFragData, bool bMVP = false, bool bToScreen = false);
	virtual COMPV_ERROR_CODE deInit();

private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
	bool m_bInit;
	bool m_bToScreen;
	bool m_bMVP;
	size_t m_nWidth;
	size_t m_nHeight;
	size_t m_nStride;
	CompVGLVertex m_Vertices[4];
	GLuint m_uNameVertexBuffer;
	GLuint m_uNameIndiceBuffer;
	GLuint m_uNamePrgAttPosition;
	GLuint m_uNamePrgAttTexCoord;
	GLuint m_uNamePrgUnifMVP;
	GLuint m_uNameVAO;
	CompVGLProgramPtr m_ptrProgram;
	CompVMVPPtr m_ptrMVP;
	COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

#endif /* _COMPV_GL_BLITTER_H_ */
