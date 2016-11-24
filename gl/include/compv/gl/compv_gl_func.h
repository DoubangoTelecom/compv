/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_GL_FUNC_H_)
#define _COMPV_GL_FUNC_H_

#include "compv/gl/compv_config.h"
#include "compv/gl/compv_common.h"
#include "compv/gl/compv_gl_utils.h"
#include "compv/gl/compv_gl_headers.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

#if COMPV_GL_DEBUG
#	define COMPV_GL_CHECK_ERROR(funcname, line) \
	{ \
		std::string errString_;  \
		COMPV_CHECK_CODE_ASSERT(COMPVGLUtils::lastError(&errString_)); \
		if (!errString_.empty()) { \
			COMPV_DEBUG_ERROR_EX("COMPV_GL_DEBUG", "%s[line %d]: error: %s", #funcname, line, errString_.c_str()); \
		} \
	}
#else
#	define COMPV_GL_CHECK_ERROR(funcname, line)
#endif

//!\\ Please make sure to have the functions declared in alphabetical order.

#define COMPV_glActiveTexture(...)					glActiveTexture(__VA_ARGS__); COMPV_GL_CHECK_ERROR("glActiveTexture", __LINE__)
#define COMPV_glAttachShader(...)					glAttachShader(__VA_ARGS__); COMPV_GL_CHECK_ERROR("glAttachShader", __LINE__)
#define COMPV_glBindBuffer(...)						glBindBuffer(__VA_ARGS__); COMPV_GL_CHECK_ERROR("glBindBuffer", __LINE__)
#define COMPV_glBindFramebuffer(...)				glBindFramebuffer(__VA_ARGS__); COMPV_GL_CHECK_ERROR("glBindFramebuffer", __LINE__)
#define COMPV_glBindRenderbuffer(...)				glBindRenderbuffer(__VA_ARGS__); COMPV_GL_CHECK_ERROR("glBindRenderbuffer", __LINE__)
#define COMPV_glBindTexture(...)					glBindTexture(__VA_ARGS__); COMPV_GL_CHECK_ERROR("glBindTexture", __LINE__)
#define COMPV_glBindVertexArray(...)				glBindVertexArray(__VA_ARGS__); COMPV_GL_CHECK_ERROR("glBindVertexArray", __LINE__)
#define COMPV_glBufferData(...)						glBufferData(__VA_ARGS__); COMPV_GL_CHECK_ERROR("glBufferData", __LINE__)
#define COMPV_glClear(...)							glClear(__VA_ARGS__); COMPV_GL_CHECK_ERROR("glClear", __LINE__)
#define COMPV_glClearColor(...)						glClearColor(__VA_ARGS__); COMPV_GL_CHECK_ERROR("glClearColor", __LINE__)
#define COMPV_glClearStencil(...)					glClearStencil(__VA_ARGS__); COMPV_GL_CHECK_ERROR("glClearStencil", __LINE__)
#define COMPV_glCompileShader(...)					glCompileShader(__VA_ARGS__); COMPV_GL_CHECK_ERROR("glCompileShader", __LINE__)
#define COMPV_glDeleteBuffers(...)					glDeleteBuffers(__VA_ARGS__); COMPV_GL_CHECK_ERROR("glDeleteBuffers", __LINE__)
#define COMPV_glDeleteVertexArrays(...)				glDeleteVertexArrays(__VA_ARGS__); COMPV_GL_CHECK_ERROR("glDeleteVertexArrays", __LINE__)
#define COMPV_glDeleteFramebuffers(...)				glDeleteFramebuffers(__VA_ARGS__); COMPV_GL_CHECK_ERROR("glDeleteFramebuffers", __LINE__)
#define COMPV_glDeleteProgram(...)					glDeleteProgram(__VA_ARGS__); COMPV_GL_CHECK_ERROR("glDeleteProgram", __LINE__)
#define COMPV_glDeleteRenderbuffers(...)			glDeleteRenderbuffers(__VA_ARGS__); COMPV_GL_CHECK_ERROR("glDeleteRenderbuffers", __LINE__)
#define COMPV_glDeleteShader(...)					glDeleteShader(__VA_ARGS__); COMPV_GL_CHECK_ERROR("glDeleteShader", __LINE__)
#define COMPV_glDeleteTextures(...)					glDeleteTextures(__VA_ARGS__); COMPV_GL_CHECK_ERROR("glDeleteTextures", __LINE__)
#define COMPV_glDisable(...)						glDisable(__VA_ARGS__); COMPV_GL_CHECK_ERROR("glDisable", __LINE__)
#define COMPV_glEnableVertexAttribArray(...)		glEnableVertexAttribArray(__VA_ARGS__); COMPV_GL_CHECK_ERROR("glEnableVertexAttribArray", __LINE__)
#define COMPV_glDrawElements(...)					glDrawElements(__VA_ARGS__); COMPV_GL_CHECK_ERROR("glDrawElements", __LINE__)
#define COMPV_glFramebufferRenderbuffer(...)		glFramebufferRenderbuffer(__VA_ARGS__); COMPV_GL_CHECK_ERROR("glFramebufferRenderbuffer", __LINE__)
#define COMPV_glFramebufferTexture2D(...)			glFramebufferTexture2D(__VA_ARGS__); COMPV_GL_CHECK_ERROR("glFramebufferTexture2D", __LINE__)
#define COMPV_glGenBuffers(...)						glGenBuffers(__VA_ARGS__); COMPV_GL_CHECK_ERROR("glGenBuffers", __LINE__)
#define COMPV_glGenFramebuffers(...)				glGenFramebuffers(__VA_ARGS__); COMPV_GL_CHECK_ERROR("glGenFramebuffers", __LINE__)
#define COMPV_glGenRenderbuffers(...)				glGenRenderbuffers(__VA_ARGS__); COMPV_GL_CHECK_ERROR("glGenRenderbuffers", __LINE__)
#define COMPV_glGenTextures(...)					glGenTextures(__VA_ARGS__); COMPV_GL_CHECK_ERROR("glGenTextures", __LINE__)
#define COMPV_glGenVertexArrays(...)				glGenVertexArrays(__VA_ARGS__); COMPV_GL_CHECK_ERROR("glGenVertexArrays", __LINE__)
#define COMPV_glGetRenderbufferParameteriv(...)		glGetRenderbufferParameteriv(__VA_ARGS__); COMPV_GL_CHECK_ERROR("glGetRenderbufferParameteriv", __LINE__)
#define COMPV_glGetIntegerv(...)					glGetIntegerv(__VA_ARGS__); COMPV_GL_CHECK_ERROR("glGetIntegerv", __LINE__)
#define COMPV_glGetProgramInfoLog(...)				glGetProgramInfoLog(__VA_ARGS__); COMPV_GL_CHECK_ERROR("glGetProgramInfoLog", __LINE__)
#define COMPV_glGetProgramiv(...)					glGetProgramiv(__VA_ARGS__); COMPV_GL_CHECK_ERROR("glGetProgramiv", __LINE__)
#define COMPV_glGetShaderInfoLog(...)				glGetShaderInfoLog(__VA_ARGS__); COMPV_GL_CHECK_ERROR("glGetShaderInfoLog", __LINE__)
#define COMPV_glGetShaderiv(...)					glGetShaderiv(__VA_ARGS__); COMPV_GL_CHECK_ERROR("glGetShaderiv", __LINE__)
#define COMPV_glLinkProgram(...)					glLinkProgram(__VA_ARGS__); COMPV_GL_CHECK_ERROR("glLinkProgram", __LINE__)
#define COMPV_glPixelStorei(...)					glPixelStorei(__VA_ARGS__); COMPV_GL_CHECK_ERROR("glPixelStorei", __LINE__)
#define COMPV_glRenderbufferStorage(...)			glRenderbufferStorage(__VA_ARGS__); COMPV_GL_CHECK_ERROR("glRenderbufferStorage", __LINE__)
#define COMPV_glShaderSource(...)					glShaderSource(__VA_ARGS__); COMPV_GL_CHECK_ERROR("glShaderSource", __LINE__)
#define COMPV_glTexImage2D(...)						glTexImage2D(__VA_ARGS__); COMPV_GL_CHECK_ERROR("glTexImage2D", __LINE__)
#define COMPV_glTexParameterf(...)					glTexParameterf(__VA_ARGS__); COMPV_GL_CHECK_ERROR("glTexParameterf", __LINE__)
#define COMPV_glTexParameteri(...)					glTexParameteri(__VA_ARGS__); COMPV_GL_CHECK_ERROR("glTexParameteri", __LINE__)
#define COMPV_glTexSubImage2D(...)					glTexSubImage2D(__VA_ARGS__); COMPV_GL_CHECK_ERROR("glTexSubImage2D", __LINE__)
#define COMPV_glUniform1i(...)						glUniform1i(__VA_ARGS__); COMPV_GL_CHECK_ERROR("glUniform1i", __LINE__)
#define COMPV_glUniformMatrix4fv(...)				glUniformMatrix4fv(__VA_ARGS__); COMPV_GL_CHECK_ERROR("glUniformMatrix4fv", __LINE__)
#define COMPV_glUseProgram(...)						glUseProgram(__VA_ARGS__); COMPV_GL_CHECK_ERROR("glUseProgram", __LINE__)
#define COMPV_glVertexAttribPointer(...)			glVertexAttribPointer(__VA_ARGS__); COMPV_GL_CHECK_ERROR("glVertexAttribPointer", __LINE__)
#define COMPV_glViewport(...)						glViewport(__VA_ARGS__); COMPV_GL_CHECK_ERROR("glViewport", __LINE__)

COMPV_NAMESPACE_END()

#endif /* _COMPV_GL_FUNC_H_ */
