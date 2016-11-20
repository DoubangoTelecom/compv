/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_UI_OPENGL_UTILS_GL_H_)
#define _COMPV_UI_OPENGL_UTILS_GL_H_

#include "compv/base/compv_config.h"
#include "compv/drawing/opengl/compv_headers_gl.h"
#include "compv/base/compv_obj.h"
#include "compv/base/compv_buffer.h"
#include "compv/base/compv_common.h"

#include <vector>

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class CompVUtilsGL
{
public:
	static COMPV_ERROR_CODE getLastError(std::string *error);
	static COMPV_ERROR_CODE checkLastError();

	static CompVGLContext getCurrentContext();
	static bool isGLContextSet() { return CompVUtilsGL::getCurrentContext() != NULL; }

	static COMPV_ERROR_CODE shadDelete(GLuint* uShad);
	static COMPV_ERROR_CODE shadDelete(std::vector<GLuint >& uShads);
	static COMPV_ERROR_CODE shadCreate(GLuint* uShader, GLenum shadType);
	static COMPV_ERROR_CODE shadCreateVert(GLuint* uShad);
	static COMPV_ERROR_CODE shadCreateFrag(GLuint* uShad);
	static bool shadIsValid(GLuint uShader);
	static COMPV_ERROR_CODE shadSetSource(GLuint uShad, GLsizei count, const GLchar **string, const GLint *length);
	static COMPV_ERROR_CODE shadSetSource(GLuint uShad, const CompVBufferPtr& buff);
	static COMPV_ERROR_CODE shadSetSource(GLuint uShad, const char* filePath);
	static COMPV_ERROR_CODE shadCompile(GLuint uShad);
	static COMPV_ERROR_CODE shadCompileGetStatus(GLuint uShad, std::string *error);
	static COMPV_ERROR_CODE shadAttach(GLuint uPrg, GLuint uShad);

	static COMPV_ERROR_CODE texCreate(GLuint* uTex);
	static COMPV_ERROR_CODE texDelete(GLuint* uTex);
	static bool texIsValid(GLuint uTex);
	static bool tex2DIsEnabled();
	static COMPV_ERROR_CODE tex2DEnable();
	static COMPV_ERROR_CODE texGetCurrent(GLuint* uTex);
	static COMPV_ERROR_CODE texSetCurrent(GLuint uTex, bool checkErr = false);

	static COMPV_ERROR_CODE prgCreate(GLuint* uPrg);
	static COMPV_ERROR_CODE prgDelete(GLuint* uPrg);
	static bool prgIsValid(GLuint uPrg);
	static bool prgIsCurrent(GLuint uPrg);
	static COMPV_ERROR_CODE prgLink(GLuint uPrg);
	static COMPV_ERROR_CODE prgLinkGetStatus(GLuint uPrg, std::string *error);
	static COMPV_ERROR_CODE prgUseBegin(GLuint uPrg, bool checkErr = false);
	static COMPV_ERROR_CODE prgUseEnd(GLuint uPrg, bool checkErr = false);

private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
	COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_UI_OPENGL_UTILS_GL_H_ */
