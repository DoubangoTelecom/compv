/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_DRAWING_RENDERER_GL_H_)
#define _COMPV_DRAWING_RENDERER_GL_H_

#include "compv/base/compv_config.h"
#include "compv/drawing/opengl/compv_headers_gl.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/drawing/compv_renderer.h"
#include "compv/drawing/opengl/compv_consts_gl.h"
#include "compv/drawing/compv_program.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_obj.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class CompVRendererGL;
typedef CompVPtr<CompVRendererGL* > CompVRendererGLPtr;
typedef CompVRendererGLPtr* CompVRendererGLPtrPtr;

class CompVRendererGL : public CompVRenderer
{
protected:
	CompVRendererGL(COMPV_PIXEL_FORMAT ePixelFormat, GLuint uNameSurfaceTexture);
public:
	virtual ~CompVRendererGL();
	COMPV_GET_OBJECT_ID("CompVRendererGL");

	virtual bool isGLEnabled()const { return true; };

	static COMPV_ERROR_CODE newObj(CompVRendererGLPtrPtr glRenderer, COMPV_PIXEL_FORMAT ePixelFormat, GLuint uNameSurfaceTexture);

protected:
	virtual CompVProgramPtr program() { return m_ptrProgram; }
	virtual const std::string& programVertexData()const = 0;
	virtual const std::string& programFragData()const = 0;
	virtual GLuint nameVertexBuffer()const { return m_uNameVertexBuffer; }
	virtual GLuint nameIndiceBuffer()const { return m_uNameIndiceBuffer; }
	virtual GLuint namePrgAttPosition()const { return m_uNamePrgAttPosition; }
	virtual GLuint namePrgAttTexCoord()const { return m_uNamePrgAttTexCoord; }
	virtual GLuint nameSurfaceTexture()const { return m_uNameSurfaceTexture;  }
	virtual GLuint indicesCount()const { return sizeof(CompVGLTexture2DIndices) / sizeof(CompVGLTexture2DIndices[0]); }
	virtual COMPV_ERROR_CODE bindBuffers();
	virtual COMPV_ERROR_CODE unbindBuffers();
	virtual COMPV_ERROR_CODE deInit();
	virtual COMPV_ERROR_CODE init(CompVMatPtr mat);

private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
	bool m_bInit;
	GLuint m_uNameSurfaceTexture;
	CompVGLVertex m_Vertices[4];
	GLuint m_uNameVertexBuffer;
	GLuint m_uNameIndiceBuffer;
	GLuint m_uNamePrgAttPosition;
	GLuint m_uNamePrgAttTexCoord;
	CompVProgramPtr m_ptrProgram;
	COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

#endif /* _COMPV_DRAWING_RENDERER_GL_H_ */