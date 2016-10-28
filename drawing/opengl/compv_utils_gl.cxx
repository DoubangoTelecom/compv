/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/drawing/opengl/compv_utils_gl.h"
#include "compv/drawing/compv_drawing.h"
#include "compv/base/compv_fileutils.h"
#include "compv/base/compv_mem.h"

COMPV_NAMESPACE_BEGIN()


#define kModuleNameGLUtils "GLUtils"

COMPV_ERROR_CODE CompVUtilsGL::getLastError(std::string *error)
{
	GLenum err;
	COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
	char buff_[33] = { 0 };
	while ((err = glGetError()) != GL_NO_ERROR) {
		if (error) {
#if defined(HAVE_GL_GLU_H) || defined(HAVE_GLU_H) || defined(__glu_h__) || defined(__GLU_H__)
			*error += "\n" + std::string((const char*)gluErrorString(err));
#else
			*error += "\n" + std::string((const char*)itoa((int)err, buff_, 10));
#endif
		}
		err_ = COMPV_ERROR_CODE_E_GL;
	}
	return err_;
}

COMPV_ERROR_CODE CompVUtilsGL::shadDelete(GLuint* uShad)
{
	COMPV_CHECK_EXP_RETURN(!uShad, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	if (CompVUtilsGL::shadIsValid(*uShad)) {
		glDeleteShader(*uShad);
	}
	*uShad = 0;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVUtilsGL::shadDelete(std::vector<GLuint >& uShads)
{
	std::vector<GLuint >::iterator it = uShads.begin();
	GLuint shad_;
	for (; it != uShads.end(); ++it) {
		shad_ = *it;
		COMPV_CHECK_CODE_RETURN(CompVUtilsGL::shadDelete(&shad_));
	}
	uShads.clear();
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVUtilsGL::shadCreate(GLuint* uShad, GLenum shadType)
{
	COMPV_CHECK_EXP_RETURN(!uShad, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
	std::string errString_;
	*uShad = 0;
	GLuint shad_ = glCreateShader(shadType);
	if (!CompVUtilsGL::shadIsValid(shad_)) {
		COMPV_CHECK_CODE_BAIL(err_ = CompVUtilsGL::getLastError(&errString_));
		COMPV_CHECK_CODE_BAIL(err_ = COMPV_ERROR_CODE_E_GL);
	}
	*uShad = shad_;
	return COMPV_ERROR_CODE_S_OK;
bail:
	COMPV_DEBUG_ERROR_EX(kModuleNameGLUtils, "glCreateShader failed: %s", errString_.c_str());
	return err_;
}

COMPV_ERROR_CODE CompVUtilsGL::shadCreateVert(GLuint* uShad)
{
	return CompVUtilsGL::shadCreate(uShad, GL_VERTEX_SHADER);
}

COMPV_ERROR_CODE CompVUtilsGL::shadCreateFrag(GLuint* uShad)
{
	return CompVUtilsGL::shadCreate(uShad, GL_FRAGMENT_SHADER);
}

bool CompVUtilsGL::shadIsValid(GLuint uShad)
{
	return uShad != 0 && glIsShader(uShad) == GL_TRUE;
}

COMPV_ERROR_CODE CompVUtilsGL::shadSetSource(GLuint uShad, GLsizei count, const GLchar **string, const GLint *length)
{
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::shadIsValid(uShad) || count <= 0 || !string || !*string, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
	std::string errString_;
	glShaderSource(uShad, count, string, length);
	COMPV_CHECK_CODE_RETURN(CompVUtilsGL::getLastError(&errString_));
	if (!errString_.empty()) {
		COMPV_DEBUG_ERROR_EX(kModuleNameGLUtils, "glShaderSource failed: %s", errString_.c_str());
		COMPV_CHECK_CODE_RETURN(err_ = COMPV_ERROR_CODE_E_GL);
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVUtilsGL::shadSetSource(GLuint uShad, const CompVBufferPtr& buff)
{
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::shadIsValid(uShad) || !buff || buff->isEmpty(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	const char* source_ = static_cast<const char*>(buff->getPtr());
	const GLint length_ = static_cast<GLint>(buff->getSize());
	COMPV_CHECK_CODE_RETURN(CompVUtilsGL::shadSetSource(uShad, 1, &source_, &length_));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVUtilsGL::shadSetSource(GLuint uShad, const char* filePath)
{
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::shadIsValid(uShad) || !filePath, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVBufferPtr buff_;
	COMPV_CHECK_CODE_RETURN(CompVFileUtils::read(filePath, &buff_));
	COMPV_CHECK_CODE_RETURN(CompVUtilsGL::shadSetSource(uShad, buff_));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVUtilsGL::shadCompile(GLuint uShad)
{
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::shadIsValid(uShad), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	std::string errString_;
	glCompileShader(uShad);
	COMPV_CHECK_CODE_RETURN(CompVUtilsGL::shadCompileGetStatus(uShad, &errString_));
	if (!errString_.empty()) {
		COMPV_DEBUG_ERROR_EX(kModuleNameGLUtils, "glCompileShader failed: %s", errString_.c_str());
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_GL);
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVUtilsGL::shadCompileGetStatus(GLuint uShad, std::string *error)
{
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::shadIsValid(uShad) || !error, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	GLint ok_ = 0;
	glGetShaderiv(uShad, GL_COMPILE_STATUS, &ok_);
	if (ok_ == GL_FALSE) {
		*error = "Unknown error";
		GLint maxLength = 0;
		glGetShaderiv(uShad, GL_INFO_LOG_LENGTH, &maxLength);
		if (maxLength > 0) {
			GLchar* infoLog = static_cast<GLchar*>(CompVMem::malloc(maxLength + 1));
			if (infoLog) {
				infoLog[maxLength] = '\0';
				glGetShaderInfoLog(uShad, maxLength, &maxLength, infoLog);
				*error = std::string((const char*)infoLog, maxLength);
				CompVMem::free((void**)&infoLog);
			}
			else {
				COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
			}
		}
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVUtilsGL::shadAttach(GLuint uPrg, GLuint uShad)
{
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::prgIsValid(uPrg) || !CompVUtilsGL::shadIsValid(uShad), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	std::string errString_;
	glAttachShader(uPrg, uShad);
	COMPV_CHECK_CODE_RETURN(CompVUtilsGL::getLastError(&errString_));
	if (!errString_.empty()) {
		COMPV_DEBUG_ERROR_EX(kModuleNameGLUtils, "glAttachShader failed: %s", errString_.c_str());
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_GL);
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVUtilsGL::texCreate(GLuint* uTex)
{
	COMPV_CHECK_EXP_RETURN(!uTex, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	*uTex = 0;
	glGenTextures(1, uTex); // returned value not texture yet until bind() is called
	if (!uTex) {
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_GL);
	}
	std::string errString_;
	COMPV_CHECK_CODE_RETURN(CompVUtilsGL::getLastError(&errString_));
	if (!errString_.empty()) {
		glDeleteTextures(1, uTex);
		*uTex = 0;
		COMPV_DEBUG_ERROR_EX(kModuleNameGLUtils, "glGenTextures failed: %s", errString_.c_str());
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_GL);
	}
	return COMPV_ERROR_CODE_S_OK;	
}

COMPV_ERROR_CODE CompVUtilsGL::texDelete(GLuint* uTex)
{
	// A texture is really a texture only after bind() -> do not use IsValid()
	if (uTex && *uTex) {
		glDeleteTextures(1, uTex);
		*uTex = 0;
	}
	return COMPV_ERROR_CODE_S_OK;
}

bool CompVUtilsGL::texIsValid(GLuint uTex)
{
	return uTex != 0 && glIsTexture(uTex) == GL_TRUE;
}

bool CompVUtilsGL::tex2DIsEnabled()
{
	return (glIsEnabled(GL_TEXTURE_2D) == GL_TRUE);
}

COMPV_ERROR_CODE CompVUtilsGL::texGetCurrent(GLuint* uTex)
{
	COMPV_CHECK_EXP_RETURN(!uTex, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	GLint text_ = 0;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &text_);
	*uTex = static_cast<GLuint>(text_);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVUtilsGL::texSetCurrent(GLuint uTex, bool checkErr /*= false*/)
{
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::texIsValid(uTex), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	glBindTexture(GL_TEXTURE_2D, uTex);
	if (checkErr) {
		std::string errString_;
		COMPV_CHECK_CODE_RETURN(CompVUtilsGL::getLastError(&errString_));
		if (!errString_.empty()) {
			COMPV_DEBUG_ERROR_EX(kModuleNameGLUtils, "glBindTexture failed: %s", errString_.c_str());
			COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_GL);
		}
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVUtilsGL::prgCreate(GLuint* uPrg)
{
	COMPV_CHECK_EXP_RETURN(!uPrg, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	*uPrg = glCreateProgram();
	if (!CompVUtilsGL::prgIsValid(*uPrg)) {
		std::string errString_;
		COMPV_CHECK_CODE_RETURN(CompVUtilsGL::getLastError(&errString_));
		if (!errString_.empty()) {
			COMPV_DEBUG_ERROR_EX(kModuleNameGLUtils, "glCreateProgram failed: %s", errString_.c_str());
		}
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_GL);
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVUtilsGL::prgDelete(GLuint* uPrg)
{
	if (uPrg) {
		if (CompVUtilsGL::prgIsValid(*uPrg)) {
			glDeleteProgram(*uPrg);
		}
		*uPrg = 0;
	}
	return COMPV_ERROR_CODE_S_OK;
}

bool CompVUtilsGL::prgIsValid(GLuint uPrg)
{
	return (uPrg != 0 && glIsProgram(uPrg) == GL_TRUE);
}

bool CompVUtilsGL::prgIsCurrent(GLuint uPrg)
{
	if (CompVUtilsGL::prgIsValid(uPrg)) {
		GLuint prg_ = 0;
		glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*)&prg_);
		return (prg_ == uPrg);
	}
	return false;
}

COMPV_ERROR_CODE CompVUtilsGL::prgLink(GLuint uPrg)
{
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::prgIsValid(uPrg), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	glLinkProgram(uPrg);
	std::string errString_;
	COMPV_CHECK_CODE_RETURN(CompVUtilsGL::prgLinkGetStatus(uPrg, &errString_));
	if (!errString_.empty()) {
		COMPV_DEBUG_ERROR_EX(kModuleNameGLUtils, "Program link info log: %s", errString_.c_str());
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_GL);
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVUtilsGL::prgLinkGetStatus(GLuint uPrg, std::string *error)
{
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::prgIsValid(uPrg) || !error, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	GLint ok_ = 0;
	glGetProgramiv(uPrg, GL_LINK_STATUS, (int *)&ok_);
	if (ok_ == GL_FALSE) {
		*error = "Unknown error";
		GLint maxLength = 0;
		glGetProgramiv(uPrg, GL_INFO_LOG_LENGTH, &maxLength);
		if (maxLength > 0) {
			GLchar* infoLog = static_cast<GLchar*>(CompVMem::malloc(maxLength + 1));
			if (infoLog) {
				infoLog[maxLength] = '\0';
				glGetProgramInfoLog(uPrg, maxLength, &maxLength, infoLog);
				*error = std::string((const char*)infoLog, maxLength);
			}
			else {
				COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
			}
		}
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVUtilsGL::prgUseBegin(GLuint uPrg, bool checkErr /*= false*/)
{
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::prgIsValid(uPrg), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	glUseProgram(uPrg);
	if (checkErr) {
		std::string errString_;
		COMPV_CHECK_CODE_RETURN(CompVUtilsGL::getLastError(&errString_));
		if (!errString_.empty()) {
			COMPV_DEBUG_ERROR_EX(kModuleNameGLUtils, "glUseProgram failed: %s", errString_.c_str());
			COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_GL);
		}
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVUtilsGL::prgUseEnd(GLuint uPrg, bool checkErr /*= false*/)
{
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::prgIsValid(uPrg), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	if (CompVUtilsGL::prgIsCurrent(uPrg)) {
		glUseProgram(0);
	}
	else {
		COMPV_DEBUG_WARN("Program (%u) not in use", uPrg);
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

