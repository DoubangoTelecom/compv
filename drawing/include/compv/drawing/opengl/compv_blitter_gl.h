/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_DRAWING_OPENGL_BLITTER_H_)
#define _COMPV_DRAWING_OPENGL_BLITTER_H_

#include "compv/base/compv_config.h"
#include "compv/drawing/opengl/compv_headers_gl.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/drawing/opengl/compv_consts_gl.h"
#include "compv/drawing/opengl/compv_program_gl.h"
#include "compv/drawing/opengl/compv_mvp_glm.h"
#include "compv/base/compv_obj.h"
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class CompVBlitterGL
{
protected:
	CompVBlitterGL();
public:
	virtual ~CompVBlitterGL();

	COMPV_INLINE size_t width()const { return m_nWidth; }
	COMPV_INLINE size_t height()const { return m_nHeight; }
	COMPV_INLINE size_t stride()const { return m_nStride; }
	COMPV_INLINE GLuint indicesCount()const { return sizeof(CompVGLTexture2DIndices) / sizeof(CompVGLTexture2DIndices[0]); }
	COMPV_INLINE CompVProgramPtr program() { return m_ptrProgram ? *m_ptrProgram : NULL; }
	
	virtual COMPV_ERROR_CODE bind();
	virtual COMPV_ERROR_CODE unbind();
	virtual COMPV_ERROR_CODE setMVP(CompVMVPPtr mvp);
	virtual COMPV_ERROR_CODE setSize(size_t width, size_t height, size_t stride);

protected:
	virtual COMPV_ERROR_CODE init(size_t width, size_t height, size_t stride, const std::string& prgVertexData, const std::string& prgFragData, bool bMVP = false, bool bToScreen = false);
	virtual COMPV_ERROR_CODE deInit();

private:
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
	CompVProgramGLPtr m_ptrProgram;
	CompVMVPPtr m_ptrMVP;
};

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

#endif /* _COMPV_DRAWING_OPENGL_BLITTER_H_ */
